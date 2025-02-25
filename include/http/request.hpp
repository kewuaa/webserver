#pragma once
#include <string>
#include <unordered_map>

#include <asyncio.hpp>
#include <magic_enum/magic_enum.hpp>

#include "types.hpp"
#include "buffer.hpp"
using namespace kwa;


namespace http::request {
    struct Request {
        bool valid { true };
        Method method { 0 };
        std::string path {};
        std::unordered_map<std::string, std::string> params {};
        std::string version {};
        std::unordered_map<std::string, std::string> headers {};
        std::string body {};
        size_t body_size { 0 };
        Request() = default;
        Request(Request&) = delete;
        Request& operator=(Request&) = delete;
        Request(Request&&) noexcept;
        Request& operator=(Request&&) noexcept;
        const std::optional<size_t>& content_length() noexcept;
        const std::optional<std::string>& content_type() noexcept;

        inline auto method_str() const noexcept {
            return magic_enum::enum_name(method);
        }
        private:
            std::optional<size_t> _content_length { std::nullopt };
            std::optional<std::string> _content_type { std::nullopt };
    };

    class Handler {
        public:
            Handler() = delete;
            Handler(Handler&) = delete;
            Handler(Handler&&) = delete;
            Handler operator=(Handler&) = delete;
            Handler operator=(Handler&&) = delete;
            Handler(asyncio::Socket&) noexcept;
            asyncio::Task<> handle_request(request::Request&, std::string_view) noexcept;
        private:
            asyncio::Socket& _sock;
            Buffer _buffer {};
            asyncio::Task<> _handle_get(const request::Request&) noexcept;
            asyncio::Task<> _handle_post(request::Request&, std::string_view) noexcept;
    };

    class Parser {
        public:
            Parser() noexcept = default;
            Parser(Parser&) = delete;
            Parser(Parser&&) = delete;
            Parser& operator=(Parser&) = delete;
            Parser& operator=(Parser&&) = delete;
            asyncio::Task<> process(const char* recv_buf, size_t size, Handler& handler) noexcept;

            inline void reset() noexcept {
                _state = State::REQUEST_LINE;
                _incomplete_line.clear();
                std::exchange(_data, {});
            }
        private:
            enum class State {
                REQUEST_LINE,
                HEADERS,
                BODY,
            };

            Buffer _buffer {};
            Request _data {};
            State _state { State::REQUEST_LINE };
            std::string _incomplete_line {};
            void _move_next_state() noexcept;
            void _parse_request_line(std::string_view line) noexcept;
            void _parse_header(std::string_view line) noexcept;
            void _parse_line(std::string_view line) noexcept;
    };
}
