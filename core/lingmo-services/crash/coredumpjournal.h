// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "coredump.h"

#include <functional>
#include <string>

#include "base/timer.h"

struct sd_journal;

// Follows systemd-coredump's journal entries for this user's processes from
// the moment it starts; earlier crashes are not reported.
class CoredumpJournal
{
public:
    explicit CoredumpJournal(std::function<void(const Coredump &dump)> coredump);
    ~CoredumpJournal();

    CoredumpJournal(const CoredumpJournal &) = delete;
    CoredumpJournal &operator=(const CoredumpJournal &) = delete;

    bool isOpen() const { return m_journal != nullptr; }

private:
    void readNewEntries();
    void scheduleTimeout();
    Coredump currentEntry() const;
    std::string field(const char *name) const;

    std::function<void(const Coredump &dump)> m_coredump;
    sd_journal *m_journal = nullptr;
    int m_watch = 0;
    Lingmo::Base::Timer m_timeout;
};
