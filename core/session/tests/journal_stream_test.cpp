// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <filesystem>
#include <sys/stat.h>
#include <syslog.h>
#include <thread>
#include <unistd.h>

#include <gtest/gtest.h>
#include <systemd/sd-journal.h>

#include <QString>
#include <QStringList>

#include "base/file.h"
#include "base/process.h"
#include "journalstream.h"
#include "testutil.h"

using Lingmo::Base::EventLoop;
using Lingmo::Base::Process;
using Lingmo::Base::file::TemporaryDirectory;
using SessionTest::runUntil;

static ino_t inode(int fd)
{
    struct stat status {};
    return ::fstat(fd, &status) == 0 ? status.st_ino : 0;
}

static QString journalLines(EventLoop &loop, const QString &identifier)
{
    Process journal(&loop);
    bool finished = false;
    journal.onFinished = [&](int, bool) { finished = true; };
    if (!journal.start(QStringLiteral("journalctl"),
                       {QStringLiteral("--quiet"),
                        QStringLiteral("--output=cat"),
                        QStringLiteral("--identifier=") + identifier}))
        return {};
    runUntil(loop, [&] { return finished; });
    return QString::fromStdString(journal.standardOutput());
}

TEST(JournalStream, childWritesUnderItsProgramName)
{
    EventLoop loop;
    const int stream = sd_journal_stream_fd("lingmo-session-test", LOG_INFO, 1);
    if (stream < 0)
        GTEST_SKIP() << "no journal";

    TemporaryDirectory root(SessionTest::temporaryParent().toStdString());
    const QString name = QStringLiteral("lingmo-journal-stream-test-")
                       + QString::number(::getpid());
    const QString program = QString::fromStdString(root.path()) + QLatin1Char('/') + name;
    std::filesystem::create_symlink("/bin/sh", program.toStdString());

    const int errors = ::dup(STDERR_FILENO);
    ASSERT_GE(errors, 0);
    ASSERT_EQ(::dup2(stream, STDERR_FILENO), STDERR_FILENO);
    const ino_t ownStream = inode(STDERR_FILENO);

    Process child(&loop);
    child.setOutputForwarded(true);
    bool finished = false;
    child.onFinished = [&](int, bool) { finished = true; };
    const bool started = Lingmo::startWithOwnJournalStream(
        child, program,
        {QStringLiteral("-c"), QStringLiteral("echo output; echo errors >&2")});
    const ino_t afterStart = inode(STDERR_FILENO);

    ::dup2(errors, STDERR_FILENO);
    ::close(errors);
    ::close(stream);

    ASSERT_TRUE(started);
    EXPECT_EQ(afterStart, ownStream);
    ASSERT_TRUE(runUntil(loop, [&] { return finished; }));

    QString lines;
    for (int attempt = 0; attempt < 50 && lines != QLatin1String("output\nerrors\n"); ++attempt) {
        lines = journalLines(loop, name);
        if (lines.isEmpty())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    EXPECT_EQ(lines, QStringLiteral("output\nerrors\n"));
}
