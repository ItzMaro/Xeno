#include <windows.h>
#include <objbase.h>

#include <psapi.h>
#include <regex>
#include <TlHelp32.h>
#include <fstream>
#include <mutex>

#include <xxhash.h>
#include <zstd.h>

#include "worker.hpp"

#include "utils/resource.h"

std::vector<std::uintptr_t> functions::GetChildrenAddresses(std::uintptr_t address, HANDLE handle) {
    std::vector<std::uintptr_t> children;
    {
        std::uintptr_t childrenPtr = read_memory<std::uintptr_t>(address + offsets::Children, handle);
        if (childrenPtr == 0)
            return children;

        std::uintptr_t childrenStart = read_memory<std::uintptr_t>(childrenPtr, handle);
        std::uintptr_t childrenEnd = read_memory<std::uintptr_t>(childrenPtr + offsets::This, handle) + 1;

        for (std::uintptr_t childAddress = childrenStart; childAddress < childrenEnd; childAddress += 0x10) {
            std::uintptr_t childPtr = read_memory<std::uintptr_t>(childAddress, handle);
            if (childPtr != 0)
                children.push_back(childPtr);
        }
    }
    return children;
}

std::string functions::ReadRobloxString(std::uintptr_t address, HANDLE handle) {
    std::uint64_t stringCount = read_memory<std::uint64_t>(address + 0x10, handle);

    if (stringCount > 15000 || stringCount <= 0)
        return "";

    if (stringCount > 15)
        address = read_memory<std::uintptr_t>(address, handle);

    std::string buffer;
    buffer.resize(stringCount);

    MEMORY_BASIC_INFORMATION bi;
    VirtualQueryEx(handle, reinterpret_cast<LPCVOID>(address), &bi, sizeof(bi));

    NtReadVirtualMemory(handle, reinterpret_cast<LPCVOID>(address), buffer.data(), (ULONG)stringCount, nullptr);

    PVOID baddr = bi.AllocationBase;
    SIZE_T size = bi.RegionSize;
    NtUnlockVirtualMemory(handle, &baddr, &size, 1);

    return buffer;
}

std::string Instance::GetBytecode() const {
    if (ClassName() != "LocalScript" && ClassName() != "ModuleScript")
        return "";

    std::uintptr_t embeddedSourceOffset = (ClassName() == "LocalScript") ? offsets::LocalScriptEmbedded : offsets::ModuleScriptEmbedded;
    std::uintptr_t embeddedPtr = read_memory<std::uintptr_t>(_Self + embeddedSourceOffset, handle);

    std::uintptr_t bytecodePtr = read_memory<std::uintptr_t>(embeddedPtr + offsets::Bytecode, handle);
    std::uint64_t bytecodeSize = read_memory<std::uint64_t>(embeddedPtr + offsets::BytecodeSize, handle);

    std::string bytecodeBuffer;
    bytecodeBuffer.resize(bytecodeSize);

    MEMORY_BASIC_INFORMATION bi;
    VirtualQueryEx(handle, reinterpret_cast<LPCVOID>(bytecodePtr), &bi, sizeof(bi));

    NtReadVirtualMemory(handle, reinterpret_cast<LPCVOID>(bytecodePtr), bytecodeBuffer.data(), (ULONG)bytecodeSize, nullptr);

    PVOID baddr = bi.AllocationBase;
    SIZE_T size = bi.RegionSize;
    NtUnlockVirtualMemory(handle, &baddr, &size, 1);

    return decompress(bytecodeBuffer);
}

static HMODULE getModule() {
    HMODULE hModule;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)getModule,
        &hModule);
    return hModule;
}

static std::filesystem::path GetHandlePath(HANDLE processHandle) {
    char buffer[MAX_PATH];
    DWORD bufferSize = sizeof(buffer);

    if (QueryFullProcessImageNameA(processHandle, 0, buffer, &bufferSize)) {
        return std::filesystem::path(buffer);
    }

    return {};
}

