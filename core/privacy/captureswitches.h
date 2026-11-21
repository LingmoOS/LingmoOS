// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>

// Every sound card's capture switches, turned off under ALSA element locks.
// Only the file that took a lock can write the element or release it, so no
// other process, the sound server included, can turn a microphone back on.
// The kernel drops the locks when this process exits.
class CaptureSwitches
{
public:
    explicit CaptureSwitches(const std::string &deviceDirectory = "/dev/snd");
    ~CaptureSwitches();

    CaptureSwitches(const CaptureSwitches &) = delete;
    CaptureSwitches &operator=(const CaptureSwitches &) = delete;

    bool isLocked() const { return m_locked; }
    void setLocked(bool locked);

private:
    void lockCard(const std::string &name);
    void unlockCard(const std::string &name);
    void readEvents();

    std::string m_deviceDirectory;
    bool m_locked = false;
    // Control node name to the open file holding its locks.
    std::map<std::string, int> m_cards;
    int m_inotify = -1;
    int m_watch = 0;
};
