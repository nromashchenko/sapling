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



#ifndef SAPLING_IO_H
#define SAPLING_IO_H

#include <string>
#include <memory>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

namespace sap::io
{
    /// \brief A buffered reader class for memory mapped files.
    class buffered_reader
    {
    public:
        explicit buffered_reader(const std::string& file_name);
        buffered_reader(const buffered_reader&) = delete;
        buffered_reader(buffered_reader&&) = delete;
        ~buffered_reader();

        buffered_reader& operator=(const buffered_reader&) = delete;
        buffered_reader& operator=(buffered_reader&&) = delete;

        std::string_view read_next_chunk();
        bool empty() const;
        bool good() const;

        static constexpr size_t buffer_size = 4096;

    private:
        std::fpos<mbstate_t> _get_file_legth();
        void _start_reading();
        void _read_next_chunk();

    private:
        using mf_source_t = boost::iostreams::mapped_file_source;
        using stream_t = boost::iostreams::stream<mf_source_t>;

        mf_source_t _msource;
        stream_t _stream;

        bool _started;
        std::fpos<mbstate_t> _file_length;
        std::fpos<mbstate_t> _read = 0;

        char _buffer[buffer_size];
    };

    /// \brief Reads the whole file in a string
    std::string read_as_string(const std::string& filename);
}

#endif //SAPLING_IO_H
