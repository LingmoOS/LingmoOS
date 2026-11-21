// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "metrics.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

class NetworkReader
{
public:
    NetworkReader(std::string procRoot, std::string sysRoot);

    void sample(Metrics* metrics, double elapsed);

private:
    std::string m_procRoot;
    std::string m_sysRoot;
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> m_previous;
};
