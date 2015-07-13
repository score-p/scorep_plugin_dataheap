//
//  main.cpp
//  scorep_dataheap_plugin
//
//  Created by Mario Bielert on 13.08.13.
//  Copyright (c) 2013-2015 TU Dresden. All rights reserved.
//

#include <algorithm>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include <memory>

#include <string>
#include <sstream>

#include <utility>

#include "scoreptime.hpp"

extern "C" {
#include <scorep/SCOREP_MetricPlugins.h>

#include <unistd.h>

// HACK: Somebody thought, it's a nice idea to include tons of headers for
//       everybody using a c++ compiler. And those will contains templates.
//       so you can't use extern "C", which breaks linking. Thanks.
//       Next time I write a C interface I'm going to do something like this:
//       #ifndef __cplusplus #define NULL -1 #endif
//       This will be fun >:-}
#undef __cplusplus
#include <dataheap.h>
}

/*--------------------------------------------------------------------------
 | TYPEDEFS AND TYPES                                                      *
 --------------------------------------------------------------------------*/
struct dataheap_connection_data
{
    std::string server;
    int port;
};

struct time_line_entry
{
    std::uint64_t time;
    std::uint64_t value;
};

typedef std::int32_t scorep_metric_event_id_t;

/*--------------------------------------------------------------------------
 | STATIC VARIABLES                                                        *
 --------------------------------------------------------------------------*/

static std::vector<std::string> dataheap_avail;

static std::unique_ptr<ScorepTime> time_sync;

static dataheap_connection* connection;

static dataheap_connection_data dataheap;

/*--------------------------------------------------------------------------
 | HELPER FUNCTIONS                                                        *
 --------------------------------------------------------------------------*/

template <typename T> T* allocate_c_memory(std::size_t num_elements)
{
    // using calloc, as it also sets the memory to 0 and has a sane interface
    return reinterpret_cast<T*>(calloc(num_elements, sizeof(T)));
}

std::string get_hostname()
{
    char hostname[2048];

    if (0 != gethostname(hostname, 2048))
    {
        throw std::runtime_error("Can't get hostname");
    }

    return hostname;
}

std::string replace_localhost(std::string in)
{
    std::size_t pos = in.find("localhost");

    if (std::string::npos != pos)
    {
        std::string hostname = get_hostname();
        in.replace(pos, 9, hostname);
    }
    return in;
}

void add_metric_property(
    std::vector<SCOREP_Metric_Plugin_MetricProperties>& metric_properties,
    const std::string& name)
{
    SCOREP_Metric_Plugin_MetricProperties prop;

    prop.name = strdup(name.c_str());

    char* desc;
    char* unit;

    dataheap_description(connection, name.c_str(), &desc);
    dataheap_unit(connection, name.c_str(), &unit);

    prop.description = strdup(desc);
    prop.unit = strdup(unit);

    prop.mode = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
    prop.value_type = SCOREP_METRIC_VALUE_DOUBLE;
    prop.base = SCOREP_METRIC_BASE_DECIMAL;
    prop.exponent = 0;

    metric_properties.push_back(prop);
}

void add_empty_property(
    std::vector<SCOREP_Metric_Plugin_MetricProperties>& metric_properties)
{
    SCOREP_Metric_Plugin_MetricProperties prop;

    prop.name = nullptr;

    metric_properties.push_back(prop);
}

/*--------------------------------------------------------------------------
 | PLUGIN CALLBACK FUNCTIONS                                               *
 --------------------------------------------------------------------------*/

