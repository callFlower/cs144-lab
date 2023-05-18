#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) :
    // int->index, string->data
    // buffer就是一个滑动窗口，只存unread和unaccepted里面的内容
    _buffer(),
    _eof_appear_sign(false),
    _eof_index(0),
    _output(capacity),
    _capacity(capacity){}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const std::string &data, const size_t index, const bool eof) {
    // eof
    // g | h |
    //       ^
    //      _eof_index
    if(eof){
        _eof_appear_sign = true;
        _eof_index = index + data.size();
    }
    // ｜           ｜            ｜
    // unread      unass        unacc
    //         |(index)     |(index)
    if(first_unassembled() > index){
        // first_unassembled() > index + data.size() --> 全是老数据，这种情况直接什么都不干，也就是丢弃
        if(first_unassembled() > (index + data.size())){ return;}
        // 第二种有老数据有新数据，使用递归把新的部分截取出来，声明index，将原本传入的参数-eof传入
        // get from pos to the end, eg: a   b   c   d   f   g
        //                                      |
        //                            index    unass
        // index = 5, unass = 7, so we need to start from data[2], which is unass-index
        push_substring(data.substr(first_unassembled() - index), first_unassembled(), eof);
    }
    size_t old_first_unassembled = first_unassembled();
    size_t old_first_unacceptable = first_unacceptable();

    // 关键代码
    // ｜                 ｜            ｜
    // unass(index)      index        unacc
    // 正好能放进缓冲区-上层的字节流-lab_0，直接调用write函数
    if(first_unassembled() == index){
        // 首先，将数据全写进去
        _output.write(data);
        // 写完后他的滑动窗口会变，往右移。
        // 这个时候就需要判断之前在buffer里面的内容能不能也放入上层的字节流-lab_0
        reassemble(old_first_unassembled, old_first_unacceptable);
    }else{
        // 乱序到达，我们放进buffer里
        put_in_buffer(data,index);
    }

    // 结束
    if (_eof_appear_sign && first_unassembled() == _eof_index){
        _buffer.clear();
        _output.end_input();
    }

}

// 滑动窗口往右移动，进行重排
void StreamReassembler::reassemble(size_t old_first_unassembled, size_t old_first_unacceptable) {
    string s = string();
    // 开始在原来的unass--unacc区间里面找
    for(size_t i = old_first_unassembled; i<old_first_unacceptable; i++){
        // 使用 find 函数查找一个不存在的键时，find 函数将返回一个指向 std::map 结尾迭代器（end()）的迭代器。
        // 所以下面if代码是当buffer里面存在想要的元素的时候
        if(_buffer.find(i) != _buffer.end()){
            // 如果是已经排好序的，把它删掉
            if(i<first_unassembled()){
                _buffer.erase(i);
                continue;
            }
            // 未排好序的
            if(i == first_unassembled() + s.length()){
                s.push_back(_buffer.at(i));
                _buffer.erase(i);
            }else{
                break;
            }
        }
    }
    _output.write(s);
}

// 逐个字符的放入buffer中
// 切记buffer中只放unread和unaccepted里面的内容
void StreamReassembler::put_in_buffer(const std::string &data, const size_t index) {
    for(size_t i = 0; i<data.size(); i++){
        if(index + i < first_unacceptable()){
            _buffer.insert(std::pair<size_t ,char>(index + i, data[i]));
        }
    }
}

// unread + capacity 就是整个窗口/buffer的大小，所以此函数返回的是窗口的首index
size_t StreamReassembler::first_unread(){ return _output.bytes_read();}
// 是 unwrite 的index，因为所有不符合位置的都不写入，也就是放入map中存储
size_t StreamReassembler::first_unassembled() { return _output.bytes_written();}
// unacceptable == unread + capacity
size_t StreamReassembler::first_unacceptable() { return _output.bytes_read() + _capacity;}

size_t StreamReassembler::unassembled_bytes() const { return _buffer.size(); }
bool StreamReassembler::empty() const { return _buffer.empty(); }
