#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/io_context.hpp> 
#include <boost/asio/error.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/format.h>
#include <fmt/color.h>

#ifdef _WIN32
	#define _WIN32_WINNT 0x0A00
#endif


using namespace boost;
using namespace std::chrono_literals;




struct address_info : public std::tuple<std::string, asio::ip::address, uint16_t>{
	std::string _name;
	asio::ip::address _addr;
	uint16_t _port;
	address_info(std::string n, asio::ip::address addr, uint16_t p): _name(n), _addr(addr), _port(p){};
};

template<>
struct fmt::formatter<address_info> {
	using unsigned_fmt = fmt::formatter<uint16_t>;
	using string_view_fmt = fmt::formatter<std::string_view>;
	constexpr auto parse(format_parse_context& context) {
		auto ctx_it = context.begin(), ctx_end = context.end();
		if(ctx_it != ctx_end && *ctx_it != '}')
			throw format_error("Invalid format");

		return ctx_it;
	}

	template<class FormatContext>
	auto format(const address_info& ai, FormatContext& ctx) {
		return format_to(ctx.out(), "\nname: {}, ip: {}, port: {}\n", fmt::format(fmt::fg(fmt::color::lime_green), ai._name) ,
																  fmt::format(fmt::fg(fmt::color::orange), ai._addr.to_string()),
																  fmt::format(fmt::fg(fmt::color::lime_green), std::to_string(ai._port))
																  );
	}
};

void read_data(asio::ip::tcp::socket& socket, std::vector<char>& _buffer){//, std::vector<char> buffer) {
	socket.async_read_some(asio::buffer(_buffer.data(), _buffer.size()),
		[&](std::error_code ec, std::size_t length) {
			if (!ec) {
				std::cout << "\n\nReading in " << length << " bytes\n\n";
				//std::vector<char>::iterator buffer_it = buffer.begin();
				auto buffer_it = std::begin(_buffer);
				for (auto& value : _buffer) {
					std::cout << *buffer_it;
					buffer_it += 1;
				}
				read_data(socket, _buffer);
			}
	});
}


int main() {

	asio::io_context io_context(1);
	asio::io_context::work idle_work(io_context);
	
	std::thread thread_context = std::thread([&](){io_context.run();});

	boost::system::error_code ec;
	std::vector<char> _buffer(1024);


	//define all packet header variables
	//!TO-DO extract to a config file 
	std::string carsonhake_dev_ip = "185.199.108.153";
	std::string google_ip_addr = "172.217.1.36";
	asio::ip::address current_ipv4 = asio::ip::make_address(carsonhake_dev_ip);
	std::string current_name = "www.carsonhake.dev";
	uint16_t current_port = 80;
	auto current_address_info = address_info(current_name, current_ipv4, current_port);


	//create the asio tcp endpoint object
	asio::ip::tcp::endpoint endpoint;
	endpoint.address(current_address_info._addr);
	endpoint.port(current_address_info._port);


	asio::ip::tcp::socket socket(io_context);//tcp socket

	//attempt connection with endpoint
	socket.connect(endpoint, ec);
	if (ec) { //no error in connection attempt
		fmt::print("Failed to Connect to: {}", current_address_info);

		//std::cout << "Failed to connect to address:\n" << ec.message() << '\n';
	}
	else {//error in connection attempt
		fmt::print("Successful Connection to: {}", current_address_info);
		
		
		std::string http_request = "Get /index.html HTTP/1.1\r\nHost: carsonhake.dev\r\n"
									"Connction: close\r\n\r\n";
		if (socket.is_open()) {
			read_data(socket, _buffer);

			socket.write_some(asio::buffer(http_request.data(), http_request.size()), ec);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(2000ms);
			io_context.stop();
			if (thread_context.joinable()) {
				thread_context.join();
			}
		}
		else {
			fmt::print(fmt::fg(fmt::color::red), "socket is not open");
		}
	}

	

	std::exit(0);
}