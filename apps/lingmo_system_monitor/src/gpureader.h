// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "metrics.h"
#include "processrecord.h"

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// One engine of a DRM client's fdinfo: busy time in nanoseconds, or busy
// and total GPU cycles where the driver counts those instead.
struct DrmEngine
{
    uint64_t busyTime = 0;
    uint64_t cycles = 0;
    uint64_t totalCycles = 0;
    int capacity = 1;
    bool hasTime = false;
    bool hasCycles = false;
};

struct DrmClient
{
    std::string id;
    std::map<std::string, DrmEngine> engines;
};

using DrmClients = std::unordered_map<std::string, DrmClient>;

class GpuReader
{
public:
    GpuReader(std::string procRoot, std::string sysRoot);

    void sample(Metrics* metrics, const std::vector<ProcessRecord>& processes, double elapsed);
    static DrmClient parseDrmFdinfo(std::string_view contents);
    static double drmBusy(const DrmClients& current, const DrmClients& previous, double elapsedTime);

private:
    struct Card
    {
        std::string device;
        std::string hwmon;
        std::string name;
        std::string driver;
        std::vector<std::string> nodes;
        DrmClients clients;
        bool primary = false;
        bool hasBusy = false;
        bool hasClientBusy = false;
        bool hasTemperature = false;
        bool hasFrequency = false;
        bool hasVram = false;
    };

    // The DRM descriptors a process held when they were last listed.
    struct Descriptors
    {
        uint64_t startTicks = 0;
        std::vector<std::pair<std::string, int>> nodes;
    };

    void scan();
    static std::string pciName(std::string_view vendor, std::string_view device);
    std::vector<std::pair<std::string, int>> drmDescriptors(const std::string& path) const;
    std::vector<DrmClients> drmClients(const std::vector<ProcessRecord>& processes,
                                       const std::vector<bool>& wanted);

    std::string m_procRoot;
    std::string m_sysRoot;
    uint32_t m_uid = 0;
    std::vector<Card> m_cards;
    std::unordered_map<std::string, int> m_nodeOwners;
    std::unordered_map<int, Descriptors> m_descriptors;
    uint64_t m_samples = 0;
    bool m_scanned = false;
};
