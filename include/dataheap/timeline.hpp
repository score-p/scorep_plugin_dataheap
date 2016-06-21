/*
 * Copyright (c) 2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to
 *    endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <dataheap/interface.hpp>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace dataheap
{
class timeline
{
public:
    class entry_proxy
    {
    public:
        entry_proxy(dataheap_time_line_entry& entry) : entry_(entry)
        {
        }

        std::uint64_t time() const;
        double value() const;

    private:
        dataheap_time_line_entry& entry_;
    };

    class iterator
    {

    public:
        iterator(dataheap_time_line_entry* data, std::size_t size, std::size_t current = 0)
        : data_(data), size_(size), current_(current)
        {
        }

        bool operator!=(const iterator& other)
        {
            return current_ != other.current_;
        }

        iterator& operator++()
        {
            current_++;

            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            operator++();
            return tmp;
        }

        // some people may think about, why an entry_proxy is needed here. And you may say,
        // that it is not. In theory, you would absolutely be right about that, however,
        // we're talking about dataheap. The C++ interface would require Qt. Everybody would
        // yell at me, when a plugin requires Qt. But the C interface is even worse. You
        // simply can't include it from C++ Code without UNDEFing "__cplusplus". Otherwise
        // everything will break badly. This leads to a whole new slew of problems. I can only
        // forward declare the pointer types, that means, I can't access this shit in the
        // code here. So I just wrote an proxy. Could have used a pair<...>, but then the
        // interface is stupid. Just enjoy the broken shit, I can't fix everything.
        entry_proxy operator*();

    private:
        dataheap_time_line_entry* data_;
        std::size_t size_;
        std::size_t current_;
    };

public:
    timeline() = default;

    timeline(dataheap_time_line* data) : data_(data)
    {
    }

    timeline(const timeline&) = delete;
    timeline& operator=(const timeline&) = delete;

    timeline(timeline&& other) : data_(other.data_)
    {
        other.data_ = nullptr;
    }

    timeline& operator=(timeline&& other)
    {
        using std::swap;

        // yeah, I know... We all gonna die.
        // Though it's evil, IT IS VALID.
        swap(data_, other.data_);

        return *this;
    }

    ~timeline();

public:
    iterator begin() const;

    iterator end() const;

private:
    dataheap_time_line* data_;
};
}
