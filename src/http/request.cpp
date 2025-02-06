#include <spdlog/spdlog.h>

#include "http/request.hpp"


namespace http {
    Request::Data::Data(Data&& data) noexcept:
        method(data.method),
        path(std::move(data.path)),
        params(std::move(data.params)),
        version(std::move(data.version)),
        headers(std::move(data.headers)),
        body(std::move(data.body))
    {
        //
    }

    Request::Data& Request::Data::operator=(Data&& data) noexcept {
        method = data.method;
        path = std::move(data.path);
        params = std::move(data.params);
        version = std::move(data.version);
        headers = std::move(data.headers);
        body = std::move(data.body);
        return *this;
    }

    std::optional<size_t> Request::Data::content_length() const noexcept {
        constexpr const char* fields[] = {
            "Content-Length",
            "content-length",
        };
        for (size_t i = 0; i < sizeof(fields)/sizeof(const char*); i++) {
            if (headers.find(fields[i]) != headers.end()) {
                return std::make_optional(std::stoll(headers.at(fields[i])));
            }
        }
        return std::nullopt;
    }

    void Request::_move_next_state() noexcept {
        switch (_state) {
            case State::REQUEST_LINE: {
                _state = State::HEADERS;
                break;
            }
            case State::HEADERS: {
                if (_data.method != Method::POST) {
                    _state = State::REQUEST_LINE;
                } else {
                    _state = State::BODY;
                }
                break;
            }
            case State::BODY: {
                _state = State::REQUEST_LINE;
                break;
            }
            default: {
                std::unreachable();
                break;
            }
        }
        if (_state == State::REQUEST_LINE) {
            _stage_data();
        }
    }

    void Request::_parse_request_line(std::string_view line) noexcept {
        auto space1 = line.find(' ', 0);
        if (space1 == std::string_view::npos) {
            _data.valid = false;
            return;
        }
        auto method = magic_enum::enum_cast<Method>(line.substr(0, space1));;
        if (!method) {
            _data.valid = false;
            return;
        }
        _data.method = *method;
        auto space2 = line.find(' ', space1 + 1);
        if (space2 == std::string_view::npos) {
            _data.valid = false;
            return;
        }
        auto path = line.substr(space1+1, space2-space1-1);
        if (auto pos = path.find('?'); pos != std::string_view::npos) {
            auto params = path.substr(pos+1);
            path = path.substr(0, pos);
            pos = 0;
            while (pos < params.size()) {
                auto split = params.find('&', pos);
                if (split == std::string_view::npos) {
                    split = params.size();
                }
                auto pair = params.substr(pos, split-pos);
                auto equal_pos = pair.find('=');
                _data.params[std::string(pair.substr(0, equal_pos))] = pair.substr(equal_pos+1);
                pos = split + 1;
            }
        }
        _data.path = path;
        _data.version = line.substr(space2+1);
        _move_next_state();
    }

    void Request::_parse_header(std::string_view line) noexcept {
        auto split = line.find(':');
        if (split == std::string_view::npos) {
            _data.valid = false;
            return;
        }
        auto key = line.substr(0, split);
        auto value = line.substr(split+1);
        while (value.starts_with(' ')) {
            value = value.substr(1);
        }
        _data.headers[std::string(key)] = value;
    }

    void Request::_parse_line(std::string_view line) noexcept {
        if (_state == State::REQUEST_LINE) {
            _parse_request_line(line);
        } else if (_state == State::HEADERS) {
            _parse_header(line);
        } else {
            std::unreachable();
        }
    }

    void Request::parse(const char* buffer, size_t size) noexcept {
        if (size < 1) {
            SPDLOG_WARN("try parse empty buffer");
            return;
        }
        std::string_view view { buffer, size };
        size_t pos = 0;
        if (
            _state != State::BODY
            && !_incomplete_line.empty()
            && _incomplete_line.back() == '\r'
            && view[0] == '\n'
        ) {
            std::string_view line = _incomplete_line;
            _parse_line(line.substr(0, line.size()-1));
            _incomplete_line.clear();
            pos++;
        }
        while (pos < size && _data.valid) {
            if (_state == State::BODY) {
                if (auto content_length = _data.content_length(); content_length) {
                    if (auto remain_size = size - pos; remain_size < content_length) {
                        _data.body += view.substr(pos);
                        pos += remain_size;
                    } else {
                        _data.body += view.substr(pos, *content_length);
                        _move_next_state();
                        pos += *content_length;
                    }
                }
            } else {
                auto crlf = view.find(CRLF, pos);
                if (crlf == std::string_view::npos) {
                    _incomplete_line = view.substr(pos);
                    break;
                }
                if (crlf == pos) {
                    if (_incomplete_line.empty()) {
                        _move_next_state();
                    } else {
                        _parse_line(_incomplete_line);
                        _incomplete_line.clear();
                    }
                    pos += 2;
                    continue;
                }
                if (!_incomplete_line.empty()) {
                    std::string_view line { buffer + pos, buffer + crlf };
                    _incomplete_line += line;
                    _parse_line(_incomplete_line);
                    _incomplete_line.clear();
                } else {
                    _parse_line({ buffer + pos, buffer + crlf });
                }
                pos = crlf + 2;
            }
        }
        if (!_data.valid) {
            _stage_data();
            reset();
        }
    }

    std::optional<Request::Data> Request::get() noexcept {
        if (_cache.empty()) {
            return std::nullopt;
        }
        auto res = std::move(_cache.front());
        _cache.pop();
        return std::make_optional(std::move(res));
    }
}
