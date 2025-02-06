#pragma once
#include <vector>
#include <format>
#include <cstring>
#include <cstddef>

#include "config.hpp"


class Buffer {
    private:
        std::vector<char> _buffer;
        size_t _read_idx { 0 };
        size_t _write_idx { 0 };

        void _swap_to_begin() noexcept;
        void _make_available(size_t n) noexcept;

        inline size_t _prependable_bytes() const noexcept {
            return _read_idx;
        }
    public:
        class View {
            friend Buffer;
            private:
                Buffer& _buf;
                size_t _begin;
                size_t _size;
                View(Buffer& buf, size_t begin, size_t size) noexcept;
            public:
                View() = delete;
                View& operator=(View&) = delete;
                View& operator=(View&&) = delete;
                View(View&) noexcept = default;
                View(View&&) noexcept = default;
                inline char* data() const noexcept {
                    return _buf._buffer.data() + _begin;
                }

                inline size_t size() const noexcept {
                    return _size;
                }
        };

        Buffer(size_t buffer_size = BUFFER_SIZE) noexcept;
        Buffer(Buffer&) = default;
        Buffer& operator=(Buffer&) = default;
        Buffer(Buffer&&) noexcept;
        Buffer& operator=(Buffer&& buffer) noexcept;
        [[nodiscard]] std::string_view read(size_t nbytes) noexcept;
        [[nodiscard]] std::string_view read_all() noexcept;
        [[nodiscard]] View malloc(size_t nbytes) noexcept;
        void free(char* data) noexcept;
        void write(char byte) noexcept;
        View write(std::string_view data) noexcept;

        template<typename... Args>
        View write(std::format_string<Args...> fmt, Args&&... args) noexcept {
            return write(std::format(fmt, std::forward<Args>(args)...));
        }

        inline size_t readable_bytes() const noexcept {
            return _write_idx - _read_idx;
        }

        inline size_t writable_bytes() const noexcept {
            return _buffer.size() - _write_idx;
        }

        inline size_t size() const noexcept {
            return _buffer.size();
        }
};
