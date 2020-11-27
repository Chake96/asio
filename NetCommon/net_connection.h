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
			void disconnect() noexcept;
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
			net::message<T> _msg_buffer; //buffer for asio reading in _inbound_queue
			owner_type owner{owner_type::server}; 

			uint32_t id = 0;
	};

	using namespace net;
	
	template<class T>
	void connection<T>::connect_to_client(uint32_t uid) noexcept{
		if (this->owner == owner_type::server && this->socket_.is_open()) {
			this->id = uid;
		}
		
	}

	template<class T>
	void connection<T>::send(const net::message<T>& msg) {

	}

}


#endif