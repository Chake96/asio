#pragma once

#ifndef ASIO_MESSAGE_QUEUE_H_
#define ASIO_MESSAGE_QUEUE_H_

#include "net_common.h"

namespace net{
		
	template<class T>
	class message_queue{
		public:
			message_queue() = default;
			message_queue(const message_queue<T>&) = delete;
			virtual ~message_queue(){this->clear();}

			const auto begin() noexcept{
				return this->dqueue_.begin();
			}

			const auto end() noexcept{
				return this->dqueue_.end();
			}

			const T& front() { //read-only
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->dqueue_.front();
			}
			
			const T& back() { //read-only
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->dqueue_.back();
			}

			void pop_front() { //write
				std::scoped_lock queue_lock(this->queue_mutex);
				this->dqueue_.pop_front();
			}

			void pop_back() {
				std::scoped_lock queue_lock(this->queue_mutex);
				this->dqueue_.pop_back();
			}

			void push_front(T& value) {
				_write_call(std::deque<T>::push_front, value);
			}

			void push_back(T value) {
				this->write_call(value, &std::deque<T>::push_back);
			}


			void clear() noexcept {
				std::scoped_lock queue_lock(this->queue_mutex);
				this->dqueue_.clear();
			}

			size_t size() {
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->dqueue_.size();
			}

			bool empty() {
				std::scoped_lock queue_lock(this->queue_mutex);
				return this->dqueue_.empty();
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
			void read_call(void(std::deque<T>::*mem_func)(),  T* return_value) {
				std::scoped_lock queue_lock(this->queue_mutex);
				*return_value = (this->dqueue_.*mem_func)();
			}

			//calls the function, does not return anything
			void read_call(void(std::deque<T>::* mem_func)()) {
				std::scoped_lock queue_lock(this->queue_mutex);
				(this->dqueue_.*mem_func)();
			}

			void write_call(T& input_val, void(std::deque<T>::* mem_func)(const T&)) {
				std::scoped_lock queue_lock(this->queue_mutex);
				(this->dqueue_.*mem_func)(input_val);
				std::unique_lock<std::mutex> u_lock(this->blocking_mutex);
				blocking_var.notify_one();
			}

		private:
			std::deque<T> dqueue_;

	};
}

#endif