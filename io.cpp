// MIT License
//
// Copyright (c) 2024 Nikolai Romashchenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//     of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
//     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//     copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
//     copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include <fstream>
#include <string>
#include <streambuf>
#include "io.h"

using namespace sap::io;
using std::string;
using std::fpos;
using std::ifstream;
using std::string_view;


buffered_reader::buffered_reader(const string& file_name)
    : _msource(file_name)
      , _stream(_msource, std::ios::in)
      , _started(false)
      , _file_length(0)
      , _read(0)
{
    _start_reading();
}

buffered_reader::~buffered_reader()
{
    _stream.close();
}

fpos<mbstate_t> buffered_reader::_get_file_legth()
{
    _stream.seekg(0, ifstream::end);
    fpos<mbstate_t> file_length = _stream.tellg();
    _stream.seekg(0, ifstream::beg);
    return file_length;
}

string_view buffered_reader::read_next_chunk()
{
    if (!_started)
    {
        _start_reading();
    }
    else
    {
        _read_next_chunk();
    }
    return string_view(_buffer);
}

bool buffered_reader::empty() const
{
    if (_file_length == 0)
    {
        return true;
    }

    return _read == _file_length;
}

bool buffered_reader::good() const
{
    return (bool)_stream;
}

void buffered_reader::_start_reading()
{
    _file_length = _get_file_legth();
    _started = true;
}

void buffered_reader::_read_next_chunk()
{
    _buffer[0] = '\0';

    if (_read < _file_length)
    {
        std::streamsize size_to_read = std::min(_file_length - _read, static_cast<std::streamoff>(buffer_size - 1));
        _stream.read(_buffer, size_to_read);
        _buffer[size_to_read] = '\0';
        _read += size_to_read;
    }
}

std::string sap::io::read_as_string(const std::string& filename)
{
    /// FIXME: error handling
    std::ifstream stream(filename);
    return std::string{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
}