#pragma once
#ifndef ASIO_NET_CONNECTION_H_
#define ASIO_NET_CONNECTION_H_


#include "net_common.h"
#include "message_queue.h"
#include "net_message.h"

using namespace boost;
namespace net {

	enum class owner_type{server, client};

	template<class T>
	class connection : public std::enable_shared_from_this<connection<T>> {
		public:
			
			//ctors, dtors
			connection(owner_type owner_in, asio::io_context& context_in, boost::asio::ip::tcp::socket socket_in, net::message_queue<net::shared_message<T>>& msg_queue)
				: context_(context_in), socket_(std::move(socket_in)), inbound_queue_(msg_queue), owner(owner_in){};
			
			virtual ~connection(){};

			//utility functions
			
			//connection manipulation
			void connect_to_client(uint32_t uid=0) noexcept;
			void connect_to_server(const asio::ip::tcp::resolver::results_type& endpoints);
			void disconnect();
			inline const bool is_connected() noexcept{return this->socket_.is_open();}; 

			void client_disconnected(std::shared_ptr<const net::connection<T>> client) noexcept;


			//message utilites
			void send(const net::message<T>& msg);
			uint32_t get_id(){return this->id;};

		protected:
			asio::ip::tcp::socket socket_;
			asio::io_context& context_;
			net::message_queue<net::message<T>> outbound_queue_;
			net::message_queue<net::shared_message<T>>& inbound_queue_;
			net::message<T> msg_buffer_; //buffer for asio reading in _inbound_queue
			owner_type owner{owner_type::server}; 


			uint32_t id = 0;


		private:
			void read_header();
			void read_body();

			void write_header();
			void write_body();

			void add_to_msg_queue(); 


	};

	using namespace net;


	template<class T>
	void connection<T>::disconnect() {
		if (this->is_connected()) {
			asio::post(this->context_, [this](){this->socket_.close();});
		}
	}
	
	template<class T>
	void connection<T>::connect_to_client(uint32_t uid) noexcept{
		if (this->owner == owner_type::server && this->socket_.is_open()) {
			this->id = uid;
			this->read_header(); //priming the ASIO context here
		}
		
	}

	template<class T>
	void connection<T>::connect_to_server(const asio::ip::tcp::resolver::results_type& endpoints) {
		asio::async_connect(this->socket_, endpoints,
			[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
				if (!ec) {
					this->read_header();
				}
			});
	}

	template<class T>
	void connection<T>::send(const net::message<T>& msg) {
		//injecting work into the context
		asio::post(this->context_, 
			[this, msg]() {
				const bool write_message_flag = !this->outbound_queue_.empty();
				this->outbound_queue_.push_back(msg);
				if (write_message_flag == false) { //only add a task to the context if the queue is empty
					this->write_header();			//ensures ASIO does not execute out of order
				}
			});
	}

	//used to prime the ASIO context
	template<class T>
	void connection<T>::read_header() {
		asio::async_read(this->socket_, asio::buffer(&msg_buffer_.header, sizeof(net::message_header<T>)),
			[this](std::error_code ec, std::size_t msg_length) {
				if (!ec) {
					if (this->msg_buffer_.header.size > 0) { //check for a body in the message
						msg_buffer_.body.resize(msg_buffer_.header.size);
						this->read_body();
					}else { //add the header-only message to the message queue
						this->add_to_msg_queue();
					}

				}
				else {
					//to-do convert standard output write to logging
					std::cout << "[" << this->id << "] Failure to read Message Header.\n";
					this->socket_.close(); //close the connection
				}
			});
	}
	
	template<class T>
	void connection<T>::read_body() {
		asio::async_read(this->socket_, asio::buffer(msg_buffer_.body.data(), msg_buffer_.header.size),
			[this](std::error_code ec, std::size_t msg_length) {
				if (!ec) {
					this->add_to_msg_queue();
				}else {
					std::cout << "[" << this->get_id() << "] Failure to read Message Body.\n";
					this->socket_.close();
				}
			});
	}

	template<class T>
	void connection<T>::write_header() {
		
		asio::async_write(this->socket_, asio::buffer(&outbound_queue_.front().header, sizeof(net::message_header<T>)),
			[this](std::error_code ec, std::size_t msg_length) {
				if (!ec) {
					if (this->outbound_queue_.front().body.size() > 0) { //check for a body in the message
						this->write_body();
					} else { //add the header-only message to the message queue
						this->outbound_queue_.pop_front();
						if (!this->outbound_queue_.empty()) { //keep sending messages if its not empty
							this->write_header();
						}
					}

				} else {
					//to-do convert standard output write to logging
					std::cout << "[" << this->id << "] Failure to write Message Header.\n";
					this->socket_.close(); //close the connection
				}
			});
	}

	template<class T>
	void connection<T>::write_body() {
		asio::async_write(this->socket_, asio::buffer(msg_buffer_.body.data(), msg_buffer_.header.size),
			[this](std::error_code ec, std::size_t msg_length) {
			if (!ec) {
				this->outbound_queue_.pop_front();

				if (!this->outbound_queue_.empty()) { //send more messages if the queue is not empty
					this->write_header();
				}
			} else {
				std::cout << "[" << this->get_id() << "] Failure to read Message Body.\n";
				this->socket_.close();
			}
		});
	}


	

	template<class T>
	void connection<T>::add_to_msg_queue() {
		if (this->owner == net::owner_type::server) {
			this->inbound_queue_.push_back({this->shared_from_this(), this->msg_buffer_}); //taking advantage of enable_shared_from_this, allows us to safely generate the new shared pointer
		}else {
			this->inbound_queue_.push_back({nullptr, this->msg_buffer_});
		}
		//reprime the ASIO context
		this->read_header(); //wait for a new header
	}

}


#endif