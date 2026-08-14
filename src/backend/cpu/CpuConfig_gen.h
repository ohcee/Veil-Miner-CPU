/* XMRig
 * Copyright (c) 2018-2024 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2024 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMRIG_CPUCONFIG_GEN_H
#define XMRIG_CPUCONFIG_GEN_H


#include "backend/common/Threads.h"
#include "backend/cpu/Cpu.h"
#include "backend/cpu/CpuThreads.h"


namespace xmrig {


static inline size_t generate(const char *key, Threads<CpuThreads> &threads, const Algorithm &algorithm, uint32_t limit)
{
    if (threads.isExist(algorithm) || threads.has(key)) {
        return 0;
    }

    return threads.move(key, Cpu::info()->threads(algorithm, limit));
}


template<Algorithm::Family FAMILY>
static inline size_t generate(Threads<CpuThreads> &, uint32_t) { return 0; }


template<>
size_t inline generate<Algorithm::CN>(Threads<CpuThreads> &threads, uint32_t limit)
{
    size_t count = 0;

    count += generate(Algorithm::kCN, threads, Algorithm::CN_1, limit);

    if (!threads.isExist(Algorithm::CN_0)) {
        threads.disable(Algorithm::CN_0);
        ++count;
    }

    return count;
}










#ifdef XMRIG_ALGO_RANDOMX
template<>
size_t inline generate<Algorithm::RANDOM_X>(Threads<CpuThreads> &threads, uint32_t limit)
{
    size_t count = 0;
    auto cpuInfo = Cpu::info();
    auto wow     = cpuInfo->threads(Algorithm::RX_WOW, limit);

    if (!threads.isExist(Algorithm::RX_ARQ)) {
        auto arq = cpuInfo->threads(Algorithm::RX_ARQ, limit);
        if (arq == wow) {
            threads.setAlias(Algorithm::RX_ARQ, Algorithm::kRX_WOW);
            ++count;
        }
        else {
            count += threads.move(Algorithm::kRX_ARQ, std::move(arq));
        }
    }

    if (!threads.isExist(Algorithm::RX_WOW)) {
        count += threads.move(Algorithm::kRX_WOW, std::move(wow));
    }

    count += generate(Algorithm::kRX, threads, Algorithm::RX_0, limit);

    return count;
}
#endif






} /* namespace xmrig */


#endif /* XMRIG_CPUCONFIG_GEN_H */
