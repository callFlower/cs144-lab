#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

//template <typename... Targs>
//void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

// constructor
ByteStream::ByteStream(const size_t capacity) :  queue_(), capacity_size_(capacity), reader_size_(0), writter_size_(0), end_input_(false), _error(false){}

size_t ByteStream::write(const string &data) {
    // see how many char we can write
    size_t able_write = min(data.size(), capacity_size_ - queue_.size());
    writter_size_ += able_write;
    for(size_t i = 0; i<able_write; i++){
        queue_.push_back(data[i]);
    }
    // return how many were written.
    return able_write;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    int peek_size = min(len, queue_.size());
    return string(queue_.begin(), queue_.begin()+peek_size);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t pop_size = min(len, queue_.size());
    reader_size_ += pop_size;
;    for(size_t i = 0; i<pop_size; i++){
        queue_.pop_front();
    }
}

void ByteStream::end_input() { end_input_ = true; }

bool ByteStream::input_ended() const { return end_input_; }

size_t ByteStream::buffer_size() const { return queue_.size(); }

bool ByteStream::buffer_empty() const { return queue_.empty(); }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return writter_size_; }

size_t ByteStream::bytes_read() const { return reader_size_; }

size_t ByteStream::remaining_capacity() const { return capacity_size_ - queue_.size(); }
