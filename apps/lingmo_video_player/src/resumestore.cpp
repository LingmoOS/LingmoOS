// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resumestore.h"

#include "base/hash.h"
#include "base/settings.h"
#include "base/strings.h"

#include <algorithm>
#include <chrono>

using namespace Lingmo;
namespace strings = base::strings;

static constexpr const char* kGroup = "Resume";
static constexpr double kMinimumPosition = 60;
static constexpr double kMinimumDuration = 180;

ResumeStore::ResumeStore(std::string file)
    : m_file(std::move(file))
{
}

std::string ResumeStore::keyFor(const std::string& url)
{
    // Paths hold characters settings keys cannot.
    return base::CryptographicHash::hashHex(url, base::CryptographicHash::Algorithm::Sha256);
}

bool ResumeStore::worthKeeping(double position, double duration)
{
    return duration >= kMinimumDuration && position >= kMinimumPosition && position < duration * 0.95;
}

// Stored as: position, time saved, audio track, subtitle track, subtitle delay.
std::optional<ResumeStore::Entry> ResumeStore::entry(const std::string& url) const
{
    const base::Settings settings(m_file);
    const std::vector<std::string> fields = settings.stringListValue(std::string(kGroup) + '/' + keyFor(url));
    if (fields.size() < 5)
        return std::nullopt;
    Entry entry;
    entry.position = std::max(0.0, strings::toDouble(fields[0]).value_or(0));
    entry.audioTrack = strings::toNumber<int>(fields[2]).value_or(0);
    entry.subtitleTrack = strings::toNumber<int>(fields[3]).value_or(0);
    entry.subtitleDelay = strings::toDouble(fields[4]).value_or(0);
    return entry;
}

double ResumeStore::position(const std::string& url) const
{
    const auto found = entry(url);
    return found ? found->position : 0;
}

void ResumeStore::save(const std::string& url, double position, double duration, const Entry& choices)
{
    if (url.empty())
        return;
    if (!worthKeeping(position, duration)) {
        forget(url);
        return;
    }

    base::Settings settings(m_file);
    const std::string key = keyFor(url);
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    settings.setValue(std::string(kGroup) + '/' + key, std::vector<std::string>{
        strings::number(position, 'f', 3),
        strings::number((long long)now),
        strings::number(choices.audioTrack),
        strings::number(choices.subtitleTrack),
        strings::number(choices.subtitleDelay, 'f', 3),
    });

    std::vector<std::string> keys = settings.childKeys(kGroup);
    if (keys.size() <= size_t(Capacity))
        return;
    auto stamp = [&settings](const std::string& name) {
        const std::vector<std::string> fields = settings.stringListValue(std::string(kGroup) + '/' + name);
        return fields.size() > 1 ? strings::toNumber<long long>(fields[1]).value_or(0) : 0;
    };
    std::vector<std::pair<long long, std::string>> stamped;
    stamped.reserve(keys.size());
    for (std::string& other : keys) {
        // Saved within the same millisecond as others, the newest still stays.
        if (other != key)
            stamped.emplace_back(stamp(other), std::move(other));
    }
    std::stable_sort(stamped.begin(), stamped.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (size_t i = 0; i + size_t(Capacity) < keys.size(); ++i)
        settings.remove(std::string(kGroup) + '/' + stamped[i].second);
}

void ResumeStore::save(const std::string& url, double position, double duration)
{
    save(url, position, duration, Entry());
}

void ResumeStore::forget(const std::string& url)
{
    base::Settings settings(m_file);
    settings.remove(std::string(kGroup) + '/' + keyFor(url));
}

void ResumeStore::clear()
{
    base::Settings settings(m_file);
    settings.remove(kGroup);
}
