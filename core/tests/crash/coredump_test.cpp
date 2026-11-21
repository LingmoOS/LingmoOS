// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "coredump.h"

#include <csignal>

// As systemd-coredump 257 writes MESSAGE.
static const char s_message[] =
        "Process 2112053 (svgrender) of user 1000 dumped core.\n"
        "\n"
        "Module libstdc++.so.6 from deb gcc-14-14.2.0-19.amd64\n"
        "Stack trace of thread 2112053:\n"
        "#0  0x00007fe7192a069c n/a (libc.so.6 + 0x9469c)\n"
        "#1  0x00007fe71924bc42 raise (libc.so.6 + 0x3fc42)\n"
        "#4  0x00007fe7198dcd32 _ZNK14QMessageLogger5fatalEPKcz (libQt6Core.so.6 + 0xdcd32)\n"
        "#10 0x00005650c8d33206 main (/usr/local/bin/svgrender + 0x1206)\n"
        "#11 0x0000000000000000 n/a (n/a + 0x0)\n"
        "\n"
        "Stack trace of thread 2112056:\n"
        "#0  0x00007fe7192a672e n/a (libc.so.6 + 0x9a72e)\n"
        "#1  0x00007fe71931c538 n/a (libc.so.6 + 0x110538)\n"
        "ELF object binary architecture: AMD x86-64";

TEST(CoredumpTest, ParsesThreadsAndFrames)
{
    const auto threads = Coredump::parseStackTrace(s_message);
    ASSERT_EQ(threads.size(), 2u);
    EXPECT_EQ(threads[0].id, "2112053");
    ASSERT_EQ(threads[0].frames.size(), 5u);
    EXPECT_EQ(threads[0].frames[0].function, "");
    EXPECT_EQ(threads[0].frames[0].module, "libc.so.6");
    EXPECT_EQ(threads[0].frames[1].function, "raise");
    EXPECT_EQ(threads[0].frames[2].function, "QMessageLogger::fatal(char const*, ...) const");
    EXPECT_EQ(threads[0].frames[3].module, "/usr/local/bin/svgrender");
    EXPECT_EQ(threads[0].frames[4].module, "");
    EXPECT_EQ(threads[1].frames.size(), 2u);
}

TEST(CoredumpTest, IgnoresAMessageWithoutStack)
{
    EXPECT_TRUE(Coredump::parseStackTrace("Process 1 (x) of user 1000 dumped core.").empty());
}

TEST(CoredumpTest, KeepsWhatItCannotDemangle)
{
    EXPECT_EQ(Coredump::demangle("_Zbogus"), "_Zbogus");
    EXPECT_EQ(Coredump::demangle("main"), "main");
}

TEST(CoredumpTest, ReadsTheApplicationFromItsScope)
{
    Coredump dump;
    dump.userUnit = "app-lingmo\\x2dsystem\\x2dmonitor-2127671.scope";
    EXPECT_EQ(dump.applicationId(), "lingmo-system-monitor");
    EXPECT_EQ(dump.applicationPid(), 2127671u);

    dump.userUnit = "app-lingmo-org.gnome.\\xe6\\x96\\x87-12.scope";
    EXPECT_EQ(dump.applicationId(), "org.gnome.\xe6\x96\x87");

    dump.userUnit = "session-662.scope";
    EXPECT_EQ(dump.applicationId(), "");
    EXPECT_EQ(dump.applicationPid(), 0u);
}

TEST(CoredumpTest, WritesAReport)
{
    Coredump dump;
    dump.signal = SIGSEGV;
    dump.signalName = "SIGSEGV";
    dump.executable = "/usr/bin/x";
    dump.threads = Coredump::parseStackTrace(s_message);
    const std::string report = dump.report("X");
    EXPECT_EQ(report.rfind("X quit unexpectedly\nExecutable: /usr/bin/x\n", 0), 0u);
    EXPECT_NE(report.find("(SIGSEGV)"), std::string::npos);
    EXPECT_NE(report.find("\nThread 2112053:\n#0 ?? (libc.so.6)\n#1 raise (libc.so.6)\n"), std::string::npos);
}

TEST(CoredumpTest, WritesTheLocalTime)
{
    Coredump dump;
    dump.time = 1760000000000;
    const std::string report = dump.report("X");
    const size_t time = report.find("Time: ");
    ASSERT_NE(time, std::string::npos);
    // 2025-10-09T08:53:20 in UTC; the zone is the machine's, and not written.
    const std::string value = report.substr(time + 6, report.find('\n', time) - time - 6);
    EXPECT_EQ(value.size(), 19u) << value;
    EXPECT_EQ(value[10], 'T');
}