static std::string generateGUID() {
    GUID guid;
    HRESULT result = CoCreateGuid(&guid);

    if (result != S_OK) {
        throw std::runtime_error("Failed to generate GUID");
    }

    char guidStr[39];
    snprintf(guidStr, sizeof(guidStr), "%08lX-%04X-%04X-%04X-%012llX", guid.Data1,
        guid.Data2, guid.Data3, (guid.Data4[0] << 8) | guid.Data4[1],
        ((static_cast<unsigned long long>(guid.Data4[2]) << 40) |
         (static_cast<unsigned long long>(guid.Data4[3]) << 32) |
         (static_cast<unsigned long long>(guid.Data4[4]) << 24) |
         (static_cast<unsigned long long>(guid.Data4[5]) << 16) |
         (static_cast<unsigned long long>(guid.Data4[6]) << 8)  |
          static_cast<unsigned long long>(guid.Data4[7])));

    return std::string(guidStr);
}

static void replaceString(std::string& source, const std::string_view toReplace, const std::string_view replacement) {
    size_t pos = source.find(toReplace);

    if (pos != std::string::npos) {
        source.replace(pos, toReplace.length(), replacement);
    }
}

RBXClient::RBXClient(DWORD processID) :
    handle(OpenProcess(PROCESS_ALL_ACCESS, TRUE, processID)),
    PID(processID)
{
    if (handle == NULL) {
        std::cerr << "[!] Failed to open process " << processID << ": " << GetLastError() << "\n";
        return;
    }

    ClientDir = GetHandlePath(handle).parent_path();
    GUID = generateGUID();
    Version = ClientDir.filename().string();

    // Not really the best way to wait for the client to load

    PROCESS_MEMORY_COUNTERS memory_counter;
    K32GetProcessMemoryInfo(handle, &memory_counter, sizeof(memory_counter));
    while (memory_counter.WorkingSetSize < 150000000) {
        K32GetProcessMemoryInfo(handle, &memory_counter, sizeof(memory_counter));
        Sleep(100); 
    }

    HWND clientHWND = GetHWNDFromPID(GetProcessId(handle));
    if (!clientHWND) {
        while (!clientHWND) {
            clientHWND = GetHWNDFromPID(GetProcessId(handle));
            std::cout << "Waiting for Client HWND\n";
            Sleep(25);
        }
        Sleep(1000);
    }

    RenderView = GetRV(handle);

    std::uintptr_t dataModelAddress = FetchDataModel();
    if (dataModelAddress == 0) {
        std::cerr << "[!] Failed to fetch datamodel\n";
        return;
    }

    Instance DataModel(dataModelAddress, handle);

    std::uintptr_t LocalPlayerAddr = read_memory<std::uintptr_t>(DataModel.FindFirstChildOfClassAddress("Players") + offsets::LocalPlayer, handle);
    while (LocalPlayerAddr == 0) {
        std::cout << "Waiting for LocalPlayer\n";
        LocalPlayerAddr = read_memory<std::uintptr_t>(DataModel.FindFirstChildOfClassAddress("Players") + offsets::LocalPlayer, handle);
        Sleep(25);
    }

    Instance LocalPlayer(LocalPlayerAddr, handle);
    Username = LocalPlayer.Name();

    while (Username == "Player") {
        std::cout << "Waiting for LocalPlayer to load\n";
        Username = LocalPlayer.Name();
        Sleep(25);
    }

    if (Username == "")
        Username = "Unknown";

    // Need to add checks else the process will crash, yes it sucks.
    auto CoreGui = DataModel.FindFirstChild("CoreGui");
    if (!CoreGui) {
        std::cerr << "[!] Game->CoreGui not found\n";
        return;
    }

    auto RobloxGui = CoreGui->FindFirstChild("RobloxGui");
    if (!RobloxGui) {
        std::cerr << "[!] CoreGui->RobloxGui not found\n";
        return;
    }
    auto Modules = RobloxGui->FindFirstChild("Modules");
    if (!Modules) {
        std::cerr << "[!] RobloxGui->Modules not found\n";
        return;
    }

    std::unique_ptr<Instance> PatchScript = nullptr;
    {
        auto Common = Modules->FindFirstChild("Common");
        if (!Common) {
            std::cerr << "[!] Modules->Common not found\n";
            return;
        }
        PatchScript = Common->FindFirstChild("Url");
    }

    if (!PatchScript) {
        std::cerr << "[!] Patch Script was not found\n";
        return;
    }

    std::string clientScript;
    {
        HMODULE module = getModule();
        if (!module) {
            std::cerr << "[!] Could not get module handle: " << GetLastError() << "\n";
            return;
        }
        HRSRC resource = FindResource(module, MAKEINTRESOURCE(SCRIPT), MAKEINTRESOURCE(LUAFILE));
        if (!resource) {
            std::cerr << "[!] Could not get the client.lua resource\n";
            return;
        }

        HGLOBAL data = LoadResource(module, resource);
        if (!data) {
            std::cerr << "[!] Could not get the could load resource\n";
            return;
        }

        DWORD size = SizeofResource(module, resource);
        char* finalData = static_cast<char*>(LockResource(data));

        clientScript.assign(finalData, size);
    }

    replaceString(clientScript, "%XENO_UNIQUE_ID%", GUID);
    replaceString(clientScript, "%XENO_VERSION%", Xeno_Version);

    const std::string PatchScriptSource = "--!native\n--!optimize 1\n--!nonstrict\nlocal a={}local b=game:GetService(\"ContentProvider\")local function c(d)local e,f=d:find(\"%.\")local g=d:sub(f+1)if g:sub(-1)~=\"/\"then g=g..\"/\"end;return g end;local d=b.BaseUrl;local g=c(d)local h=string.format(\"https://games.%s\",g)local i=string.format(\"https://apis.rcs.%s\",g)local j=string.format(\"https://apis.%s\",g)local k=string.format(\"https://accountsettings.%s\",g)local l=string.format(\"https://gameinternationalization.%s\",g)local m=string.format(\"https://locale.%s\",g)local n=string.format(\"https://users.%s\",g)local o={GAME_URL=h,RCS_URL=i,APIS_URL=j,ACCOUNT_SETTINGS_URL=k,GAME_INTERNATIONALIZATION_URL=l,LOCALE_URL=m,ROLES_URL=n}setmetatable(a,{__newindex=function(p,q,r)end,__index=function(p,r)return o[r]end})return a";

    if (DataModel.Name() == "App") { // In home page
        PatchScript->SetBytecode(Compile("coroutine.wrap(function(...)" + clientScript + "\nend)();" + PatchScriptSource));
        return;
    }

    // In-game, hooking a module that has custom bytecode we are writing.

    auto RobloxReplicatedStorage = DataModel.FindFirstChildOfClass("RobloxReplicatedStorage");
    if (RobloxReplicatedStorage->FindFirstChild("Xeno")) {
        std::cerr << "[!] Client '" << Username << "' is already attached\n";
        // When player serverhops the GUID is going to be replaced with the new one. This fixes the communication with the bridge
        PatchScript->SetBytecode(Compile("coroutine.wrap(function(...)" + clientScript + "\nend)();" + PatchScriptSource));
        return;
    }

    std::lock_guard<std::mutex> lock(clientsMtx);

    std::unique_ptr<Instance> PlayerListManager = nullptr;
    {
        auto PlayerList = Modules->FindFirstChild("PlayerList");
        if (!PlayerList) {
            std::cerr << "[!] Modules->PlayerList not found\n";
            return;
        }
        PlayerListManager = PlayerList->FindFirstChild("PlayerListManager");
    }
    if (!PlayerListManager) {
        std::cerr << "[!] PlayerListManager not found\n";
        return;
    }

    if (!RobloxGui->FindFirstChild("DropDownFullscreenFrame")) { // If the player is joining the game
        std::cout << "Waiting for " + Username + " to join the game\n";
        RobloxGui->WaitForChildAddress("DropDownFullscreenFrame");
        if (DataModel.Name() == "App") { // In case the player leaves the join page
            PatchScript->SetBytecode(Compile("coroutine.wrap(function(...)" + clientScript + "\nend)();" + PatchScriptSource));
            return;
        }
        std::cout << "Joined, Waiting 2.5 seconds for " + Username + " to load\n";
        Sleep(2500);
    }

    std::unique_ptr<Instance> VRNavigation = nullptr;
    {
        auto StarterPlayer = DataModel.FindFirstChildOfClass("StarterPlayer");
        if (!StarterPlayer) {
            std::cerr << "[!] Game->StarterPlayer not found\n";
            return;
        }
        auto StarterPlayerScripts = StarterPlayer->FindFirstChild("StarterPlayerScripts");
        if (!StarterPlayerScripts) {
            std::cerr << "[!] StarterPlayer->StarterPlayerScripts not found\n";
            return;
        }
        auto PlayerModule = StarterPlayerScripts->FindFirstChild("PlayerModule");
        if (!PlayerModule) {
            std::cerr << "[!] StarterPlayerScripts->PlayerModule not found\n";
            return;
        }
        auto ControlModule = PlayerModule->FindFirstChild("ControlModule");
        if (!ControlModule) {
            std::cerr << "[!] PlayerModule->ControlModule not found\n";
            return;
        }
        VRNavigation = ControlModule->FindFirstChild("VRNavigation");
    }

    if (!VRNavigation) {
        std::cerr << "[!] VRNavigation not found (Is the client '" + Username + "' already attached?)\n";
        return;
    }

    VRNavigation->UnlockModule();
    write_memory<std::uintptr_t>(PlayerListManager->Self() + offsets::This, VRNavigation->Self(), handle);

    VRNavigation->SetBytecode(Compile("script.Parent=nil;coroutine.wrap(function(...)" + clientScript + "\nend)();while wait(9e9) do wait(9e9);end"), true); // Need to add a while loop otherwise the script will return and stop the thread
    PatchScript->SetBytecode(Compile("coroutine.wrap(function(...)" + clientScript + "\nend)();" + PatchScriptSource)); // For later use (when player leaves game/teleports)

    std::thread([clientHWND]() {
        HWND previousHWND = GetForegroundWindow();

        while (GetForegroundWindow() != clientHWND) {
            SetForegroundWindow(clientHWND);
            Sleep(5);
        }

        keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, 0), KEYEVENTF_SCANCODE, 0);
        keybd_event(VK_ESCAPE, MapVirtualKey(VK_ESCAPE, 0), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0);

        Sleep(100);

        if (previousHWND != nullptr) {
            SetForegroundWindow(previousHWND);
        }
    }).detach();

    Sleep(800);

    write_memory<std::uintptr_t>(PlayerListManager->Self() + offsets::This, PlayerListManager->Self(), handle);

    if (Username == "Unknown") { // Atleast the username dosen't have to be "Unknown" and we can try to change that
        std::uintptr_t newLocalPlayerAddr = read_memory<std::uintptr_t>(DataModel.FindFirstChildOfClassAddress("Players") + offsets::LocalPlayer, handle);
        Instance newLocalPlayer = Instance(newLocalPlayerAddr, handle);
        Username = newLocalPlayer.Name();
        if (Username == "")
            Username = "Unknown";
    }
}

