// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "journalstream.h"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#include <systemd/sd-journal.h>

namespace Lingmo {

static constexpr char kJournalStreamSocket[] = "/run/systemd/journal/stdout";

static bool writesToJournal()
{
    sockaddr_un address {};
    socklen_t length = sizeof(address);
    if (::getpeername(STDERR_FILENO, reinterpret_cast<sockaddr *>(&address), &length) != 0
        || address.sun_family != AF_UNIX)
        return false;
    return std::strncmp(address.sun_path, kJournalStreamSocket,
                        sizeof(address.sun_path)) == 0;
}

static int openStream(const QString &identifier)
{
    if (!writesToJournal())
        return -1;
    return sd_journal_stream_fd(identifier.toLocal8Bit().constData(), LOG_INFO, 1);
}

void useOwnJournalStream(const QString &identifier)
{
    const int stream = openStream(identifier);
    if (stream < 0)
        return;
    std::fflush(stdout);
    ::dup2(stream, STDOUT_FILENO);
    ::dup2(stream, STDERR_FILENO);
    ::close(stream);
}

bool startWithOwnJournalStream(Lingmo::Base::Process &process,
                                const QString &program,
                                const QStringList &arguments)
{
    const QString name = QString::fromStdString(
        std::filesystem::path(program.toStdString()).filename().string());
    const int stream = openStream(name);
    if (stream < 0)
        return process.start(program.toStdString(), toStdStrings(arguments));

    const int output = ::fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 10);
    const int errors = ::fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 10);
    if (output < 0 || errors < 0) {
        if (output >= 0)
            ::close(output);
        if (errors >= 0)
            ::close(errors);
        ::close(stream);
        return process.start(program.toStdString(), toStdStrings(arguments));
    }

    // The program inherits this process's output and errors as it starts,
    // so they are the stream only for that moment.
    std::fflush(stdout);
    ::dup2(stream, STDOUT_FILENO);
    ::dup2(stream, STDERR_FILENO);
    const bool started = process.start(program.toStdString(), toStdStrings(arguments));
    ::dup2(output, STDOUT_FILENO);
    ::dup2(errors, STDERR_FILENO);

    ::close(output);
    ::close(errors);
    ::close(stream);
    return started;
}

} // namespace Lingmo
