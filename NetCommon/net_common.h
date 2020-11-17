#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <optional>
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>
#include <utility>


#include <fmt/core.h>

#ifdef _WIN32
	#define _WIN32_WINNT 0x0A00
#endif


#include <boost/asio.hpp>

#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <boost/asio/ts/io_context.hpp> 
#include <boost/asio/error.hpp>