void RBXClient::execute(const std::string& source) const {
    std::uintptr_t dataModel_Address = FetchDataModel();

    Instance DataModel(dataModel_Address, handle);
    auto RobloxReplicatedStorage = DataModel.FindFirstChildOfClass("RobloxReplicatedStorage");
    if (!RobloxReplicatedStorage)
        return;

    auto xenoFolder = RobloxReplicatedStorage->FindFirstChild("Xeno");
    if (!xenoFolder)
        return;

    auto xenoModules = xenoFolder->FindFirstChild("Scripts");
    if (!xenoModules)
        return;

    auto xenoModule = xenoModules->FindFirstChildOfClass("ModuleScript");
    if (!xenoModule)
        return;

    xenoModule->SetBytecode(Compile("return {['x e n o']=function(...)do local function s(i, v)getfenv(debug.info(0, 'f'))[i] = v;getfenv(debug.info(1, 'f'))[i] = v;end;for i,v in pairs(getfenv(debug.info(1,'f')))do s(i, v)end;setmetatable(getgenv(),{__newindex=function(t,i,v)rawset(t,i,v)s(i,v)end})end;" + source +"\nend}"), true);
    
    xenoModule->UnlockModule();
}

bool RBXClient::loadstring(const std::string& source, const std::string& script_name, const std::string& chunk_name) const {
    std::uintptr_t dataModel_Address = FetchDataModel();

    Instance DataModel(dataModel_Address, handle);
    auto RobloxReplicatedStorage = DataModel.FindFirstChildOfClass("RobloxReplicatedStorage");
    if (!RobloxReplicatedStorage)
        return false;

    auto xenoFolder = RobloxReplicatedStorage->FindFirstChild("Xeno");
    if (!xenoFolder)
        return false;

    auto cloned_module = xenoFolder->FindFirstChild(script_name);
    if (!cloned_module)
        return false;

    cloned_module->SetBytecode(Compile("return{[ [[" + chunk_name + "]] ]=function(...)do local function s(i, v)getfenv(debug.info(0, 'f'))[i] = v;getfenv(debug.info(1, 'f'))[i] = v;end;for i,v in pairs(getfenv(debug.info(1,'f')))do s(i, v)end;setmetatable(getgenv and getgenv()or{},{__newindex=function(t,i,v)rawset(t,i,v)s(i,v)end})end;" + source + "\nend}"), true);

    cloned_module->UnlockModule();

    return true;
}

