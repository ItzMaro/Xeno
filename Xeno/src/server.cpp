#include <regex>
#include <filesystem>
#include <fstream>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "server/httplib.h"
using namespace httplib;

#include "server/nlohmann/json.hpp"
using json = nlohmann::json;

#include "server/server.h"
#include "utils/base64.hpp"
#include "worker.hpp"

std::vector<std::shared_ptr<RBXClient>> Clients;
std::unordered_map<std::string, json> Globals;

static bool withinDirectory(const std::filesystem::path& base, const std::filesystem::path& path) {
	std::filesystem::path baseAbs = std::filesystem::absolute(base).lexically_normal();
	std::filesystem::path resolvedPath = baseAbs;

	for (const std::filesystem::path& part : path) {
		if (part == "..") {
			if (resolvedPath.has_parent_path()) {
				resolvedPath = resolvedPath.parent_path();
			}
		} else if (part != ".") {
			resolvedPath /= part;
		}
	}

	resolvedPath = resolvedPath.lexically_normal();

	return std::mismatch(baseAbs.begin(), baseAbs.end(), resolvedPath.begin()).first == baseAbs.end();
}

bool console_active = false;
std::mutex rconsoleMtx;
static void activate_console() {
	// using mutex will be a bit slower but it will fix some console issues
	std::lock_guard<std::mutex> lock(rconsoleMtx);
	if (console_active)
		return;

	console_active = true;

	AllocConsole();
	SetConsoleTitleA("Xeno");
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
	freopen_s(&pCout, "CONIN$", "r", stdin);
}

