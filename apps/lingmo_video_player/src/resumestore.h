// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>

// Where each recently watched file was left, with its audio and subtitle
// choices, so it reopens there. Only the most recent entries are kept.
class ResumeStore
{
public:
    static constexpr int Capacity = 200;

    struct Entry
    {
        // Seconds.
        double position = 0;
        // mpv track ids: 0 is off, -1 the file's default.
        int audioTrack = -1;
        int subtitleTrack = -1;
        double subtitleDelay = 0;
    };

    explicit ResumeStore(std::string file);

    std::optional<Entry> entry(const std::string& url) const;
    double position(const std::string& url) const;
    // Remembers where url was left, or forgets it when that is not worth
    // returning to: a short file, the first minute, or the end. The entry's
    // own position is ignored; without one, tracks are the file's defaults.
    void save(const std::string& url, double position, double duration, const Entry& choices);
    void save(const std::string& url, double position, double duration);
    void forget(const std::string& url);
    void clear();

    static bool worthKeeping(double position, double duration);

private:
    static std::string keyFor(const std::string& url);

    std::string m_file;
};
