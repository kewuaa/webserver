#include "utils.hpp"
#include "http/request.hpp"


namespace http::request {
    void Parser::_move_next_state() noexcept {
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
    }

    void Parser::_parse_request_line(std::string_view line) noexcept {
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
                _data.params[std::string(pair.substr(0, equal_pos))] = url_decode(pair.substr(equal_pos+1));
                pos = split + 1;
            }
        }
        _data.path = url_decode(path);
        _data.version = line.substr(space2+1);

        SPDLOG_DEBUG("method: {}", _data.method_str());
        SPDLOG_DEBUG("path: {}", _data.path);
        for (auto& [key, value] : _data.params) {
            SPDLOG_DEBUG("{} -> {}", key, value);
        }
        SPDLOG_DEBUG("version: {}", _data.version);

        _move_next_state();
    }

    void Parser::_parse_header(std::string_view line) noexcept {
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
        SPDLOG_DEBUG("{} -> {}", key, value);
        _data.headers[std::string(key)] = url_decode(value);
    }

    void Parser::_parse_line(std::string_view line) noexcept {
        SPDLOG_DEBUG("parsing line: {}", line);
        if (_state == State::REQUEST_LINE) {
            _parse_request_line(line);
        } else if (_state == State::HEADERS) {
            _parse_header(line);
        } else {
            std::unreachable();
        }
    }

    asyncio::Task<> Parser::process(const char* recv_buf, size_t size, Handler& handler) noexcept {
        if (size < 1) {
            SPDLOG_WARN("try parse empty buffer");
            co_return;
        }
        std::string_view view { recv_buf, size };
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
        while (true) {
            if (!_data.valid) {
                handler.handle_request(_data, "");
                // reset state
                _state = State::REQUEST_LINE;
                _incomplete_line.clear();
                _data = {};
                break;
            }

            if (_state == State::BODY) {
                if (
                    auto content_length = _data.content_length();
                    content_length.has_value()
                ) {
                    auto remain_size = size - pos;
                    auto need_size = *content_length - _data.body_size;
                    auto load_size = std::min(remain_size, need_size);
                    if (load_size > 0) {
                        std::string_view body = view.substr(pos, load_size);
                        _data.body_size += load_size;
                        pos += load_size;
                        co_await handler.handle_request(_data, body);
                    } else if (*content_length == 0) {
                        co_await handler.handle_request(_data, "");
                    } else {
                        break;
                    }
                    if (_data.body_size == *content_length) {
                        _data = {};
                        _move_next_state();
                    }
                } else {
                    _data.valid = false;
                    SPDLOG_ERROR("`Content-Length` or `content-length` not found in headers");
                }
                continue;
            }

            if (pos >= size) {
                break;
            }
            auto crlf = view.find(CRLF, pos);
            if (crlf == std::string_view::npos) {
                _incomplete_line = view.substr(pos);
                break;
            }
            if (crlf == pos) {
                if (_incomplete_line.empty()) {
                    _move_next_state();
                    if (_state == State::REQUEST_LINE) {
                        handler.handle_request(_data, "");
                    }
                } else {
                    _parse_line(_incomplete_line);
                    _incomplete_line.clear();
                }
                pos += 2;
                continue;
            }
            if (!_incomplete_line.empty()) {
                std::string_view line { recv_buf + pos, recv_buf + crlf };
                _incomplete_line += line;
                _parse_line(_incomplete_line);
                _incomplete_line.clear();
            } else {
                _parse_line({ recv_buf + pos, recv_buf + crlf });
            }
            pos = crlf + 2;
        }
    }
}
