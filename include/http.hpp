#pragma once
#include <unordered_set>

#include <asyncio.hpp>

#include "http/types.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
using namespace kwa;


namespace http {

asyncio::Task<> init_connection(int conn) noexcept;

}
