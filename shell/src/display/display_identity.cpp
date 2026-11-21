// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display/display_identity.h"

#include <algorithm>
#include <map>

#include "base/uuid.h"

namespace Lingmo::Display {
namespace {
std::string baseKey(const DisplaySnapshot &display)
{
    if (display.connectionType == DisplayConnectionType::Virtual)
        return "virtual:" + display.portKey;
    return display.edidKey.empty() ? "port:" + display.portKey : "edid:" + display.edidKey;
}
} // namespace

void DisplayIdentity::rememberPersistentKeys(const std::vector<std::string> &keys)
{
    for (const auto &key : keys) {
        if (key.starts_with("edid:") && key.find('@') != std::string::npos)
            m_ambiguous.insert(key.substr(0, key.find('@')));
    }
}

void DisplayIdentity::assign(std::vector<DisplaySnapshot> &displays)
{
    std::sort(displays.begin(), displays.end(), [](const auto &a, const auto &b) {
        return a.portKey < b.portKey;
    });
    std::map<std::string, int> counts;
    for (const auto &display : displays)
        ++counts[baseKey(display)];
    std::set<DisplayId> used;
    for (auto &display : displays) {
        const std::string base = baseKey(display);
        bool duplicate = counts[base] > 1 || m_ambiguous.contains(base);
        for (const auto &known : m_known)
            duplicate |= baseKey(known) == base
                    && known.persistentKey.starts_with(base + '@');
        if (duplicate)
            m_ambiguous.insert(base);
        display.persistentKey = duplicate ? base + '@' + display.portKey : base;
        const DisplaySnapshot *match = nullptr;
        for (const auto &known : m_known) {
            if (baseKey(known) == base && known.portKey == display.portKey
                && !used.contains(known.sessionId)) {
                match = &known;
                break;
            }
        }
        if (!match && !duplicate) {
            for (const auto &known : m_known) {
                if (baseKey(known) == base && !used.contains(known.sessionId)) {
                    match = &known;
                    break;
                }
            }
        }
        display.sessionId = match ? match->sessionId
                                  : DisplayId("output-" + base::Uuid::generate().toString());
        used.insert(display.sessionId);
    }
    for (const auto &display : displays) {
        auto it = std::find_if(m_known.begin(), m_known.end(), [&](const auto &known) {
            return known.sessionId == display.sessionId;
        });
        if (it == m_known.end())
            m_known.push_back(display);
        else
            *it = display;
    }
}
} // namespace Lingmo::Display
