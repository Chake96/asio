#pragma once

#ifndef ASIO_MESSAGE_QUEUE_H_
#define ASIO_MESSAGE_QUEUE_H_

#include "net_common.h"
#include <boost/noncopyable.hpp>

namespace net{
		
	template<class T>
	class message_queue : boost::noncopyable{
		public:
			message_queue() = default;

			const auto begin() noexcept{
				return this->_dqueue.begin();
			}

			const auto end() noexcept{
				return this->_dqueue.end();
			}

			const T& front() { //read-only
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->_dqueue.front();
			}
			
			const T& back() { //read-only
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->_dqueue.back();
			}

			void pop_front() { //write
				std::scoped_lock queue_lock(this->queue_mutex);
				this->_dqueue.pop_front();
			}

			void pop_back() {
				std::scoped_lock queue_lock(this->queue_mutex);
				this->_dqueue.pop_back();
			}

			void push_front(T& value) {
				_write_call(std::deque<T>::push_front, value);
			}

			void push_back(T value) {
				_write_call(value, &std::deque<T>::push_back);
			}


			void clear() noexcept {
				std::scoped_lock queue_lock(this->queue_mutex);
				this->_dqueue.clear();
			}

			size_t size() {
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->_dqueue.size();
			}

			bool empty() {
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->_dqueue.empty();
			}

			void wait() {
				while (this->empty()) {
					std::unique_lock<std::mutex> u_lock(this->block_mutex);
					this->blocking_var.wait(u_lock);
				}
			}

		protected:
			std::mutex queue_mutex;
			std::mutex blocking_mutex;
			std::condition_variable blocking_var;

			//possibly convert to an std::optional return value
			void _read_call(void(std::deque<T>::*mem_func)(),  T* return_value) {
				std::scoped_lock queue_lock(this->queue_mutex);
				*return_value = (this->_dqueue.*mem_func)();
			}

			//calls the function, does not return anything
			void _read_call(void(std::deque<T>::* mem_func)()) {
				std::scoped_lock queue_lock(this->queue_mutex);
				(this->_dqueue.*mem_func)();
			}

			void _write_call(T& input_val, void(std::deque<T>::* mem_func)(const T&)) {
				std::scoped_lock queue_lock(this->queue_mutex);
				(this->_dqueue.*mem_func)(input_val);
				std::unique_lock<std::mutex> u_lock(this->blocking_mutex);
				blocking_var.notify_one();
			}

		private:
			std::deque<T> _dqueue;

	};
}

#endif