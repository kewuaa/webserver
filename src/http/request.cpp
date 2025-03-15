#include <spdlog/spdlog.h>

#include "http/request.hpp"


namespace http::request {

Request::Request(Request&& data) noexcept:
    valid(data.valid),
    method(data.method),
    path(std::move(data.path)),
    params(std::move(data.params)),
    version(std::move(data.version)),
    headers(std::move(data.headers)),
    body(std::move(data.body)),
    body_size(std::exchange(data.body_size, 0)),
    _content_length(std::exchange(data._content_length, std::nullopt)),
    _content_type(std::exchange(data._content_type, std::nullopt))
{
    //
}

Request& Request::operator=(Request&& data) noexcept {
    valid = data.valid;
    method = data.method;
    path = std::move(data.path);
    params = std::move(data.params);
    version = std::move(data.version);
    headers = std::move(data.headers);
    body = std::move(data.body);
    body_size = std::exchange(data.body_size, 0);
    _content_length = std::exchange(data._content_length, std::nullopt);
    _content_type = std::exchange(data._content_type, std::nullopt);
    return *this;
}

const std::optional<size_t>& Request::content_length() noexcept {
    if (_content_length.has_value()) {
        return _content_length;
    }

    constexpr const char* fields[] = {
        "Content-Length",
        "content-length",
    };
    for (size_t i = 0; i < sizeof(fields)/sizeof(const char*); i++) {
        if (headers.contains(fields[i])) {
            _content_length = std::make_optional(std::stoll(headers.at(fields[i])));
            break;
        }
    }
    return _content_length;
}

const std::optional<std::string>& Request::content_type() noexcept {
    if (_content_type.has_value()) {
        return _content_type;
    }

    constexpr const char* fields[] = {
        "Content-Type",
        "content-type",
    };
    for (size_t i = 0; i < sizeof(fields)/sizeof(const char*); i++) {
        if (headers.contains(fields[i])) {
            _content_type = std::make_optional(std::string_view(headers.at(fields[i])));
            break;
        }
    }
    return _content_type;
}

}
