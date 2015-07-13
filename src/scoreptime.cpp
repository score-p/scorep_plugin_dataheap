//
//  scoreptime.cpp
//  scorep_dataheap_plugin
//
//  Created by Mario Bielert on 05.04.13.
//  Copyright (c) 2013-2015 TU Dresden. All rights reserved.
//

#include "scoreptime.hpp"

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <thread>

std::chrono::nanoseconds ScorepTime::now() const
{
    return std::chrono::nanoseconds(wtime());
}

std::chrono::nanoseconds
ScorepTime::scorep_to_local(std::chrono::nanoseconds timestamp) const
{
    return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(
        timestamp.count() / factor - offset / factor));
}

std::chrono::nanoseconds
ScorepTime::local_to_scorep(std::chrono::nanoseconds timestamp) const
{
    return std::chrono::nanoseconds(static_cast<std::chrono::nanoseconds::rep>(
        factor * timestamp.count() + offset));
}

ScorepTime::ScorepTime(uint64_t (*wtime)(void)) : wtime(wtime)
{
    // initialize time calculations
    std::chrono::nanoseconds first_scorep = now();
    std::chrono::nanoseconds first_local = std::chrono::system_clock::now().time_since_epoch();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::chrono::nanoseconds second_scorep = now();
    std::chrono::nanoseconds second_local = std::chrono::system_clock::now().time_since_epoch();

    factor = static_cast<double>((second_scorep - first_scorep).count()) /
             (second_local - first_local).count();
    offset = first_scorep.count() - factor * first_local.count();

    char* offset_charp = getenv("DATAHEAP_OFFSET_NS");
    if (offset_charp)
    {
        offset += atoll(offset_charp);
    }
}