static void serve(Response& res, const json& body) {
	const std::string_view cType = body["c"];

	if (cType == "rf") { // read file
		if (!body.contains("p") /*path*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		if (!std::filesystem::is_regular_file(path)) {
			res.status = 400;
			res.set_content(R"({"error":"Given path is not a normal file"})", "application/json");
			return;
		}

		std::ifstream file(path);
		if (!file) {
			res.status = 500;
			res.set_content(R"({"error":"Could not open the given file"})", "application/json");
			return;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();

		res.status = 200;
		res.set_content(buffer.str(), "text/plain");
		return;
	}

	if (cType == "lf") { // list files
		if (!body.contains("p") /*path*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}
		if (!std::filesystem::is_directory(path)) {
			res.status = 400;
			res.set_content(R"({"error":"Given path is not a directory"})", "application/json");
			return;
		}

		json responseJ;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
			responseJ.push_back(entry.path().string());
		}

		res.status = 200;
		res.set_content(responseJ.dump(), "application/json");
		return;
	}

	if (cType == "if") { // is folder
		if (!body.contains("p") /*path*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		res.status = 200;
		if (std::filesystem::is_directory(path)) {
			res.set_content("dir", "text/plain");
			return;
		}
		if (std::filesystem::is_regular_file(path)) {
			res.set_content("file", "text/plain");
			return;
		}
		return;
	}

	if (cType == "mf") { // make folder
		if (!body.contains("p") /*path*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		if (std::filesystem::is_directory(path)) {
			res.status = 400;
			res.set_content(R"({"error":"Provided directory already exists"})", "application/json");
			return;
		}

		if (std::filesystem::is_regular_file(path)) {
			res.status = 400;
			res.set_content(R"({"error":"A file with the provided directory name already exists"})", "application/json");
			return;
		}

		try {
			if (std::filesystem::create_directory(path)) {
				res.status = 201;
			} else {
				res.status = 500;
				res.set_content(R"({"error":"Could not create a new directory"})", "application/json");
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			res.status = 500;
			res.set_content("{\"error\":\"Could not create a new directory: " + std::string(e.what()) + "\"})", "application/json");
		}
		return;
	}

	if (cType == "dfl") { // delete folder
		if (!body.contains("p") /*path*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		if (!std::filesystem::is_directory(path)) {
			res.status = 400;
			res.set_content(R"({"error":"Given path is not a directory"})", "application/json");
			return;
		}

		try {
			if (std::filesystem::remove_all(path)) {
				res.status = 200;
			} else {
				res.status = 500;
				res.set_content(R"({"error":"Could not delete directory"})", "application/json");
			}
		} catch (const std::filesystem::filesystem_error& e) {
			res.status = 500;
			res.set_content("{\"error\":\"Could not delete directory: " + std::string(e.what()) + "\"})", "application/json");
		}

		return;
	}

	if (cType == "df") { // delete file
		if (!body.contains("p") /*path*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		if (!std::filesystem::is_regular_file(path)) {
			res.status = 400;
			res.set_content(R"({"error":"Given path is not a normal file"})", "application/json");
			return;
		}

		try {
			if (std::filesystem::remove(path)) {
				res.status = 200;
			} else {
				res.status = 500;
				res.set_content(R"({"error":"Could not delete file"})", "application/json");
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			res.status = 500;
			res.set_content("{\"error\":\"Could not delete file: " + std::string(e.what()) + "\"})", "application/json");
		}
		return;
	}

	if (cType == "cas") { // get custom asset
		if (!body.contains("p") /*path*/ || !body.contains("pid") /*process id*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string path = body["p"];
		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		const std::filesystem::path sourcePath = std::filesystem::current_path() / path;

		if (!std::filesystem::is_regular_file(sourcePath)) {
			res.status = 400;
			res.set_content(R"({"error":"Given path is not a normal file"})", "application/json");
			return;
		}

		const std::string pid = body["pid"];
		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				std::filesystem::path contentDir = client->ClientDir / "content";

				if (!std::filesystem::is_directory(contentDir)) {
					res.status = 400;
					res.set_content(R"({"error":"directory 'content' does not exist or is not a directory"})", "application/json");
					return;
				}

				std::filesystem::path destinationPath = contentDir / sourcePath.filename();
				std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);

				res.status = 200;
				res.set_content("rbxasset://" + destinationPath.filename().string(), "text/plain");
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	if (cType == "rq") { // request
		if (!body.contains("l") /*url*/ || !body.contains("m") /*method*/ || !body.contains("b") /*body*/ || !body.contains("h") /*headers*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string url = body["l"];
		const std::string method = body["m"];
		const std::string rBody = base64::from_base64(body["b"]);
		const json headersJ = body["h"];

		const std::regex urlR(R"(^(http[s]?:\/\/)?([^\/]+)(\/.*)?$)");
		std::smatch urlM;
		std::string host;
		std::string path;
		if (std::regex_match(url, urlM, urlR)) {
			host = urlM[2];
			path = urlM[3].str();
		} else {
			res.status = 400;
			res.set_content(R"({"error":"Invalid URL"})", "application/json");
			return;
		}

		Client client(host.c_str());
		client.set_follow_location(true);

		Headers headers;
		for (auto it = headersJ.begin(); it != headersJ.end(); ++it) {
			headers.insert({ it.key(), it.value() });
		}

		Result proxiedRes;
		if (method == "GET") {
			proxiedRes = client.Get(path, headers);
		} else if (method == "POST") {
			proxiedRes = client.Post(path, headers, rBody, "application/json");
		} else if (method == "PUT") {
			proxiedRes = client.Put(path, headers, rBody, "application/json");
		} else if (method == "DELETE") {
			proxiedRes = client.Delete(path, headers, rBody, "application/json");
		} else if (method == "PATCH") {
			proxiedRes = client.Patch(path, headers, rBody, "application/json");
		} else {
			res.status = 400;
			res.set_content(R"({"error":"Unsupported HTTP method"})", "application/json");
			return;
		}

		if (proxiedRes) {
			json responseJ;
			responseJ["c"] = proxiedRes->status;
			responseJ["r"] = proxiedRes->reason;
			responseJ["v"] = proxiedRes->version;

			json rHeadersJ;
			for (const auto& header : proxiedRes->headers) {
				rHeadersJ[header.first] = header.second;
			}
			responseJ["h"] = rHeadersJ;

			const auto contentType = proxiedRes->get_header_value("Content-Type");
			if (contentType.find("application/json") == std::string::npos &&
				contentType.find("text/") == std::string::npos) { // convert binary files to base 64
				responseJ["b"] = base64::to_base64(proxiedRes->body);
				responseJ["b64"] = true;
			} else {
				responseJ["b"] = proxiedRes->body;
			}

			res.status = 200;
			res.set_content(responseJ.dump(), "application/json");
		} else {
			res.status = 500;
			res.set_content(R"({"error":"Failed to reach target server"})", "application/json");
		}
		return;
	}

	if (cType == "qtp") { // queue on teleport
		if (!body.contains("pid") /*username*/ || !body.contains("t") /*type*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string pid = body["pid"];
		const std::string type = body["t"];

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				if (type == "g") { // get
					res.status = 200;
					res.set_content(client->TeleportQueue, "text/plain");
					return;
				} else if (type == "s") { // set

					if (!body.contains("ct")) {
						res.status = 400;
						res.set_content(R"({"error":"Missing required fields"})", "application/json");
						return;
					}
					const std::string source = body["ct"];

					res.status = 200;
					client->TeleportQueue = source;
					return;
				}
				res.status = 400;
				res.set_content(R"({"error":"Invalid queue on teleport request"})", "application/json");
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	if (cType == "btc") { // get script bytecode
		if (!body.contains("pid") /*process id*/ || !body.contains("cn") /*container name*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string pid = body["pid"];
		const std::string containerName = body["cn"];

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				const std::string bytecode = client->GetBytecode(containerName);

				res.status = 200;
				res.set_content(bytecode, "text/plain");
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	if (cType == "rc") { // rconsole
		if (!body.contains("t") /*console type*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string type = body["t"];

		if (type == "cls") { // clear
			if (console_active) {
				system("cls");
			}
			return;
		}

		if (type == "crt") { // create
			activate_console();
			return;
		}

		if (type == "dst") { // destroy
			if (console_active) {
				console_active = false;
				FreeConsole();
			}
			return;
		}

		if (!body.contains("ct") /*content*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		if (type == "prt") { // print
			activate_console();
			std::string text = body["ct"];
			text = base64::from_base64(text); // use base64 so the entire text can be shown without null termination
			std::cout << text << std::endl;
		}

		if (type == "ttl") { // title
			const std::string title = body["ct"];
			SetConsoleTitleA(title.c_str());
		}

		res.status = 200;
		return;
	}

	if (cType == "ax") { // get autoexec contents
		std::string content;
		std::filesystem::path xenoDir = std::filesystem::current_path().parent_path();
		std::filesystem::path autoexecDir = xenoDir / "autoexec";
		if (!std::filesystem::exists(autoexecDir)) {
			std::filesystem::create_directory(autoexecDir);
		}
		if (!std::filesystem::is_directory(autoexecDir)) {
			std::filesystem::remove(autoexecDir);
			std::filesystem::create_directory(autoexecDir);
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(autoexecDir)) {
			if (!entry.is_regular_file())
				continue;

			std::ifstream file(entry.path());
			if (file.is_open()) {
				std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				if (compilable(file_contents) == "success") {
					content += "task.spawn(function(...)" + file_contents + "\nend)\n";
				}
				file.close();
			}
		}

		res.status = 200;
		res.set_content(content, "text/plain");
		return;
	}

	if (cType == "hw") { // get hwid
		std::string hwid;
		HW_PROFILE_INFO hwProfileInfo;

		if (GetCurrentHwProfile(&hwProfileInfo)) {
			std::wstring wHWID = hwProfileInfo.szHwProfileGuid;
			hwid = std::string(wHWID.begin(), wHWID.end());
		}

		res.status = 200;
		res.set_content(hwid, "text/plain");
		return;
	}

	if (cType == "adr") { // get instance container value ptr
		if (!body.contains("cn") /*instance container name*/ || !body.contains("pid") /*process id*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string containerName = body["cn"];
		const std::string pid = body["pid"];

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				const std::uintptr_t address = client->GetObjectValuePtr(containerName);

				res.status = 200;
				res.set_content(std::to_string(address), "text/plain");
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	if (cType == "spf") { // spoof instance
		if (!body.contains("cn") /*instance container name*/ || !body.contains("pid") /*process id*/ || !body.contains("adr") /*new address*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string containerName = body["cn"];
		const std::string pid = body["pid"];
		const std::string newAddressStr = body["adr"];
		const std::uintptr_t newAddress = static_cast<std::uintptr_t>(std::stoull(newAddressStr));

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				client->SpoofInstance(containerName, newAddress);

				res.status = 200;
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	if (cType == "clt") { // is client loaded
		if (!body.contains("gd") /*guid*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string guid = body["gd"];

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (client->GUID == guid) {
				res.status = 200;
				res.set_content(std::to_string(client->PID), "text/plain");
				return;
			}
		}

		res.status = 400;
		return;
	}

	if (cType == "gb") { // globals
		if (!body.contains("t") /*type*/ || !body.contains("n") /*global name*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string type = body["t"];
		const std::string gName = body["n"];
		if (type == "s") { // set
			if (!body.contains("v") /*value*/ || !body.contains("vt") /*value type*/) {
				res.status = 400;
				res.set_content(R"({"error":"Missing required fields"})", "application/json");
				return;
			}

			const std::string val = body["v"];
			const std::string valType = body["vt"];

			Globals[gName] = { {"d", val}, {"t", valType} };

			res.status = 200;
			return;
		} else if (type == "g") { // get
			res.status = 200;
			if (Globals.find(gName) != Globals.end()) {
				res.set_content(Globals[gName].dump(), "application/json");
				return;
			} else {
				res.set_content(R"({"d":null,"t":null})", "application/json");
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Invalid globals request"})", "application/json");
		return;
	}

	if (cType == "um") { // unlock module
		if (!body.contains("cn") /*instance container name*/ || !body.contains("pid") /*process id*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string containerName = body["cn"];
		const std::string pid = body["pid"];

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				client->UnlockModule(containerName);

				res.status = 200;
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	if (cType == "prp") { // get properties
		if (!body.contains("cn") /*instance container name*/ || !body.contains("pid") /*process id*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}

		const std::string containerName = body["cn"];
		const std::string pid = body["pid"];

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				json Properties = client->GetProperties(containerName);

				res.status = 200;
				res.set_content(Properties.dump(), "application/json");
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	}

	res.status = 400;
	res.set_content(R"({"error":"Invalid value for the field 'c'"})", "application/json");
};

void setup_connection()
{
	Server svr;
	if (!svr.is_valid()) {
		std::cerr << "Server could not be started\n";
		return;
	}

	std::filesystem::path mainDir;
	std::filesystem::path workspaceDir;

	char mainPath[MAX_PATH];
	GetModuleFileNameA(NULL, mainPath, MAX_PATH);

	mainDir = std::filesystem::path(mainPath).parent_path();
	workspaceDir = mainDir / "workspace";
	if (!std::filesystem::exists(workspaceDir)) {
		std::filesystem::create_directory(workspaceDir);
	}
	if (!std::filesystem::is_directory(workspaceDir)) {
		std::filesystem::remove(workspaceDir);
		std::filesystem::create_directory(workspaceDir);
	}

	std::filesystem::current_path(workspaceDir);

	svr.Post("/send", [](const Request& req, Response& res) {
		json body;
		try {
			body = json::parse(req.body);
		} catch (json::parse_error&) {
			res.status = 400;
			res.set_content(R"({"error":"Invalid JSON"})", "application/json");
			return;
		}

		if (body.empty()) {
			res.status = 400;
			res.set_content(R"({"error":"Empty JSON"})", "application/json");
			return;
		}

		if (!body.contains("c")) {
			res.status = 400;
			res.set_content(R"({"error":"Missing 'c' field"})", "application/json");
		}
		serve(res, body);
	});

	svr.Post("/writefile", [](const Request& req, Response& res) {
		if (!req.has_param("p") /*path*/ || req.body.empty() /*content*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string path = req.get_param_value("p");
		const std::string_view content = req.body;

		if (!withinDirectory(std::filesystem::current_path(), path)) {
			res.status = 400;
			res.set_content(R"({"error":"Attempt to escape directory"})", "application/json");
			return;
		}

		if (std::filesystem::is_directory(path)) {
			res.status = 400;
			res.set_content(R"({"error":"Given path is a directory and not a file"})", "application/json");
			return;
		}

		std::ofstream file(path, std::ios::trunc | std::ios::binary);
		if (!file) {
			res.status = 500;
			res.set_content(R"({"error":"Could not open the given file"})", "application/json");
			return;
		}

		file.write(content.data(), content.size());
		file.close();

		res.status = 200;
		return;
	});

	svr.Post("/compilable", [](const Request& req, Response& res) {
		if (req.body.empty() /*source*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string source = req.body;
		bool returnBytecode = false;

		if (req.has_param("btc"))
			returnBytecode = true;

		res.status = 200;
		res.set_content(compilable(source, returnBytecode), "text/plain");
	});

	svr.Post("/loadstring", [](const Request& req, Response& res) {
		if (!req.has_param("n") /*script name*/ || !req.has_param("pid") /*process id*/ || !req.has_param("cn") /*chunk name*/ || req.body.empty() /*source*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string source = req.body;
		const std::string scriptName = req.get_param_value("n");
		const std::string pid = req.get_param_value("pid");
		const std::string chunkName = req.get_param_value("cn");

		std::lock_guard<std::mutex> lock(clientsMtx);
		for (const auto& client : Clients) {
			if (std::to_string(client->PID) == pid) {
				bool success = client->loadstring(source, scriptName, chunkName);
				if (!success) {
					res.status = 400;
					res.set_content(R"({"error":"An error occurred with loadstring"})", "application/json");
				}

				res.status = 200;
				return;
			}
		}

		res.status = 400;
		res.set_content(R"({"error":"Client with the given PID was not found"})", "application/json");
		return;
	});

	svr.Post("/setclipboard", [](const Request& req, Response& res) {
		if (req.body.empty() /*content*/) {
			res.status = 400;
			res.set_content(R"({"error":"Missing required fields"})", "application/json");
			return;
		}
		const std::string content = req.body;

		if (!OpenClipboard(nullptr)) {
			res.status = 400;
			res.set_content(R"({"error":"Failed to open clipboard"})", "application/json");
			return;
		}

		EmptyClipboard();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, content.size() + 1);

		if (!hMem) {
			res.status = 400;
			res.set_content(R"({"error":"Failed to set clipboard | 1"})", "application/json");
			return;
		}

		char* pMem = static_cast<char*>(GlobalLock(hMem));
		if (!pMem) {
			GlobalFree(hMem);
			CloseClipboard();

			res.status = 400;
			res.set_content(R"({"error":"Failed to set clipboard | 2"})", "application/json");
			return;
		}

		std::copy(content.begin(), content.end(), pMem);
		pMem[content.size()] = '\0';
		GlobalUnlock(hMem);
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();

		res.status = 200;
		return;
	});

	svr.set_exception_handler([](const Request& req, Response& res, std::exception_ptr ep) {
		std::string errorMessage;
		try {
			std::rethrow_exception(ep);
		} catch (std::exception& e) {
			errorMessage = e.what();
		} catch (...) {
			errorMessage = "Unknown Exception";
		}
		res.set_content("{\"error\":\"" + errorMessage + "\"}", "application/json");
		res.status = 500;
	});

	svr.listen("localhost", 19283);
}