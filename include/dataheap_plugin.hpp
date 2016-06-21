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

#include <scorep/plugin/plugin.hpp>

#include <dataheap/dataheap.hpp>

#include <unistd.h>

using namespace scorep::plugin::policy;

template <typename T, typename Policies>
using handle_oid_policy = object_id<dataheap::channel, T, Policies>;

class dataheap_plugin : public scorep::plugin::base<dataheap_plugin, async, post_mortem,
                                                    scorep_clock, per_host, handle_oid_policy>
{

    static std::string get_hostname()
    {
        char hostname[2048];

        if (0 != gethostname(hostname, 2048))
        {
            scorep::exception::raise("Can't get the hostname of the current machine");
        }

        return hostname;
    }

    static std::string replace_localhost(std::string in)
    {
        std::size_t pos = in.find("localhost");

        if (std::string::npos != pos)
        {
            std::string hostname = get_hostname();
            in.replace(pos, 9, hostname);
        }
        return in;
    }

public:
    dataheap_plugin()
    {
        connect_to_dataheap();
    }

    void connect_to_dataheap()
    {
        std::string server;
        int port;

        std::stringstream s;
        s << scorep::environment_variable::get("SERVER");

        std::getline(s, server, ':');

        s >> port;

        if (!s)
        {
            scorep::exception::raise("Couldn't parse ",
                                     scorep::environment_variable::name("SERVER"),
                                     ". Please set it to HOSTNAME:PORT");
        }

        dataheap_.connect(server, port);
    }

    std::vector<scorep::plugin::metric_property>
    get_metric_properties(const std::string& channel_name)
    {
        auto channel = dataheap_.find_channel(replace_localhost(channel_name));

        make_handle(channel_name, channel);

        return { scorep::plugin::metric_property(channel_name, channel.description(),
                                                 channel.unit())
                     .absolute_last() };
    }

    void add_metric(dataheap::channel& chan)
    {
        chan.subscribe();
    }

    void start()
    {
        convert_.synchronize_point();
    }

    void stop()
    {
        convert_.synchronize_point();
    }

    template <typename Cursor>
    void get_all_values(dataheap::channel& chan, Cursor& c)
    {
        auto timeline = chan.unsubscribe();

        for (auto entry : timeline)
        {
            const auto time =
                std::chrono::system_clock::time_point(std::chrono::milliseconds(entry.time()));
            c.write(convert_.to_ticks(time), entry.value());
        }
    }

public:
    dataheap::dataheap dataheap_;
    scorep::chrono::time_convert<> convert_;
};
