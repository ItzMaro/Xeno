#include <future>

#include "worker.hpp"
#include "server/server.h"
#include "utils/ntdll.h"

extern std::vector<std::shared_ptr<RBXClient>> Clients;
std::mutex clientsMtx;

std::unordered_set<DWORD> closedClients;
std::unordered_set<DWORD> initializingClients;

static void newClient(DWORD pid) {
    if (closedClients.find(pid) != closedClients.end()) {
        return;
    }
    initializingClients.insert(pid);
    std::shared_ptr<RBXClient> client = std::make_shared<RBXClient>(pid);
    {
        std::lock_guard<std::mutex> lock(clientsMtx);
        Clients.push_back(std::move(client));
        initializingClients.erase(pid);
    }
}

static void init() {
    DWORD CurrentPID = GetCurrentProcessId();

    std::vector<DWORD> xenoPIDs = GetProcessIDsByName(L"Xeno.exe");
    std::vector<DWORD> xenoUIPIDs = GetProcessIDsByName(L"XenoUI.exe");

    xenoPIDs.insert(xenoPIDs.end(), xenoUIPIDs.begin(), xenoUIPIDs.end());

    for (const DWORD& pid : xenoPIDs) { // Terminate existing Xeno processes
        if (pid == CurrentPID)
            continue;
        HANDLE hXeno = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hXeno)
            TerminateProcess(hXeno, 0);
    }

    while (true) {
        std::vector<DWORD> client_pids = GetProcessIDsByName(L"RobloxPlayerBeta.exe");
        std::unordered_set<DWORD> current_pids(client_pids.begin(), client_pids.end());
        {
            std::lock_guard<std::mutex> lock(clientsMtx);

            Clients.erase(std::remove_if(Clients.begin(), Clients.end(), [&](const std::shared_ptr<RBXClient>& client) {
                if (current_pids.find(client->PID) == current_pids.end() || !client->isProcessAlive()) {
                    closedClients.insert(client->PID);
                    return true;
                }
                return false;
            }), Clients.end());

            for (const DWORD& pid : client_pids) {
                if (std::none_of(Clients.begin(), Clients.end(),
                    [&](const std::shared_ptr<RBXClient>& client) {return client->PID == pid;}) && initializingClients.find(pid) == initializingClients.end())
                {
                    std::thread(newClient, pid).detach();
                }
            }
        }
        Sleep(250);
    }
}

extern "C" {
    __declspec(dllexport) ClientInfo* GetClients() {
        static std::vector<ClientInfo> simpleClients;
        {
            std::lock_guard<std::mutex> lock(clientsMtx);
            simpleClients.clear();

            for (const auto& client : Clients) {
                simpleClients.push_back({client->Version.c_str() /*Version*/, client->Username.c_str() /*Username*/, static_cast<int>(client->PID) /*Process ID*/});
            }

            simpleClients.push_back({ nullptr, nullptr, 0 });
        }
        return simpleClients.data();
    }

    __declspec(dllexport) void Initialize() {
        /*
        FILE* conOut;
        AllocConsole();
        SetConsoleTitleA("Xeno");
        freopen_s(&conOut, "CONOUT$", "w", stdout);
        freopen_s(&conOut, "CONOUT$", "w", stderr);
        */
        
        HMODULE ntdll = LoadLibraryA("ntdll.dll");
        if (!ntdll) {
            std::cerr << "Could not load ntdll.dll\n";
            return;
        }
        NTDLL_INIT_FCNS(ntdll);

        std::thread(init).detach();
        std::thread(setup_connection).detach();
    }

    __declspec(dllexport) void Execute(const char* script_source, const char** client_users, int num_users) {
        std::string source(script_source);
        std::lock_guard<std::mutex> lock(clientsMtx);
        for (int i = 0; i < num_users; ++i) {
            for (const auto& client : Clients) {
                if (strcmp(client_users[i], client->Username.c_str()) == 0) {
                    client->execute(source);
                    break;
                }
            }
        }
    }

    __declspec(dllexport) const char* Compilable(const char* script_source) {
        std::string source(script_source);
        std::string result = compilable(source);
        char* result_cstr = new char[result.length() + 1];
        strcpy_s(result_cstr, result.length() + 1, result.c_str());
        return result_cstr;
    }
}