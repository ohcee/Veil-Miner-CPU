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

#ifndef XMRIG_ALGORITHM_H
#define XMRIG_ALGORITHM_H


#include <functional>
#include <vector>


#include "3rdparty/rapidjson/fwd.h"


namespace xmrig {


class Algorithm
{
public:
    // Id encoding:
    // 1 byte: family
    // 1 byte: L3 memory as power of 2.
    // 1 byte: L2 memory as power of 2.
    // 1 byte: extra variant (coin) id.
    enum Id : uint32_t {
        INVALID         = 0,
        RX_VEIL         = 0x72151201,   // "rx/veil"          RandomX (Veil).
    };

    enum Family : uint32_t {
        UNKNOWN         = 0,
        RANDOM_X        = 0x72000000,
    };

    static const char *kINVALID;

    static const char *kRX;
    static const char *kRX_VEIL;

    inline Algorithm() = default;
    inline Algorithm(const char *algo) : m_id(parse(algo))  {}
    inline Algorithm(Id id) : m_id(id)                      {}
    Algorithm(const rapidjson::Value &value);
    Algorithm(uint32_t id);

    static inline constexpr size_t l2(Id id)                { return family(id) == RANDOM_X ? (1U << ((id >> 8) & 0xff)) : 0U; }
    static inline constexpr size_t l3(Id id)                { return 1ULL << ((id >> 16) & 0xff); }
    static inline constexpr uint32_t family(Id id)          { return id & 0xff000000; }

    inline bool isEqual(const Algorithm &other) const       { return m_id == other.m_id; }
    inline bool isValid() const                             { return m_id != INVALID && family() > UNKNOWN; }
    inline Id id() const                                    { return m_id; }
    inline size_t l2() const                                { return l2(m_id); }
    inline uint32_t family() const                          { return family(m_id); }
    inline uint32_t minIntensity() const                    { return 1; };
    inline uint32_t maxIntensity() const                    { return 1; };

    inline size_t l3() const                                { return l3(m_id); }

    inline bool operator!=(Algorithm::Id id) const          { return m_id != id; }
    inline bool operator!=(const Algorithm &other) const    { return !isEqual(other); }
    inline bool operator==(Algorithm::Id id) const          { return m_id == id; }
    inline bool operator==(const Algorithm &other) const    { return isEqual(other); }
    inline operator Algorithm::Id() const                   { return m_id; }

    const char *name() const;
    rapidjson::Value toJSON() const;
    rapidjson::Value toJSON(rapidjson::Document &doc) const;

    static Id parse(const char *name);
    static size_t count();
    static std::vector<Algorithm> all(const std::function<bool(const Algorithm &algo)> &filter = nullptr);

private:
    Id m_id = INVALID;
};


using Algorithms = std::vector<Algorithm>;


} /* namespace xmrig */


#endif /* XMRIG_ALGORITHM_H */
