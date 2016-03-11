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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
