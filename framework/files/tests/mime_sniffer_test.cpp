// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <sys/wait.h>
#include <tuple>

#include "base/paths.h"
#include "lingmo/files/mime_sniffer_p.h"
#include "testing.h"

using namespace Lingmo;
using namespace Lingmo::Files;
using namespace Lingmo::Files::testing;

static const char* const UserPackage =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n"
    "  <mime-type type=\"text/x-matlab\"><glob-deleteall/><glob pattern=\"*.mlab\"/></mime-type>\n"
    "  <mime-type type=\"text/x-makefile\"><glob-deleteall/></mime-type>\n"
    "  <mime-type type=\"text/troff\"><glob-deleteall/></mime-type>\n"
    "  <mime-type type=\"video/mp2t\"><glob-deleteall/><glob pattern=\"*.m2ts\"/></mime-type>\n"
    "  <mime-type type=\"application/x-lingmo-test\">\n"
    "    <glob pattern=\"*.ts\"/>\n"
    "    <magic priority=\"90\"><match type=\"string\" value=\"CUTEFISH\" offset=\"0:64\"/></magic>\n"
    "  </mime-type>\n"
    "</mime-info>\n";

// A user MIME directory, before the system one, that deletes system globs from
// the literal list, the suffix tree and the complex globs, and adds its own.
// The expected types were recorded from a reference implementation.
TEST(MimeSnifferTest, LocalDirectoryDeletesSystemGlobs)
{
    const std::string update = base::paths::findExecutable("update-mime-database");
    if (update.empty())
        GTEST_SKIP() << "update-mime-database is not installed";

    TemporaryDirectory data;
    ASSERT_TRUE(makePath(data.filePath("mime/packages")));
    writeFile(data.filePath("mime/packages/test.xml"), UserPackage);

    const pid_t pid = ::fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        const int null = ::open("/dev/null", O_WRONLY);
        ::dup2(null, 1);
        ::dup2(null, 2);
        const std::string directory = data.filePath("mime");
        ::execl(update.c_str(), update.c_str(), directory.c_str(), nullptr);
        ::_exit(127);
    }
    int status = 0;
    ASSERT_EQ(::waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);

    internal::setUserDataDirectory(data.path());

    TemporaryDirectory dir;
    const std::vector<std::tuple<std::string, std::string, std::string>> files = {
        {"Makefile", "all:\n", "text/plain"},
        {"GNUmakefile", "all:\n", "text/plain"},
        {"function.m", "function y = f(x)\nend\n", "text/x-objcsrc"},
        {"script.mlab", "x = 1\n", "text/x-matlab"},
        {"page.1", "plain words\n", "application/x-troff-man"},
        {"libsample.so.6", "\x7f" "ELF", "application/x-troff-man"},
        {"video.ts", "xxCUTEFISHxx", "application/x-lingmo-test"},
        {"strings.ts", "<?xml version=\"1.0\"?>\n<TS version=\"2.1\"></TS>\n", "text/vnd.trolltech.linguist"},
        {"clip.m2ts", std::string(1, 0x47) + std::string(187, '\0'), "video/mp2t"},
        {"marked", "xxCUTEFISHxx", "application/x-lingmo-test"},
        {"photo.jpg", "\xff\xd8\xff\xe0", "image/jpeg"},
    };

    for (const auto& [name, contents, type] : files) {
        writeFile(dir.filePath(name), contents);
        EXPECT_EQ(name + ' ' + internal::MimeSniffer::self()->mimeTypeForFile(dir.filePath(name)), name + ' ' + type);
    }

    // The deletions took effect: the type no longer comes from these names.
    EXPECT_NE(internal::MimeSniffer::self()->mimeTypeForFile(dir.filePath("Makefile")), "text/x-makefile");
    EXPECT_EQ(internal::MimeSniffer::self()->canonicalName("application/x-lingmo-test"), "application/x-lingmo-test");

    internal::setUserDataDirectory({});
}
