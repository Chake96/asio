#pragma once


#include "message_queue.h"
#include "net_message.h"

#ifndef ASIO_NET_CONNECTION_H_
#define ASIO_NET_CONNECTION_H_

namespace net {
	namespace {
		enum class owner_type{server, client};
	}

	template<class T>
	class connection : public std::enable_shared_from_this<connection<T>> {
		public:
			
			//ctors, dtors
			connection(){}
			virtual ~connection(){}

			//utility functions
			
			//connection manipulation
			void connect() noexcept;
			void disconnect() noexcept;
			const bool is_connected() noexcept; 

			void 


			//message utilites
			void send(const net::message<T>& msg);

		protected:
			asio::ip:tcp::socket _socket;
			asio::io_context* _context;
			net::message_queue<net::message<T>> _outbound_queue;
			net::message_queue<net::message<T>> _inbound_queue;
			net::message<T> _msg_buffer; //buffer for asio reading in _inbound_queue
			owner_type owner{owner_type::server};

			uint32_t id = 0;
	};

}


#endif