#pragma once
#include <string>
#include <queue>
#include <unordered_map>

#include <magic_enum/magic_enum.hpp>


#include "types.hpp"


namespace http {
    class Request {
        public:
            struct Data {
                bool valid { true };
                Method method { 0 };
                std::string path {};
                std::unordered_map<std::string, std::string> params {};
                std::string version {};
                std::unordered_map<std::string, std::string> headers {};
                std::string body {};
                Data() = default;
                Data(Data&) = delete;
                Data& operator=(Data&) = delete;
                Data(Data&&) noexcept;
                Data& operator=(Data&&) noexcept;
                std::optional<size_t> content_length() const noexcept;

                inline auto method_str() const noexcept {
                    return magic_enum::enum_name(method);
                }
            };

            Request() noexcept = default;
            Request(Request&) = delete;
            Request(Request&&) = delete;
            Request& operator=(Request&) = delete;
            Request& operator=(Request&&) = delete;
            void parse(const char* buffer, size_t size) noexcept;
            std::optional<Data> get() noexcept;

            inline bool has_data() const noexcept {
                return !_cache.empty();
            }

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

            Data _data {};
            std::queue<Data> _cache {};
            State _state { State::REQUEST_LINE };
            std::string _incomplete_line {};
            void _move_next_state() noexcept;
            void _parse_request_line(std::string_view line) noexcept;
            void _parse_header(std::string_view line) noexcept;
            void _parse_line(std::string_view line) noexcept;

            inline void _stage_data() noexcept {
                _cache.push(std::exchange(_data, {}));
            }
    };
}
