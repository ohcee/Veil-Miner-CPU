/* XMRig
 * Copyright (c) 2018-2025 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2025 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
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

#pragma once

#include "3rdparty/rapidjson/fwd.h"


namespace xmrig {


class IJsonReader;
class String;


class IConfig
{
public:
    enum Keys {
        // common
        AlgorithmKey         = 'a',
        CoinKey              = 1025,
        ApiWorkerIdKey       = 4002,
        ApiIdKey             = 4005,
        HttpPort             = 4100,
        HttpAccessTokenKey   = 4101,
        HttpRestrictedKey    = 4104,
        HttpEnabledKey       = 4106,
        HttpHostKey          = 4107,
        BackgroundKey        = 'B',
        ColorKey             = 1002,
        ConfigKey            = 'c',
        KeepAliveKey         = 'k',
        LogFileKey           = 'l',
        PasswordKey          = 'p',
        RetriesKey           = 'r',
        RetryPauseKey        = 'R',
        RigIdKey             = 1012,
        SyslogKey            = 'S',
        UrlKey               = 'o',
        UserAgentKey         = 1008,
        UserKey              = 'u',
        UserpassKey          = 'O',
        ProxyKey             = 'x',
        VerboseKey           = 1100,
        TlsKey               = 1013,
        FingerprintKey       = 1014,
        DataDirKey           = 1035,
        TitleKey             = 1037,
        NoTitleKey           = 1038,
        PauseOnBatteryKey    = 1041,
        DmiKey               = 1049,
        HugePageSizeKey      = 1050,
        PauseOnActiveKey     = 1051,
        DnsIPv4Key           = '4',
        DnsIPv6Key           = '6',
        DnsTtlKey            = 1054,
        HugePagesJitKey      = 1057,

        // xmrig common
        CPUPriorityKey       = 1021,
        NicehashKey          = 1006,
        PrintTimeKey         = 1007,

        // xmrig cpu
        CPUKey               = 1024,
        CPUAffinityKey       = 1020,
        DryRunKey            = 5000,
        HugePagesKey         = 1009,
        ThreadsKey           = 't',
        AssemblyKey          = 1015,
        RandomXInitKey       = 1022,
        RandomXNumaKey       = 1023,
        RandomXModeKey       = 1029,
        RandomX1GbPagesKey   = 1031,
        RandomXWrmsrKey      = 1032,
        RandomXRdmsrKey      = 1033,
        CPUMaxThreadsKey     = 1026,
        MemoryPoolKey        = 1027,
        YieldKey             = 1030,
        RandomXCacheQoSKey   = 1040,

        // xmrig-proxy
        AccessLogFileKey     = 'A',
        BindKey              = 'b',
        CustomDiffKey        = 1102,
        CustomDiffStatsKey   = 1104,
        DebugKey             = 1101,
        ModeKey              = 'm',
        PoolCoinKey          = 'C',
        ReuseTimeoutKey      = 1106,
        WorkersKey           = 1103,
        WorkersAdvKey        = 1107,
        TlsBindKey           = 1108,
        TlsCertKey           = 1109,
        TlsCertKeyKey        = 1110,
        TlsDHparamKey        = 1111,
        TlsCiphersKey        = 1112,
        TlsCipherSuitesKey   = 1113,
        TlsProtocolsKey      = 1114,
        TlsGenKey            = 1117,
        AlgoExtKey           = 1115,
        ProxyPasswordKey     = 1116,
        LoginFileKey         = 'L',
    };

    virtual ~IConfig() = default;

    virtual bool isWatch() const                                       = 0;
    virtual bool read(const IJsonReader &reader, const char *fileName) = 0;
    virtual bool save()                                                = 0;
    virtual const String &fileName() const                             = 0;
    virtual void getJSON(rapidjson::Document &doc) const               = 0;
    virtual void setFileName(const char *fileName)                     = 0;
};


} // namespace xmrig
