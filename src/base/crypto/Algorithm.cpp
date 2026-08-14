/* XMRig
 * Copyright (c) 2018      Lee Clagett <https://github.com/vtnerd>
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

#include "base/crypto/Algorithm.h"
#include "3rdparty/rapidjson/document.h"
#include "base/tools/String.h"


#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <map>


#ifdef _MSC_VER
#   define strcasecmp  _stricmp
#endif


namespace xmrig {


const char *Algorithm::kINVALID         = "invalid";
const char *Algorithm::kRX              = "rx";
const char *Algorithm::kRX_VEIL         = "rx/veil";


#define ALGO_NAME(ALGO)         { Algorithm::ALGO, Algorithm::k##ALGO }
#define ALGO_ALIAS(ALGO, NAME)  { NAME, Algorithm::ALGO }
#define ALGO_ALIAS_AUTO(ALGO)   { Algorithm::k##ALGO, Algorithm::ALGO }


static const std::map<uint32_t, const char *> kAlgorithmNames = {
    ALGO_NAME(RX_VEIL),
};


struct aliasCompare
{
   inline bool operator()(const char *a, const char *b) const   { return strcasecmp(a, b) < 0; }
};


static const std::map<const char *, Algorithm::Id, aliasCompare> kAlgorithmAliases = {
    ALGO_ALIAS_AUTO(RX_VEIL),       ALGO_ALIAS(RX_VEIL,         "randomx/veil"),
                                    ALGO_ALIAS(RX_VEIL,         "randomveil"),
                                    ALGO_ALIAS(RX_VEIL,         "randomx"),
                                    ALGO_ALIAS(RX_VEIL,         "rx"),
};


} /* namespace xmrig */


xmrig::Algorithm::Algorithm(const rapidjson::Value &value) :
    m_id(parse(value.GetString()))
{
}


xmrig::Algorithm::Algorithm(uint32_t id) :
    m_id(kAlgorithmNames.count(id) ? static_cast<Id>(id) : INVALID)
{
}


const char *xmrig::Algorithm::name() const
{
    if (!isValid()) {
        return kINVALID;
    }

    assert(kAlgorithmNames.count(m_id));
    const auto it = kAlgorithmNames.find(m_id);

    return it != kAlgorithmNames.end() ? it->second : kINVALID;
}


rapidjson::Value xmrig::Algorithm::toJSON() const
{
    using namespace rapidjson;

    return isValid() ? Value(StringRef(name())) : Value(kNullType);
}


rapidjson::Value xmrig::Algorithm::toJSON(rapidjson::Document &) const
{
    return toJSON();
}


xmrig::Algorithm::Id xmrig::Algorithm::parse(const char *name)
{
    if (name == nullptr || strlen(name) < 1) {
        return INVALID;
    }

    const auto it = kAlgorithmAliases.find(name);

    return it != kAlgorithmAliases.end() ? it->second : INVALID;
}


size_t xmrig::Algorithm::count()
{
    return kAlgorithmNames.size();
}


std::vector<xmrig::Algorithm> xmrig::Algorithm::all(const std::function<bool(const Algorithm &algo)> &filter)
{
    static const std::vector<Id> order = {
        RX_VEIL,
    };

    Algorithms out;
    out.reserve(count());

    for (const Id algo : order) {
        if (kAlgorithmNames.count(algo) && (!filter || filter(algo))) {
            out.emplace_back(algo);
        }
    }

    return out;
}
