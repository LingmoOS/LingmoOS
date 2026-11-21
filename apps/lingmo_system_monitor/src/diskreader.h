// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "metrics.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class DiskReader
{
public:
    DiskReader(std::string procRoot, std::string sysRoot);

    void sample(Metrics* metrics, double elapsed);
    // The mounted file systems a user stores files on, from mountinfo.
    static std::vector<Volume> volumes(const std::string& mountInfo = "/proc/self/mountinfo");
    // A mountinfo path with its octal escapes decoded.
    static std::string unescapePath(std::string_view path);

private:
    std::string m_procRoot;
    std::string m_sysRoot;
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> m_previous;
};
