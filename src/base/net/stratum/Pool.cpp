/* XMRig
 * Copyright (c) 2019      Howard Chu  <https://github.com/hyc>
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
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>


#include "base/net/stratum/Pool.h"
#include "3rdparty/rapidjson/document.h"
#include "base/io/json/Json.h"
#include "base/io/log/Log.h"
#include "base/kernel/Platform.h"
#include "base/net/stratum/Client.h"




#ifdef _MSC_VER
#   define strcasecmp  _stricmp
#endif


namespace xmrig {


const String Pool::kDefaultPassword       = "x";
const String Pool::kDefaultUser           = "x";


const char *Pool::kAlgo                   = "algo";
const char *Pool::kCoin                   = "coin";
const char *Pool::kEnabled                = "enabled";
const char *Pool::kFingerprint            = "tls-fingerprint";
const char *Pool::kKeepalive              = "keepalive";
const char *Pool::kNicehash               = "nicehash";
const char *Pool::kPass                   = "pass";
const char *Pool::kRigId                  = "rig-id";
const char *Pool::kSOCKS5                 = "socks5";
const char *Pool::kTls                    = "tls";
const char *Pool::kSni                    = "sni";
const char *Pool::kUrl                    = "url";
const char *Pool::kUser                   = "user";
const char *Pool::kNicehashHost           = "nicehash.com";


} // namespace xmrig


xmrig::Pool::Pool(const char *url) :
    m_flags(1 << FLAG_ENABLED),
    m_url(url)
{
}


xmrig::Pool::Pool(const rapidjson::Value &object) :
    m_flags(1 << FLAG_ENABLED),
    m_url(Json::getString(object, kUrl))
{
    if (!m_url.isValid()) {
        return;
    }

    m_user           = Json::getString(object, kUser);
    m_password       = Json::getString(object, kPass);
    m_rigId          = Json::getString(object, kRigId);
    m_fingerprint    = Json::getString(object, kFingerprint);
    m_algorithm      = Json::getString(object, kAlgo);
    m_coin           = Json::getString(object, kCoin);
    m_proxy          = Json::getValue(object, kSOCKS5);

    m_flags.set(FLAG_ENABLED,  Json::getBool(object, kEnabled, true));
    m_flags.set(FLAG_NICEHASH, Json::getBool(object, kNicehash) || m_url.host().contains(kNicehashHost));
    m_flags.set(FLAG_TLS,      Json::getBool(object, kTls) || m_url.isTLS());
    m_flags.set(FLAG_SNI,      Json::getBool(object, kSni));

    setKeepAlive(Json::getValue(object, kKeepalive));
}




bool xmrig::Pool::isEnabled() const
{
#   ifndef XMRIG_FEATURE_TLS
    if (isTLS()) {
        return false;
    }
#   endif

    return m_flags.test(FLAG_ENABLED) && isValid();
}


bool xmrig::Pool::isEqual(const Pool &other) const
{
    return (m_flags           == other.m_flags
            && m_keepAlive    == other.m_keepAlive
            && m_algorithm    == other.m_algorithm
            && m_coin         == other.m_coin
            && m_mode         == other.m_mode
            && m_fingerprint  == other.m_fingerprint
            && m_password     == other.m_password
            && m_rigId        == other.m_rigId
            && m_url          == other.m_url
            && m_user         == other.m_user
            && m_proxy        == other.m_proxy
            );
}


xmrig::IClient *xmrig::Pool::createClient(int id, IClientListener *listener) const
{
    IClient *client = new Client(id, Platform::userAgent(), listener);

    client->setPool(*this);

    return client;
}


rapidjson::Value xmrig::Pool::toJSON(rapidjson::Document &doc) const
{
    using namespace rapidjson;

    auto &allocator = doc.GetAllocator();

    Value obj(kObjectType);

    obj.AddMember(StringRef(kAlgo),  m_algorithm.toJSON(), allocator);
    obj.AddMember(StringRef(kCoin),  m_coin.toJSON(), allocator);
    obj.AddMember(StringRef(kUrl),   url().toJSON(), allocator);
    obj.AddMember(StringRef(kUser),  m_user.toJSON(), allocator);
    obj.AddMember(StringRef(kPass),  m_password.toJSON(), allocator);
    obj.AddMember(StringRef(kRigId), m_rigId.toJSON(), allocator);

#   ifndef XMRIG_PROXY_PROJECT
    obj.AddMember(StringRef(kNicehash), isNicehash(), allocator);
#   endif

    if (m_keepAlive == 0 || m_keepAlive == kKeepAliveTimeout) {
        obj.AddMember(StringRef(kKeepalive), m_keepAlive > 0, allocator);
    }
    else {
        obj.AddMember(StringRef(kKeepalive), m_keepAlive, allocator);
    }

    obj.AddMember(StringRef(kEnabled),      m_flags.test(FLAG_ENABLED), allocator);
    obj.AddMember(StringRef(kTls),          isTLS(), allocator);
    obj.AddMember(StringRef(kSni),          isSNI(), allocator);
    obj.AddMember(StringRef(kFingerprint),  m_fingerprint.toJSON(), allocator);
    obj.AddMember(StringRef(kSOCKS5),       m_proxy.toJSON(doc), allocator);

    return obj;
}


std::string xmrig::Pool::printableName() const
{
    std::string out(CSI "1;" + std::to_string(isEnabled() ? (isTLS() ? 32 : 36) : 31) + "m" + url().data() + CLEAR);

    if (m_coin.isValid()) {
        out += std::string(" coin ") + WHITE_BOLD_S + m_coin.name() + CLEAR;
    }
    else {
        out += std::string(" algo ") + WHITE_BOLD_S + (m_algorithm.isValid() ? m_algorithm.name() : "auto") + CLEAR;
    }

    return out;
}


#ifdef APP_DEBUG
void xmrig::Pool::print() const
{
    LOG_NOTICE("url:       %s", url().data());
    LOG_DEBUG ("host:      %s", host().data());
    LOG_DEBUG ("port:      %d", static_cast<int>(port()));
    LOG_DEBUG ("user:      %s", m_user.data());
    LOG_DEBUG ("pass:      %s", m_password.data());
    LOG_DEBUG ("rig-id     %s", m_rigId.data());
    LOG_DEBUG ("algo:      %s", m_algorithm.name());
    LOG_DEBUG ("nicehash:  %d", static_cast<int>(m_flags.test(FLAG_NICEHASH)));
    LOG_DEBUG ("keepAlive: %d", m_keepAlive);
}
#endif


void xmrig::Pool::setKeepAlive(const rapidjson::Value &value)
{
    if (value.IsInt()) {
        setKeepAlive(value.GetInt());
    }
    else if (value.IsBool()) {
        setKeepAlive(value.GetBool());
    }
}
