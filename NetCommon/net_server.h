#pragma once

#ifndef ASIO_NET_SERVER_H_
#define ASIO_NET_SERVER_H_

#include "net_common.h"
#include "net_client.h"
#include "net_message.h"
#include "net_connection.h"
using namespace boost::asio;


namespace net {

	template<class T>
	class net_server {
		public:
			net_server(uint16_t port):acceptor_(this->context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
			}


			virtual ~net_server() {
				this->stop();
			}

			bool start();
			void stop();

			//asynchronous wait 
			void wait_for_client();

			void message_client(std::shared_ptr<net::connection<T>> client_connection, const net::message<T>& msg);
			void message_all(const net::message<T>& msg);

			void update(std::size_t n_max_msgs = -1);


		protected:

			uint32_t UUID_counter = 10000; //10000 ids for connection

			//holds all incoming message packets
			net::message_queue<net::shared_message<T>> inbound_msg_queue_;

			asio::io_context context_;
			std::thread thread_context_;

			//rely on context_
			asio::ip::tcp::acceptor acceptor_; //listens for new TCP connections

			virtual bool client_connect_handle(std::shared_ptr<net::connection<T>> client) {
				bool return_status = true;



				return return_status;
			}

			virtual bool client_disconnect_handle(std::shared_ptr<net::connection<T>> client) {
				bool return_status = true;



				return return_status;

			}


			virtual bool received_message_handle(std::shared_ptr<net::connection<T>> client, message<T>& recv_msg) {
				bool return_status = true;



				return return_status;
			}


		private:
		
			std::vector<std::shared_ptr<net::net_client<T>>> client_list;
			std::deque<std::shared_ptr<net::connection<T>>> active_connections_;

	};

	template<class T>
	bool net::net_server<T>::start() {

		try {
			this->wait_for_client();

			this->thread_context_ = std::thread([&]() {
				this->context_.run();
				});
		}
		catch (std::exception& err) {
			std::cerr << "Server Exception: " << err.what() << "\n";
			return EXIT_FAILURE;
		}
		//to-do, define verbrosity command line argument to determine if server ever prints to console
		std::cout << "Server Start was Successful\n";
		return EXIT_SUCCESS;
	}

	template<class T>
	void net::net_server<T>::stop(){
			this->context_.stop();

			if (this->thread_context_.joinable())
				this->thread_context_.join();

			std::cout << "Server Stop was Successful.\n Goodbye...";
	}

	template<class T>
	void net::net_server<T>::wait_for_client() {
		//create a new connection if we receive anything on the socket
		this->acceptor_.async_accept([&](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec) {
				std::cout << "New Connection from: " << socket.remote_endpoint() << "\n";

				//potential to replace with auto, flush out type deduction 
				std::shared_ptr<net::connection<T>> new_connection = std::make_shared<net::connection<T>>(
					net::owner_type::server,
					this->context_,
					std::move(socket),
					this->inbound_msg_queue_
					);

				if (this->client_connect_handle(new_connection)) {
					this->active_connections_.push_back(std::move(new_connection));
					this->active_connections_.back()->connect_to_client(this->UUID_counter++);

					//to-do log new accepted connection
					std::cout << "[" << this->active_connections_.back()->get_id() << "] Connection Approved\n";
				}
				else {
					std::cout << "Connection was declined by [Server]\n";
				}
			}
			else {
				std::cout << "Connection to new client failed: " << ec.message() << '\n';
			}

			this->wait_for_client();//keep the context busy
			});

	}

	template<class T>
	void net::net_server<T>::message_client(std::shared_ptr<net::connection<T>> client_connection, const net::message<T>& msg) {
		//check validty of the client
		if (client_connection && client_connection->is_connected())
		{
			client_connection->send(msg);
		}
		else
		{
			this->client_disconnect_handle(client_connection);
			client_connection->reset();
			this->active_connections_.erase(
				std::remove(this->active_connections_.begin(), this->active_connections_.end(), client_connection),
				this->active_connections_.end()
			);

		}
	}

	template<class T>
	void net::net_server<T>::message_all(const net::message<T>& msg) {
		bool client_error = false; //a flag to determine whether the
		for (auto& client_connection : this->active_connections_)
		{
			if (client_connection && client_connection->is_connected())
			{
				client_connection->send(msg);
			}
			else
			{
				this->client_disconnect_handle(client_connection);
				client_connection.reset();
				client_error = true;
			}
		}
		if (client_error)
		{
			this->active_connections_.erase(
				std::remove(this->active_connections_.begin(), this->active_connections_.end(), nullptr), this->active_connections_.end()
			);
		}

	}

	template<class T>
	void net::net_server<T>::update(std::size_t n_max_msgs) {
		std::size_t n_msgs_recvd = 0;

		while (n_msgs_recvd < n_max_msgs && !this->inbound_msg_queue_.empty()) {
			auto msg = this->inbound_msg_queue_.front();
			this->inbound_msg_queue_.pop_front();
			this->received_message_handle(msg.remote, msg.message);
			n_msgs_recvd += 1;
		}
	}
}

#endif