/* XMRig
 * Copyright (c) 2018-2019 tevador     <tevador@gmail.com>
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#include "crypto/randomx/randomx.h"
#include "crypto/rx/RxAlgo.h"


xmrig::Algorithm::Id xmrig::RxAlgo::apply(Algorithm::Id algorithm)
{
    randomx_apply_config(*base(algorithm));

    return algorithm;
}


const RandomX_ConfigurationBase *xmrig::RxAlgo::base(Algorithm::Id algorithm)
{
    // Veil uses the reference RandomX configuration; the chain-specific part
    // is the sha256d header transform handled in RxVeil.
    return &RandomX_MoneroConfig;
}


uint32_t xmrig::RxAlgo::version(Algorithm::Id algorithm)
{
    return 104;
}


uint32_t xmrig::RxAlgo::programCount(Algorithm::Id algorithm)
{
    return base(algorithm)->ProgramCount;
}


uint32_t xmrig::RxAlgo::programIterations(Algorithm::Id algorithm)
{
    return base(algorithm)->ProgramIterations;
}


uint32_t xmrig::RxAlgo::programSize(Algorithm::Id algorithm)
{
    return base(algorithm)->ProgramSize;
}
