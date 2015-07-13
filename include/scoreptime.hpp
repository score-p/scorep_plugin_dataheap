//
//  scoreptime.hpp
//  scorep_dataheap_plugin
//
//  Created by Mario Bielert on 05.04.13.
//  Copyright (c) 2013-2015 TU Dresden. All rights reserved.
//

#ifndef INCLUDE_NI_PLUGIN_TIME_SCOREP
#define INCLUDE_NI_PLUGIN_TIME_SCOREP

#include <cstdint>
#include <chrono>

class ScorepTime
{
public:

    std::chrono::nanoseconds now() const;

    std::chrono::nanoseconds scorep_to_local(std::chrono::nanoseconds timestamp) const;

    std::chrono::nanoseconds local_to_scorep(std::chrono::nanoseconds timestamp) const;

    ScorepTime(uint64_t ( *wtime )( void ));

private:
    double factor, offset;
    uint64_t ( *wtime )( void );
};

#endif //INCLUDE_NI_PLUGIN_TIME_SCOREP
