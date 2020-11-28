#pragma once



#ifndef ASIO_NET_CLIENT_H_
#define ASIO_NET_CLIENT_H_

#include "net_common.h"
using namespace boost;


namespace net {

	template<class T>
	class net_client {
		public:
			net_client(){}

			virtual ~net_client() {
				this->disconnect();
			}

			/*Connection Handlers*/

			void connect(const std::string& host_name, const uint16_t port) {
				try {
					
					//use a resolver to establish a list of TCP endpoints
					asio::ip::tcp::resolver resolve(this->context_);
					asio::ip::tcp::resolver::results_type end_points = resolve.resolve(host_name, std::to_string(port));



					//establish the connection
					this->connection_ = std::make_unique<net::connection<T>>(net::owner_type::client, 
																			this->context_, 
																			asio::ip::tcp::socket(this->context_),
																			this->inbound_msg_queue_
																			);
					this->connection_->connect_to_server(end_points);

					//begin the context thread
					this->thread_context_ = std::thread([this]() {
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

			bool inline is_connected() {
				bool connection_status = false;
				if (connection_) {
					connection_status = connection_->is_connected();
				}
				return connection_status;
			}


			/*Data Handlers*/
			void send(const message<T>& msg) {
				if (this->is_connected()) {
					this->connection_->send(msg);
				}
			}

			//getter for the inbound message queue
			net::message_queue<shared_message<T>>& incoming() {
				return this->inbound_msg_queue_;
			}

			

		protected:
			asio::io_context context_; //handles data flow
			std::thread thread_context_; //used by the context_
			std::unique_ptr<net::connection<T>> connection_; //client's sole connection object, TO-DO: could possibly reimplement using a singleton

		private:
			message_queue<net::shared_message<T>> inbound_msg_queue_; //holds messages received from server, thread-safe
	};

}


#endif 