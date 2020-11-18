#pragma once



#ifndef ASIO_NET_CLIENT_H_
#define ASIO_NET_CLIENT_H_

#include "net_common.h"


namespace net {

	template<class T>
	class net_client {
		public:
			net_client():_socket(this->_context){}

			virtual ~net_client() {
				this->disconnect();
			}

			void connect(const std::string& host_name, const uint16_t port) {
				try {
					this->_connection = std::make_unique<net::connection<T>>();
					
					//use a resolver to establish a list of TCP endpoints
					asio::ip::tcp::resolver resolve(this->_context);
					end_points = resolver.resolve(host_name, std::to_string(port));

					//establish the connection
					this->_connection->connect(end_points);

					//begin the context thread
					this->_thread_context = std::thread([]() {
							this->_context.run();
						})

				}
				catch (std::exception& err) {
					std::cerr << "Client Exception: " << err.what() << '\n';
				}

			}

			void disconnect() {
				if (this->is_connected()) {
					this->_connection->disconnect();
				}

				this->_context.stop();
				if (this->_thread_context.joinable()) {
					_thread_context.join();
				}

				this->_connection->release();
			}

			bool inline is_connected(){
				bool connection_status = false;
				if (_connection) {
					connection_status = _connection->is_connected();
				}
				return connection_status;
			}

		protected:
			asio::io_context _context;
			std::thread _thread_context;
			std::unique_ptr<net::connection<T>> _connection;

		private:
			message_queue<net::shared_message<T>> _inbound_msg_queue;
	};

}


#endif 