#include <iostream>
#include <chrono>
#include <array>

#include <net_includes.h>
#include <net_client.h>

enum class NetworkMessageTypes :uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage
};

class SimpleClient : public net::net_client<NetworkMessageTypes> {
public:
	void ping_server() {
		net::message<NetworkMessageTypes> message{ NetworkMessageTypes::ServerPing };
		char time_buffer[20];
		auto time_point = std::chrono::system_clock::now(); //to-do serialize time_point as string

		message << time_point;
		this->send(message);
	}

	void message_all() {

	}
};


int main() {
	SimpleClient client;
	client.connect("127.0.0.1", 60000);

	//console input variables
	bool quit_flag = false;
	constexpr std::size_t console_key_size = 3;
	std::array<bool, console_key_size> key = { false,false,false }; //keys: 0,1, and 2
	std::array<bool, console_key_size> pkey_state = { false,false,false }; //tracks previous key states
	while (quit_flag == false) {
		//to-do implement cross-platform asynchronous capabilities
		if (GetForegroundWindow() == GetConsoleWindow()) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[0] && !pkey_state[0]) client.ping_server();
		if (key[1] && !pkey_state[1]) client.message_all();
		if (key[2] && !pkey_state[2]) quit_flag = true;

		for (std::size_t key_i = 0; key_i < console_key_size; key_i++) {
			pkey_state[key_i] = key[key_i];
		}

		if (client.is_connected()) {

			if(client.incoming().empty() == false){
				auto message = client.incoming().front().message;
				client.incoming().pop_front();

				if (message.header.id == NetworkMessageTypes::ServerAccept) {
					std::cout << "Connection Succesfully Established with Server\n";
				} else if (message.header.id == NetworkMessageTypes::ServerPing) {
					std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point time_then;
					message >> time_then;
					std::cout << "Ping Came Back\nRTT: " <<
								std::chrono::duration<double>(time_now - time_then).count() << '\n';
				} else {
					std::cout << "Message of Unknown Type received\n Responding to Sender";
					//to-do signal to sender that last message was illformed
				}

			}
		} else {
			std::cout << "Server Unreachable\n";
			quit_flag = true;
		}

	}
	std::exit(0);
}