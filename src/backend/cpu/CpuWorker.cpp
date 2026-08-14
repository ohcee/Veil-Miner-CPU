/* XMRig
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

#include <cassert>
#include <thread>
#include <mutex>


#include "backend/cpu/Cpu.h"
#include "backend/cpu/CpuWorker.h"
#include "base/tools/Alignment.h"
#include "base/tools/Chrono.h"
#include "base/tools/bswap_64.h"
#include "core/config/Config.h"
#include "core/Miner.h"
#include "crypto/common/Nonce.h"
#include "crypto/common/VirtualMemory.h"
#include "crypto/rx/Rx.h"
#include "crypto/rx/RxCache.h"
#include "crypto/rx/RxDataset.h"
#include "crypto/rx/RxVm.h"
#include "net/JobResults.h"
#include "crypto/rx/RxVeil.h"

#ifdef XMRIG_ALGO_RANDOMX
#   include "crypto/randomx/randomx.h"
#endif




namespace xmrig {

static constexpr uint32_t kReserveCount = 32768;



} // namespace xmrig



template<size_t N>
xmrig::CpuWorker<N>::CpuWorker(size_t id, const CpuLaunchData &data) :
    Worker(id, data.affinity, data.priority),
    m_algorithm(data.algorithm),
    m_assembly(data.assembly),
    m_hwAES(data.hwAES),
    m_yield(data.yield),
    m_miner(data.miner),
    m_threads(data.threads)
{
    {
        m_memory = new VirtualMemory(m_algorithm.l3() * N, data.hugePages, false, true, node(), VirtualMemory::kDefaultHugePageSize);
    }

}


template<size_t N>
xmrig::CpuWorker<N>::~CpuWorker()
{
#   ifdef XMRIG_ALGO_RANDOMX
    RxVm::destroy(m_vm);
#   endif

    {
        delete m_memory;
    }

}


#ifdef XMRIG_ALGO_RANDOMX
template<size_t N>
void xmrig::CpuWorker<N>::allocateRandomX_VM()
{
    RxDataset *dataset = Rx::dataset(m_job.currentJob(), node());

    while (dataset == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        if (Nonce::sequence(Nonce::CPU) == 0) {
            return;
        }

        dataset = Rx::dataset(m_job.currentJob(), node());
    }

    if (!m_vm) {
        // Try to allocate scratchpad from dataset's 1 GB huge pages, if normal huge pages are not available
        uint8_t* scratchpad = m_memory->isHugePages() ? m_memory->scratchpad() : dataset->tryAllocateScrathpad();
        m_vm = RxVm::create(dataset, scratchpad ? scratchpad : m_memory->scratchpad(), !m_hwAES, m_assembly, node());
    }
    else if (!dataset->get() && (m_job.currentJob().seed() != m_seed)) {
        // Update RandomX light VM with the new seed
        randomx_vm_set_cache(m_vm, dataset->cache()->get());
    }
    m_seed = m_job.currentJob().seed();
}
#endif


template<size_t N>
bool xmrig::CpuWorker<N>::selfTest()
{
#   ifdef XMRIG_ALGO_RANDOMX
    if (m_algorithm.family() == Algorithm::RANDOM_X) {
        return N == 1;
    }
#   endif

    return false;
}


template<size_t N>
void xmrig::CpuWorker<N>::hashrateData(uint64_t &hashCount, uint64_t &, uint64_t &rawHashes) const
{
    hashCount = m_count;
    rawHashes = m_count;
}


template<size_t N>
void xmrig::CpuWorker<N>::start()
{
    while (Nonce::sequence(Nonce::CPU) > 0) {
        if (Nonce::isPaused()) {
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            while (Nonce::isPaused() && Nonce::sequence(Nonce::CPU) > 0);

            if (Nonce::sequence(Nonce::CPU) == 0) {
                break;
            }

            consumeJob();
        }

#       ifdef XMRIG_ALGO_RANDOMX
        bool first = true;
        alignas(16) uint64_t tempHash[8] = {};
        sph_sha256_context sha256_ctx_cache;
        alignas(16) uint64_t dsha256[4] = {};
#       endif

        while (!Nonce::isOutdated(Nonce::CPU, m_job.sequence())) {
            const Job &job = m_job.currentJob();

            if (job.algorithm().l3() != m_algorithm.l3()) {
                break;
            }

            uint32_t current_job_nonces[N];
            for (size_t i = 0; i < N; ++i) {
                current_job_nonces[i] = readUnaligned(m_job.nonce(i));
            }


#           ifdef XMRIG_ALGO_RANDOMX
            if (job.algorithm().id() == Algorithm::RX_VEIL) {
                if (first) {
                    RxVeil::initJob(job.blob(), job.size(), m_job.nonceOffset(),
                                    sha256_ctx_cache, dsha256, m_vm, tempHash);
                    first = false;
                }
                if (!nextRound()) {
                    break;
                }
                RxVeil::hashStep(m_job.blob(), job.size(), m_job.nonceOffset(),
                                 sha256_ctx_cache, dsha256, m_vm, tempHash, m_hash);

                const uint64_t value = RxVeil::difficultyValue(m_hash);

                if (value < job.target()) {
                    JobResults::submit(job, current_job_nonces[0], m_hash);
                }

                m_count += N;
            }
            else
#           endif
            {
                if (!nextRound()) {
                    break;
                }
            }


            if (m_yield) {
                std::this_thread::yield();
            }
        }

        if (!Nonce::isPaused()) {
            consumeJob();
        }
    }
}


template<size_t N>
bool xmrig::CpuWorker<N>::nextRound()
{
    constexpr uint32_t count = kReserveCount;

    if (!m_job.nextRound(count, 1)) {
        JobResults::done(m_job.currentJob());

        return false;
    }

    return true;
}


template<size_t N>
void xmrig::CpuWorker<N>::consumeJob()
{
    if (Nonce::sequence(Nonce::CPU) == 0) {
        return;
    }

    auto job = m_miner->job();

    constexpr uint32_t count = kReserveCount;

    m_job.add(job, count, Nonce::CPU);

#   ifdef XMRIG_ALGO_RANDOMX
    if (m_job.currentJob().algorithm().family() == Algorithm::RANDOM_X) {
        allocateRandomX_VM();
    }
#   endif
}


namespace xmrig {

template class CpuWorker<1>;
template class CpuWorker<2>;
template class CpuWorker<3>;
template class CpuWorker<4>;
template class CpuWorker<5>;
template class CpuWorker<8>;

} // namespace xmrig

