#pragma once



#ifndef ASIO_NET_CLIENT_H_
#define ASIO_NET_CLIENT_H_

#include "net_common.h"
using namespace boost;


namespace net {

	template<class T>
	class net_client {
		public:
			net_client():context_(this->context_){}

			virtual ~net_client() {
				this->disconnect();
			}

			void connect(const std::string& host_name, const uint16_t port) {
				try {
					this->connection_ = std::make_unique<net::connection<T>>();
					
					//use a resolver to establish a list of TCP endpoints
					asio::ip::tcp::resolver resolve(this->context_);
					asio::ip::tcp::resolver::results_type end_points = resolve.resolve(host_name, std::to_string(port));

					//establish the connection
					this->connection_->connect(end_points);

					//begin the context thread
					this->thread_context_ = std::thread([]() {
							this->context_.run();
						});

				}catch (std::exception& err) {
					std::cerr << "Client Exception: " << err.what() << '\n';
				}

			}

			void disconnect() {
				if (this->is_connected()) {
					this->connection_->disconnect();
				}

				this->context_.stop();
				if (this->thread_context_.joinable()) {
					thread_context_.join();
				}

				this->connection_.release();
			}

			bool inline is_connected(){
				bool connection_status = false;
				if (connection_) {
					connection_status = connection_->is_connected();
				}
				return connection_status;
			}

		protected:
			asio::io_context context_;
			std::thread thread_context_;
			std::unique_ptr<net::connection<T>> connection_;

		private:
			message_queue<net::shared_message<T>> _inbound_msg_queue;
	};

}


#endif 