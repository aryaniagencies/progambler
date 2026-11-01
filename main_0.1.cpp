#include "webview.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <nlohmann/json.hpp>

#include <fstream>

#define SOCKET_PATH "/tmp/gambler.sock"

// Shorthand ke liye
using json = nlohmann::json;
using namespace std;

typedef void (*func)(string);
func vtable[256];

vector<string> files;

webview::webview w(true, nullptr);

ifstream filelist;

void handle_login(const uint8_t* data, size_t size) {
	std::cout << "[Cmd 0] Login handler called. Data size: " << size << " bytes\n";
	// Data ko apne struct/format me cast kar lo:
	// auto* login_packet = reinterpret_cast<LoginStruct*>(data);
}

void handle_save_file(const uint8_t* data, size_t size) {
	std::cout << "[Cmd 1] Save file handler called!\n";
}

int startserv(webview::webview* w) {

	// Purani socket file agar bachi ho toh delete kar do
	unlink(SOCKET_PATH);

	// 1. Socket create karo
	int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd == -1) {
		std::cerr << "Socket creation failed!\n";
		return 1;
	}

	// 2. Address bind karo
	struct sockaddr_un address;
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == -1) {
		std::cerr << "Bind failed!\n";
		close(server_fd);
		return 1;
	}
	if (listen(server_fd, 5) == -1) {
		std::cerr << "Listen failed!\n";
		close(server_fd);
		return 1;
	}

	std::cout << "[Server] Listening on Unix socket: " << SOCKET_PATH << "\n";

	while(true) {
		int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd == -1) break;

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

		if (bytes_read > 0) {
			std::string msg(buffer);

			// ⚠️ Zaroori: UI/Webview ko thread-safe tarike se dispatch karein
			w->dispatch([&w, msg]() {
					std::string js_code = "updateUI('" + msg + "');";
					w->eval(js_code);
					});
		}	
	// Cleanup
	close(client_fd);
	close(server_fd);
	unlink(SOCKET_PATH);
}	
	return 0;
}

void gamble(string data) {
	// generate a random number
	srand(std::chrono::system_clock::now().time_since_epoch().count());
        string path = files[rand()%47830];
        int width, height;

	path = "/mnt/f" + path.substr(2);
	replace(path.begin(), path.end(), '\\', '/');

	// send data back to front end
	cout << path << endl;
	// w.dispatch([&w, msg]() { std::string js_code = "updateUI('mainimage', '"+filePath+"');"); w.eval(js_code);});

}

int main(){
	vtable[0]=gamble;
	filelist.open("images.txt");
	int count;
        string temp;	

	while(std::getline(filelist, temp)) {
                files.push_back(temp);
        }
        filelist.close();

	w.set_title("Single Entry Dispatcher");
	w.set_size(800, 600, WEBVIEW_HINT_NONE);

	// 2. Sirf EK hi single bind poori application ke liye!
	w.bind("dispatch", [&](const std::string &seq, const std::string &req, void *arg){
			if (req.empty()) return "";

			// cout << req;
			json temp=json::parse(req);
			int cmd_id = temp[0];
			string data=temp[1];
			// 2nd byte se aage ka data (Payload) pass karo
			// data[strlen(data)-2]='\0';
			vtable[cmd_id](data);
			return "{\"status\": \"ok\"}";
			}, nullptr);

	w.navigate("file://"+string(getenv("PWD"))+"/frontend/index.html");

	thread server(startserv, &w);
	server.detach();

	w.run();
	return 0;
}
