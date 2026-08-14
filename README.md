# XMRig-Veil

A CPU miner for [Veil](https://veil-project.com) and nothing else.

This is a fork of [XMRig](https://github.com/xmrig/xmrig) 6.25.0 stripped down to a single algorithm: `rx/veil`, the RandomX variant Veil uses for CPU mining. Veil hashes RandomX over the double SHA256 of the block header, and this miner caches the SHA256 midstate so only the nonce tail is rehashed on every attempt.

There is no donation and no dev fee. All hashrate goes to the address you configure.

## What it keeps from XMRig

* The full RandomX engine with JIT, hardware AES, huge pages and 1GB pages, MSR mod and NUMA support.
* Automatic CPU thread configuration.
* Stratum over TCP and TLS with failover between pools.
* A localhost JSON API for stats if you turn on the `http` section.

## What was removed

* Every other algorithm. CryptoNight, KawPow, GhostRider, Argon2 and the other RandomX variants are gone from the source tree, not just disabled. The config parser rejects anything that is not `rx/veil`.
* The OpenCL and CUDA backends. Veil GPU mining is ProgPow and lives in other miners; this one is CPU only.
* All donation and dev fee code.
* The benchmark, Monero solo mining and everything else that only served other coins.

## Build

Dependencies are cmake, libuv, openssl and hwloc.

Linux:

```
sudo apt install -y cmake build-essential libuv1-dev libssl-dev libhwloc-dev
git clone https://github.com/ohcee/xmrig-veil.git
cd xmrig-veil
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

macOS:

```
brew install cmake libuv openssl hwloc
git clone https://github.com/ohcee/xmrig-veil.git
cd xmrig-veil
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
cmake --build build -j
```

Windows builds work with Visual Studio 2022 and vcpkg, see the CI workflow for the exact flags.

## Usage

Point it at a pool that serves `rx/veil` work and give it your Veil address:

```
./xmrig -o POOL_HOST:3333 -u YOUR_VEIL_ADDRESS
```

Or use a config file, which is the better way. The miner looks for `config.json` next to the binary, then `~/.xmrig-veil.json` and `~/.config/xmrig-veil.json`. A minimal one:

```json
{
    "cpu": true,
    "pools": [
        {
            "algo": "rx/veil",
            "coin": "veil",
            "url": "POOL_HOST:3333",
            "user": "YOUR_VEIL_ADDRESS",
            "keepalive": true,
            "tls": false
        }
    ]
}
```

The full set of options is in [src/config.json](src/config.json). Huge pages make a real difference for RandomX, so run with them if you can. On Linux that usually means `sudo sysctl -w vm.nr_hugepages=1280` or letting the miner do it as root.

## License

GPLv3, same as upstream. This fork exists because of the work of the [XMRig](https://github.com/xmrig) developers and [sech1](https://github.com/SChernykh), who wrote the fast RandomX code this miner is built on.