std::uintptr_t RBXClient::GetObjectValuePtr(const std::string_view objectval_name) const
{
    std::uintptr_t dataModel_Address = FetchDataModel();

    Instance DataModel(dataModel_Address, handle);
    auto RobloxReplicatedStorage = DataModel.FindFirstChildOfClass("RobloxReplicatedStorage");

    auto xenoFolder = RobloxReplicatedStorage->FindFirstChild("Xeno");
    if (!xenoFolder)
        return 0;

    auto objectValContainer = xenoFolder->FindFirstChild("Instance Pointers");
    if (!objectValContainer)
        return 0;

    std::uintptr_t objectValue = objectValContainer->FindFirstChildAddress(objectval_name);
    if (objectValue == 0)
        return 0;

    return read_memory<std::uintptr_t>(objectValue + offsets::ObjectValue, handle);
}

std::vector<DWORD> GetProcessIDsByName(const std::wstring_view processName) {
    std::vector<DWORD> processIDs;

    HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapShot == INVALID_HANDLE_VALUE)
        return processIDs;

    PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };

    if (Process32FirstW(snapShot, &entry)) {
        if (_wcsicmp(processName.data(), entry.szExeFile) == 0) {
            processIDs.push_back(entry.th32ProcessID);
        }
        while (Process32NextW(snapShot, &entry)) {
            if (_wcsicmp(processName.data(), entry.szExeFile) == 0) {
                processIDs.push_back(entry.th32ProcessID);
            }
        }
    }

    CloseHandle(snapShot);
    return processIDs;
}

