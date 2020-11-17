#pragma once

#include "net_common.h"



namespace net{
	template<class T>
	struct message_header {
		T id{};
		size_t size = 0;
	};

	template<class T>
	struct message {
		message_header<T> header{};
		std::vector<uint8_t> body; //made up of bytes

		const size_t size() {
			return sizeof(message_header<T>) + (body.capacity()*sizeof(body.at(0)));
		}

		friend std::ostream& operator<< (std::ostream& os, const message<T>& msg) {
			os << "ID: " << msg.header.id << " Size: " << msg.size();
			return os;
		}

		//push data onto the message body
		template<class data_type>
		friend message<T>& operator<< (message<T>& msg, const data_type& data) {
			//static_assert(std::is_standard_layout<data_type>::value, "Data is too complex to be pushed into vector");
			if constexpr (std::is_standard_layout<data_type>::value) {
				// Cache current size of vector, as this will be the point we insert the data
				size_t i = msg.body.size();

				// Resize the vector by the size of the data being pushed
				msg.body.resize(msg.body.size() + sizeof(data_type));

				// Physically copy the data into the newly allocated vector space
				std::memcpy(msg.body.data() + i, &data, sizeof(data_type));

				// Recalculate the message size
				msg.header.size = msg.size();

				// Return the target message so it can be "chained"
			}
			return msg;

		}

		//pop data from the message body
		template<class data_type>
		friend message<T>& operator>> (message<T>& msg, data_type& data) {
			if constexpr (std::is_standard_layout<data_type>::value) {
				size_t i = msg.body.size() - sizeof(data_type);

				std::memcpy(&data, msg.body.data() + i, sizeof(data_type));

				msg.body.shrink_to_fit();
				
				// Recalculate the message size
				msg.header.size = msg.size();

			}
			return msg;
		}
	};

}