/*
 * Copyright (c) 2015-2016, Technische Universität Dresden, Germany
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

#include <dataheap/channel.hpp>
#include <dataheap/dataheap.hpp>
#include <dataheap/exception.hpp>

#include <scorep/plugin/log.hpp>

extern "C" {
// HACK: Somebody thought, it's a nice idea to include tons of headers for
//       everybody using a c++ compiler. And those will contains templates.
//       so you can't use extern "C", which breaks linking. Thanks.
//       Next time I write a C interface I'm going to do something like this:
//       #ifndef __cplusplus #define NULL -1 #endif
//       This will be fun >:-}

#undef __cplusplus
#include <dataheap.h>
}

namespace dataheap
{
dataheap::dataheap(const std::string& server, int port) : connection_(nullptr)
{
    connect(server, port);
}

dataheap::~dataheap()
{
    if (connected())
        disconnect();
}

void dataheap::connect(const std::string& server, int port)
{
    if (connected_)
    {
        scorep::plugin::logging::warn() << "Try to connect, though already connected.";
        return;
    }

    int ret = dataheap_create_context(&connection_);

    if (ret)
    {
        scorep::exception::raise<connection_error>("Failed to create dataheap context. (", ret,
                                                   ")");
    }

    ret = dataheap_initialize(server.c_str(), port, nullptr, 1, connection_);

    if (ret)
    {
        scorep::exception::raise<connection_error>("Failed to initialize connection to dataheap. (",
                                                   ret, ")");
    }

    string_list* counters;

    ret = dataheap_list_counter(connection_, &counters);

    if (ret)
    {
        scorep::exception::raise<connection_error>(
            "Failed to get list of all counters from dataheap server. (", ret, ")");
    }

    for (std::size_t i = 0; i < counters->count; ++i)
    {
        auto it = available_channels_.emplace(counters->data[i]);
        free(counters->data[i]);
    }

    free(counters->data);
    free(counters);

    connected_ = true;
}

void dataheap::disconnect()
{
    if (connected_)
    {
        dataheap_finalize(connection_);
        dataheap_destroy_context(connection_);

        connected_ = false;
    }
    else
    {
        scorep::exception::raise<connection_error>("Trying to disconnect, though not connected.");
    }
}

bool dataheap::connected() const
{
    return connected_;
}

bool dataheap::is_available(const std::string& channel) const
{
    return available_channels_.count(channel) > 0;
}

channel dataheap::find_channel(const std::string& name)
{
    char* desc;
    char* unit;

    dataheap_description(connection_, name.c_str(), &desc);
    dataheap_unit(connection_, name.c_str(), &unit);

    channel result(*this, name, desc, unit);

    free(desc);
    free(unit);

    return result;
}
// FIXME: It is Friday 6pm, so fuck off, I'll just do it here.
//        Additionally, I don't want to make the fucking HACK up there twice...

void channel::subscribe()
{
    if (subscribed_)
    {
        scorep::exception::raise<connection_error>("Trying to subscribe for channel '", name_,
                                                   "', though already subscribed.'");
    }

    int ret = dataheap_subscribe(dh_.connection_, name_.c_str(), 1, &handle_);

    if (ret)
    {
        scorep::exception::raise<connection_error>("Failed to subscribe on dataheap for channel: ",
                                                   name_);
    }

    subscribed_ = true;
}

timeline channel::unsubscribe()
{
    if (!subscribed_)
    {
        scorep::exception::raise<connection_error>("Trying to unsubscribe for channel '", name_,
                                                   "', though not subscribed.'");
    }

    dataheap_time_line* tl;

    int ret = dataheap_unsubscribe(dh_.connection_, handle_, &tl);

    if (ret)
    {
        scorep::exception::raise<connection_error>(
            "Failed to unsubscribe on dataheap for channel: ", name_);
    }

    subscribed_ = false;

    return timeline(tl);
}

channel::~channel()
{
    if (subscribed_)
    {
        unsubscribe();
    }
}

timeline::~timeline()
{
    if (data_)
    {
        free(data_->data);
        free(data_);
    }
}

timeline::iterator timeline::begin() const
{
    if (data_ == nullptr)
    {
        scorep::exception::raise<connection_error>(
            "Try to access channel subscription data before it was actually set.");
    }

    return iterator(data_->data, data_->count);
}

timeline::iterator timeline::end() const
{
    if (data_ == nullptr)
    {
        scorep::exception::raise<connection_error>(
            "Try to access channel subscription data before it was actually set.");
    }

    return iterator(data_->data, data_->count, data_->count);
}

timeline::entry_proxy timeline::iterator::operator*()
{
    return timeline::entry_proxy(data_[current_]);
}

uint64_t timeline::entry_proxy::time() const
{
    return entry_.time_in_ms;
}

double timeline::entry_proxy::value() const
{
    return entry_.value;
}
}