std::uintptr_t GetRV(HANDLE handle)
{
    std::filesystem::path localAppData = std::filesystem::temp_directory_path().parent_path().parent_path();
    std::filesystem::path logs = localAppData / "Roblox" / "logs";

    if (!std::filesystem::is_directory(logs)) {
        std::cerr << "[!] Roblox logs directory not found\n";
        return 0;
    }

    int tries = 5;
    while (tries >= 1) {
        tries -= 1;
        std::vector<std::filesystem::path> logFiles;

        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(logs)) {
            std::filesystem::path path = entry.path();
            if (entry.is_regular_file() && path.extension() == ".log")
                logFiles.push_back(path);
        }

        if (logFiles.empty()) {
            std::cerr << "[!] No Roblox log files found\n";
            return 0;
        }

        std::sort(logFiles.begin(), logFiles.end(), [](const auto& x, const auto& y) {
            return std::filesystem::last_write_time(x) > std::filesystem::last_write_time(y);
        });

        std::vector<std::filesystem::path> lockedFiles;
        for (const std::filesystem::path& logPath : logFiles) {
            try {
                std::filesystem::remove(logPath);
            } catch (const std::filesystem::filesystem_error& e) {
                lockedFiles.push_back(logPath);
            }
        }
        if (lockedFiles.empty()) {
            std::cerr << "[!] No log file is being used by another process\n";
            return 0;
        }

        for (const std::filesystem::path& logPath : lockedFiles) {
            std::ifstream logFile(logPath);
            if (!logFile.is_open()) {
                std::cout << "[!] Could not open file " << logPath << "\n";
                continue;
            }

            std::string renderview;
            std::stringstream buffer;
            buffer << logFile.rdbuf();
            std::string content = buffer.str();

            std::regex rgx(R"(\bSurfaceController\[_:1\]::initialize view\((.*?)\))");
            std::smatch match;

            if (std::regex_search(content, match, rgx)) {
                if (match.size() <= 1)
                    continue;
                renderview = match[1].str();
            }

            std::uintptr_t renderviewAddress = std::strtoull(renderview.c_str(), nullptr, 16);
            std::uintptr_t fakeDataModel = read_memory<std::uintptr_t>(renderviewAddress + 0x118, handle);
            if (fakeDataModel != 0)
                return renderviewAddress;
        }

        Sleep(1000);
    }
    return 0;
}

