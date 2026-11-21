// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coredumpjournal.h"

#include <algorithm>
#include <cstring>
#include <sys/epoll.h>
#include <systemd/sd-journal.h>
#include <time.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/strings.h"
#include "log.h"

using namespace Lingmo::Base;
using namespace Lingmo::Base::strings;

// systemd-coredump's "Process … dumped core." message.
static const char s_coredumpMessageId[] = "MESSAGE_ID=fc2e22bc6ee647b6b90729ab34a250b1";

CoredumpJournal::CoredumpJournal(std::function<void(const Coredump &dump)> coredump)
    : m_coredump(std::move(coredump))
{
    m_timeout.setSingleShot(true);
    m_timeout.onTimeout = [this] {
        readNewEntries();
    };

    // Without a persistent journal, entries land in the system journal, which
    // only the adm and systemd-journal groups may read.
    if (sd_journal_open(&m_journal, SD_JOURNAL_LOCAL_ONLY) < 0) {
        LM_WARNING(services::log()) << "Crash reports: cannot open the journal";
        m_journal = nullptr;
        return;
    }
    // Before anything else, so no change goes unwatched.
    const int fd = sd_journal_get_fd(m_journal);
    if (fd < 0) {
        LM_WARNING(services::log()) << "Crash reports: cannot watch the journal";
        sd_journal_close(m_journal);
        m_journal = nullptr;
        return;
    }
    const std::string uid = "COREDUMP_UID=" + std::to_string(::getuid());
    sd_journal_add_match(m_journal, s_coredumpMessageId, 0);
    sd_journal_add_match(m_journal, uid.c_str(), 0);
    // The stack trace of a process with many threads outgrows the 64 KiB default.
    sd_journal_set_data_threshold(m_journal, 0);

    sd_journal_seek_tail(m_journal);
    // Steps onto the last entry, so the next one is the first new entry.
    sd_journal_previous(m_journal);

    m_watch = EventLoop::current()->watch(fd, uint32_t(sd_journal_get_events(m_journal)), [this](uint32_t) {
        readNewEntries();
    });
    scheduleTimeout();
}

CoredumpJournal::~CoredumpJournal()
{
    if (m_watch)
        EventLoop::current()->unwatch(m_watch);
    if (m_journal)
        sd_journal_close(m_journal);
}

void CoredumpJournal::readNewEntries()
{
    if (!m_journal)
        return;
    sd_journal_process(m_journal);
    while (sd_journal_next(m_journal) > 0) {
        if (m_coredump)
            m_coredump(currentEntry());
    }
    scheduleTimeout();
}

void CoredumpJournal::scheduleTimeout()
{
    // Some journal changes only show up after a timeout, not on the fd.
    uint64_t until = 0;
    if (sd_journal_get_timeout(m_journal, &until) < 0 || until == uint64_t(-1)) {
        m_timeout.stop();
        return;
    }
    timespec now {};
    clock_gettime(CLOCK_MONOTONIC, &now);
    const uint64_t nowUs = uint64_t(now.tv_sec) * 1000000 + uint64_t(now.tv_nsec) / 1000;
    const int64_t delayMs = until > nowUs ? int64_t((until - nowUs + 999) / 1000) : 0;
    m_timeout.start(std::chrono::milliseconds(std::min<int64_t>(delayMs, 60000)));
}

std::string CoredumpJournal::field(const char *name) const
{
    const void *data = nullptr;
    size_t length = 0;
    if (sd_journal_get_data(m_journal, name, &data, &length) < 0)
        return {};
    const size_t prefix = strlen(name) + 1;
    if (length < prefix)
        return {};
    return std::string(static_cast<const char *>(data) + prefix, length - prefix);
}

Coredump CoredumpJournal::currentEntry() const
{
    Coredump dump;
    dump.pid = toNumber<uint32_t>(field("COREDUMP_PID")).value_or(0);
    dump.signal = toNumber<int>(field("COREDUMP_SIGNAL")).value_or(0);
    dump.signalName = field("COREDUMP_SIGNAL_NAME");
    dump.executable = field("COREDUMP_EXE");
    dump.commandLine = field("COREDUMP_CMDLINE");
    dump.userUnit = field("COREDUMP_USER_UNIT");
    dump.cgroup = field("COREDUMP_CGROUP");
    // Seconds precision, in microseconds.
    const int64_t timestamp = toNumber<int64_t>(field("COREDUMP_TIMESTAMP")).value_or(0);
    if (timestamp > 0)
        dump.time = timestamp / 1000;
    dump.threads = Coredump::parseStackTrace(field("MESSAGE"));
    return dump;
}
