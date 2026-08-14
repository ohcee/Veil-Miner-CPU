/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2020 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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


#include "net/JobResults.h"
#include "backend/common/Tags.h"
#include "base/io/Async.h"
#include "base/io/log/Log.h"
#include "base/kernel/interfaces/IAsyncListener.h"
#include "base/tools/Object.h"
#include "net/interfaces/IJobResultListener.h"
#include "net/JobResult.h"


#ifdef XMRIG_ALGO_RANDOMX
#   include "crypto/randomx/randomx.h"
#   include "crypto/rx/Rx.h"
#   include "crypto/rx/RxVm.h"
#endif


#ifdef XMRIG_ALGO_KAWPOW
#   include "crypto/kawpow/KPCache.h"
#   include "crypto/kawpow/KPHash.h"
#endif




#include <cassert>
#include <list>
#include <memory>
#include <mutex>
#include <uv.h>


namespace xmrig {




class JobResultsPrivate : public IAsyncListener
{
public:
    XMRIG_DISABLE_COPY_MOVE_DEFAULT(JobResultsPrivate)

    inline JobResultsPrivate(IJobResultListener *listener, bool hwAES) :
        m_hwAES(hwAES),
        m_listener(listener)
    {
        m_async = std::make_shared<Async>(this);
    }


    ~JobResultsPrivate() override = default;


    inline void submit(const JobResult &result)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_results.push_back(result);

        m_async->send();
    }




protected:
    inline void onAsync() override  { submit(); }


private:
    inline void submit()
    {
        std::list<JobResult> results;

        m_mutex.lock();
        m_results.swap(results);
        m_mutex.unlock();

        for (const auto &result : results) {
            m_listener->onJobResult(result);
        }
    }

    const bool m_hwAES;
    IJobResultListener *m_listener;
    std::list<JobResult> m_results;
    std::mutex m_mutex;
    std::shared_ptr<Async> m_async;

};


static JobResultsPrivate *handler = nullptr;


} // namespace xmrig


void xmrig::JobResults::done(const Job &job)
{
    submit(JobResult(job));
}


void xmrig::JobResults::setListener(IJobResultListener *listener, bool hwAES)
{
    assert(handler == nullptr);

    handler = new JobResultsPrivate(listener, hwAES);
}


void xmrig::JobResults::stop()
{
    assert(handler != nullptr);

    delete handler;

    handler = nullptr;
}


void xmrig::JobResults::submit(const Job &job, uint32_t nonce, const uint8_t *result)
{
    submit(JobResult(job, nonce, result));
}


void xmrig::JobResults::submit(const Job& job, uint32_t nonce, const uint8_t* result, const uint8_t* miner_signature)
{
    submit(JobResult(job, nonce, result, nullptr, nullptr, miner_signature));
}


void xmrig::JobResults::submit(const JobResult &result)
{
    assert(handler != nullptr);

    if (handler) {
        handler->submit(result);
    }
}