static std::string compress(const std::string_view bytecode)
{
    const auto data_size = bytecode.size();
    const auto max_size = ZSTD_compressBound(data_size);
    auto buffer = std::vector<char>(max_size + 8);

    strcpy_s(&buffer[0], buffer.capacity(), "RSB1");
    memcpy_s(&buffer[4], buffer.capacity(), &data_size, sizeof(data_size));

    const auto compressed_size = ZSTD_compress(&buffer[8], max_size, bytecode.data(), data_size, ZSTD_maxCLevel());
    if (ZSTD_isError(compressed_size))
        return "";

    const auto size = compressed_size + 8;
    const auto key = XXH32(buffer.data(), size, 42u);
    const auto bytes = reinterpret_cast<const uint8_t*>(&key);

    for (auto i = 0u; i < size; ++i)
        buffer[i] ^= bytes[i % 4] + i * 41u;

    return std::string(buffer.data(), size);
}

std::string decompress(const std::string_view compressed) {
    const uint8_t bytecodeSignature[4] = { 'R', 'S', 'B', '1' };
    const int bytecodeHashMultiplier = 41;
    const int bytecodeHashSeed = 42;

    if (compressed.size() < 8)
        return "Compressed data too short";

    std::vector<uint8_t> compressedData(compressed.begin(), compressed.end());
    std::vector<uint8_t> headerBuffer(4);

    for (size_t i = 0; i < 4; ++i) {
        headerBuffer[i] = compressedData[i] ^ bytecodeSignature[i];
        headerBuffer[i] = (headerBuffer[i] - i * bytecodeHashMultiplier) % 256;
    }

    for (size_t i = 0; i < compressedData.size(); ++i) {
        compressedData[i] ^= (headerBuffer[i % 4] + i * bytecodeHashMultiplier) % 256;
    }

    uint32_t hashValue = 0;
    for (size_t i = 0; i < 4; ++i) {
        hashValue |= headerBuffer[i] << (i * 8);
    }

    uint32_t rehash = XXH32(compressedData.data(), compressedData.size(), bytecodeHashSeed);
    if (rehash != hashValue)
        return "Hash mismatch during decompression";

    uint32_t decompressedSize = 0;
    for (size_t i = 4; i < 8; ++i) {
        decompressedSize |= compressedData[i] << ((i - 4) * 8);
    }

    compressedData = std::vector<uint8_t>(compressedData.begin() + 8, compressedData.end());
    std::vector<uint8_t> decompressed(decompressedSize);

    size_t const actualDecompressedSize = ZSTD_decompress(decompressed.data(), decompressedSize, compressedData.data(), compressedData.size());
    if (ZSTD_isError(actualDecompressedSize))
        return "ZSTD decompression error: " + std::string(ZSTD_getErrorName(actualDecompressedSize));

    decompressed.resize(actualDecompressedSize);
    return std::string(decompressed.begin(), decompressed.end());
}

