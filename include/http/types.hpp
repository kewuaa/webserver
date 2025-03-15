#pragma once
#include <magic_enum/magic_enum.hpp>
#define CRLF "\r\n"


namespace http {

enum class Method {
    GET,
    POST,
};


enum class StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404
};


constexpr const char* status_message(StatusCode code) noexcept {
    switch (code) {
        case StatusCode::OK: {
            return "OK";
        }
        case StatusCode::BAD_REQUEST: {
            return "Bad Request";
        }
        case StatusCode::FORBIDDEN: {
            return "Forbidden";
        }
        case StatusCode::NOT_FOUND: {
            return "Not Found";
        }
    }
    std::unreachable();
}

}
