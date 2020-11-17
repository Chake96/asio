#include <iostream>

#include <net_includes.h>

enum class CustomMessageTypes :uint32_t {
	ERROR_MESSAGE = 0,
	CNTRL_MESSAGE = 1,
	INFO_MESSAGE = 2
};

int main() {

	net::message<CustomMessageTypes> current_msg;
	current_msg.header.id = CustomMessageTypes::CNTRL_MESSAGE;

	uint16_t a = 1;
	uint32_t b = 2;
	char temp = 't';
	struct {
		float x;
		float y;
	} d[5];

	current_msg << a << b << temp << d;

	net::message_queue<int> msgs;
	int j = 10;
	for (int i = 0; i < 10; i++) {
		msgs.push_back(i-100);
	}

	for(auto i: msgs)
		std::cout << i << ' ';
	std::cout << std::endl;

	std::exit(0);
}