int32_t init()
{
    char* tmp = getenv("SCOREP_METRIC_DATAHEAP_SERVER");

    if (tmp != 0)
    {
        std::stringstream s;
        s << tmp;

        std::getline(s, dataheap.server, ':');

        s >> dataheap.port;

        if (!s)
        {
            std::cerr << "Couldn't parse SCOREP_METRIC_DATAHEAP_SERVER. Please set it "
                         "to HOSTNAME:PORT" << std::endl;

            return -1;
        }
    }
    else
    {
        std::cerr << "Couldn't read SCOREP_METRIC_DATAHEAP_SERVER. Please set it to "
                     "HOSTNAME:PORT" << std::endl;

        return -2;
    }

    std::cout << "Using dataheap server: " << dataheap.server << ":"
              << dataheap.port << std::endl;

    int ret;

    connection = nullptr;

    ret = dataheap_create_context(&connection);

    if (ret)
    {
        std::cerr << "Failed to create dataheap context: (" << ret << ")" << std::endl;
        return -1;
    }

    ret = dataheap_initialize(dataheap.server.c_str(), dataheap.port, nullptr,
                              1, connection);

    if (ret)
    {
        std::cerr << "Failed to initialize connection to dataheap. (" << ret << ")" << std::endl;
        return -1;
    }

    string_list* counters;

    ret = dataheap_list_counter(connection, &counters);

    if (ret)
    {
        std::cerr << "Failed to get list of all counters from dataheap server. (" << ret << ")" << std::endl;
    }

    dataheap_avail = std::vector<std::string>(counters->data,
                                              counters->data + counters->count);

    return 0;
}

void set_clock(uint64_t (*clock)(void))
{
    time_sync.reset(new ScorepTime(clock));
}

SCOREP_Metric_Plugin_MetricProperties* get_event_info(char* raw_counter)
{
    std::vector<SCOREP_Metric_Plugin_MetricProperties> metric_properties;

    add_metric_property(metric_properties, replace_localhost(raw_counter));

    add_empty_property(metric_properties);

    auto ret = allocate_c_memory<SCOREP_Metric_Plugin_MetricProperties>(
        metric_properties.size());
    memcpy(ret, &metric_properties[0],
           metric_properties.size() *
               sizeof(SCOREP_Metric_Plugin_MetricProperties));
    return ret;
}

scorep_metric_event_id_t add_counter(char* raw_counter)
{
    std::string counter = replace_localhost(raw_counter);

    auto it = std::find(dataheap_avail.begin(), dataheap_avail.end(), counter);

    if (it == dataheap_avail.end())
    {
        std::cerr << "Counter does not exist in dataheap: " << counter
                  << std::endl;
        return -1;
    }

    uint64_t dataheap_id;
    int ret = dataheap_subscribe(connection, counter.c_str(), 1, &dataheap_id);

    if (ret)
    {
        std::cerr << "Failed to subscribe on dataheap for counter: " << counter
                  << std::endl;
        return -1;
    }

    std::cout << "Successfully subscribed to counter " << counter << " with id "
              << dataheap_id << std::endl;
    return dataheap_id;
}

uint64_t get_all_values(scorep_metric_event_id_t id,
                        SCOREP_MetricTimeValuePair** time_value_list)
{
    dataheap_time_line* tl;

    dataheap_unsubscribe(connection, id, &tl);

    dataheap_time_line_entry_t* entries = tl->data;

    std::size_t num_results = tl->count;

    std::cout << "Received " << num_results << " datapoints scorep id: " << id
              << ", dataheap id: " << id << ".\n";

    // allocate memory
    *time_value_list =
        allocate_c_memory<SCOREP_MetricTimeValuePair>(num_results);

    for (std::size_t i = 0; i < num_results; ++i)
    {
        (*time_value_list)[i].timestamp =
            time_sync->local_to_scorep(std::chrono::milliseconds(
                                           entries[i].time_in_ms)).count();
        (*time_value_list)[i].value = *reinterpret_cast<uint64_t*>(&(entries[i].value));
    }

    return num_results;
}

void finalize()
{
    dataheap_finalize(connection);
    dataheap_destroy_context(connection);
}

/*--------------------------------------------------------------------------
 | PLUGIN ENTRY POINT                                                      *
 --------------------------------------------------------------------------*/

SCOREP_METRIC_PLUGIN_ENTRY(dataheap_plugin)
{
    /* Initialize info data (with zero) */
    SCOREP_Metric_Plugin_Info info;
    memset(&info, 0, sizeof(SCOREP_Metric_Plugin_Info));

    /* Set up the structure */
    info.plugin_version = SCOREP_METRIC_PLUGIN_VERSION;
    info.run_per = SCOREP_METRIC_PER_HOST;
    info.sync = SCOREP_METRIC_ASYNC;
    info.initialize = init;
    info.finalize = finalize;
    info.get_event_info = get_event_info;
    info.add_counter = add_counter;
    info.get_all_values = get_all_values;
    info.set_clock_function = set_clock;
    info.delta_t = UINT64_MAX;

    return info;
}
