#include <utility>
#include <cassert>

#include <spdlog/spdlog.h>

#include "buffer.hpp"


Buffer::View::View(Buffer& buf, size_t begin, size_t size) noexcept:
    _buf(buf),
    _begin(begin),
    _size(size)
{
    //
}


void Buffer::_swap_to_begin() noexcept {
    auto nbytes = readable_bytes();
    std::memcpy(
        _buffer.data(),
        _buffer.data() + _read_idx,
        nbytes
    );
    _read_idx = 0;
    _write_idx = nbytes;
}


void Buffer::_make_available(size_t nbytes) noexcept {
    auto diff = nbytes - writable_bytes();
    if (diff <= 0) {
        return;
    }
    if (diff <= _prependable_bytes()) {
        _swap_to_begin();
        return;
    }
    _buffer.resize(_buffer.size() + (diff < 15 ? 15 : diff));
}


Buffer::Buffer(size_t buffer_size) noexcept:
    _buffer(buffer_size, 0)
{
    //
}


Buffer::Buffer(Buffer&& buffer) noexcept:
    _buffer(std::move(buffer._buffer)),
    _read_idx(std::exchange(buffer._read_idx, 0)),
    _write_idx(std::exchange(buffer._write_idx, 0))
{
    //
}


Buffer& Buffer::operator=(Buffer&& buffer) noexcept {
    _buffer = std::move(buffer._buffer);
    _read_idx = std::exchange(buffer._read_idx, 0);
    _write_idx = std::exchange(buffer._write_idx, 0);
    return *this;
}


std::string_view Buffer::read(size_t nbytes) noexcept {
    assert(readable_bytes() >= nbytes && "no enough data to load");
    std::string_view res { _buffer.data() + _read_idx, nbytes };
    _read_idx += nbytes;
    if (_read_idx == _write_idx) {
        _read_idx = _write_idx = 0;
    }
    return res;
}


std::string_view Buffer::read_all() noexcept {
    std::string_view res { _buffer.data() + _read_idx, readable_bytes() };
    _read_idx = _write_idx = 0;
    return res;
}


Buffer::View Buffer::malloc(size_t nbytes) noexcept {
    _make_available(nbytes);
    View res = { *this, _write_idx, nbytes };
    _write_idx += nbytes;
    return res;
}


void Buffer::free(char* data) noexcept {
    assert(_buffer.data() + _write_idx - data >= 0);
    _write_idx = data - _buffer.data();
}


void Buffer::write(char byte) noexcept {
    _make_available(1);
    _buffer[_write_idx] = byte;
    _write_idx++;
}


Buffer::View Buffer::write(std::string_view data) noexcept {
    if (data.empty()) {
        SPDLOG_WARN("try to write empty data");
        return { *this, _write_idx, 0 };
    }
    auto view = this->malloc(data.size());
    std::copy(data.begin(), data.end(), view.data());
    return view;
}