std::string compilable(const std::string& source, bool returnBytecode) {
    static bytecode_encoder_t encoder = bytecode_encoder_t();
    std::string bytecode = Luau::compile(source, {}, {}, &encoder);
    if (bytecode[0] == '\0') {
        bytecode.erase(std::remove(bytecode.begin(), bytecode.end(), '\0'), bytecode.end());
        return bytecode;
    }
    if (returnBytecode) {
        return bytecode;
    }
    return "success";
}

std::string Compile(const std::string& source)
{
    static bytecode_encoder_t encoder = bytecode_encoder_t();
    const std::string bytecode = Luau::compile(source, {}, {}, &encoder);

    if (bytecode[0] == '\0') {
        std::string bytecodeP = bytecode;
        bytecodeP.erase(std::remove(bytecodeP.begin(), bytecodeP.end(), '\0'), bytecodeP.end());
        std::cerr << "Byecode compile failed: " << bytecodeP << std::endl;
    }

    return compress(bytecode);
}

static BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);

    std::pair<DWORD, HWND*>* params = reinterpret_cast<std::pair<DWORD, HWND*>*>(lParam);
    DWORD targetProcessId = params->first;
    HWND* resultHWND = params->second;

    if (lpdwProcessId == targetProcessId)
    {
        *resultHWND = hwnd;
        return FALSE;
    }

    return TRUE;
}

HWND GetHWNDFromPID(DWORD process_id) {
    HWND hwnd = nullptr;
    std::pair<DWORD, HWND*> params(process_id, &hwnd);

    EnumWindows(EnumWindowsProcMy, reinterpret_cast<LPARAM>(&params));

    return hwnd;
}

template<typename T>
T read_memory(std::uintptr_t address, HANDLE handle)
{
    T value = 0;
    MEMORY_BASIC_INFORMATION bi;

    VirtualQueryEx(handle, reinterpret_cast<LPCVOID>(address), &bi, sizeof(bi));

    NtReadVirtualMemory(handle, reinterpret_cast<LPCVOID>(address), &value, sizeof(value), nullptr);

    PVOID baddr = bi.AllocationBase;
    SIZE_T size = bi.RegionSize;
    NtUnlockVirtualMemory(handle, &baddr, &size, 1);

    return value;
}

template <typename T>
bool write_memory(std::uintptr_t address, const T& value, HANDLE handle)
{
    SIZE_T bytesWritten;
    DWORD oldProtection;

    if (!VirtualProtectEx(handle, reinterpret_cast<LPVOID>(address), sizeof(value), PAGE_READWRITE, &oldProtection)) {
        return false;
    }

    if (NtWriteVirtualMemory(handle, reinterpret_cast<PVOID>(address), (PVOID)&value, sizeof(value), &bytesWritten) || bytesWritten != sizeof(value)) {
        return false;
    }

    DWORD d;
    if (!VirtualProtectEx(handle, reinterpret_cast<LPVOID>(address), sizeof(value), oldProtection, &d)) {
        return false;
    }

    return true;
}