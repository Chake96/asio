#pragma once

#ifndef ASIO_NET_SERVER_H_
#define ASIO_NET_SERVER_H_

#include "net_includes.h"
 

namespace net {

	template<class T>
	class net_server {
		public:
			net_server(uint16_t port):_acceptor(this->context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port) {

			}


			virtual ~net_server() {
				this->stop();
			}

			int start() {
				
				try {
					this->wait_for_client();

					this->_thread_context = std::thread([]() {
						this->_context.run();
					})
				}
				catch (std::exception& err) {
					std::cerr << "Server Exception: " << err.what() << "\n";
					return EXIT_FAILURE;
				}
				//to-do, define verbrosity command line argument to determine if server ever prints to console
				std::cout << "Server Start was Successful\n";
				return EXIT_SUCCESS;
			}


			void stop() {
				this->_context.stop();

				if(this->_thread_context.joinable())
					this->_thread_context.join();

				std::cout << "Server Stop was Successful. Goodbye.";
			}

			//asynchronous wait 
			void wait_for_client() {
				this->acceptor.async_accept([](std::error_code ec, asio::ip::tcp::socket socket) {
					if (!ec) {
						std::cout << "New Connection from: " << socket.remote_endpoint() << "\n";

						std::shared_ptr<net::connection<T>> new_connection = std::make_shared<net::connection<T>>(
																				net::connection<T>::owner::server,
																				this->_context, 
																				std::move(socket), 
																				this->_inbound_msg_queue
																				);

						if (this->client_connect_handle(new_connection)) {
							this->_active_connections.push_back(std::move(new_connection));
							this->_active_connections.back()->

						}
						else {
							std::cout << "Connection was declined by [Server]\n";
						}
					}
					else {
						std::cout << "Connection to new client failed: " << ec.what() << '\n';
					}

					wait_for_client();//keep the context busy
					});

			}

			void message_client(std::shared_ptr<net::connection<T>> client_connection, const net::message<T>& msg) {

			}

			void message_all(const net::message<T>& msg, 
							 const std::vector<std::shared_ptr<net::connection<T>>> client_blacklist) {


			}



		protected:

			uint32_t UUID_counter = 10000; //10000 ids for connection

			//holds all incoming message packets
			net::message_queue<net::shared_message<T>> _inbound_msg_queue;

			asio::io_context _context;
			std::thread _thread_context;

			//rely on _context
			asio::ip::tcp::acceptor _asio_acceptor; //listens for new TCP connections

			virtual bool client_connect_handle(std::shared_ptr<net::connection<T>> client) {

			}

			virtual bool client_disconnect_handle(std::shared_ptr<net::connection<T>> client) {


			}


			virtual bool received_message_handle(std::shared_ptr<net::connection<T>> client, message<T>& recv_msg) {

			}


		private:
			std::vector<std::shared_ptr<net::net_client<T>> _client_list;
			std::deque<std::shared_ptr<net::connection<T>>> _active_connections;

	};


}


#endif