// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <filesystem>
#include <unistd.h>

#include "base/file.h"
#include "base/paths.h"

using namespace Lingmo::Base;
using paths::UserDirectory;

// The expectations are the reference results for the same file.
TEST(UserDirectoryTest, ReadsUserDirsFile)
{
    const std::string file = std::filesystem::temp_directory_path() / ("user-dirs-" + std::to_string(getpid()));
    ASSERT_TRUE(file::writeAtomically(file, "XDG_DESKTOP_DIR=\"$HOME\"\n"
                                            "XDG_DOCUMENTS_DIR=\"/srv/docs/\"\n"
                                            "XDG_DOWNLOAD_DIR=$HOME/Dl\n"
                                            "XDG_MUSIC_DIR=\"\"\n"
                                            "XDG_PICTURES_DIR=\"$HOME/Pics\"\r\n"
                                            "XDG_PICTURES_DIR=\"$HOME/Later\"\n"
                                            "XDG_VIDEOS_DIR=\n"
                                            "XDG_TEMPLATES_DIR=\"$HOMEX\"\n"
                                            "  XDG_PUBLICSHARE_DIR=\"/x\"\n"));
    const std::string home = paths::home();
    EXPECT_EQ(paths::userDirectory(UserDirectory::Desktop, file), home);
    EXPECT_EQ(paths::userDirectory(UserDirectory::Documents, file), "/srv/docs");
    EXPECT_EQ(paths::userDirectory(UserDirectory::Downloads, file), home + "/Dl");
    // Two quotes alone are not stripped.
    EXPECT_EQ(paths::userDirectory(UserDirectory::Music, file), "\"\"");
    // The last line counts.
    EXPECT_EQ(paths::userDirectory(UserDirectory::Pictures, file), home + "/Later");
    EXPECT_EQ(paths::userDirectory(UserDirectory::Videos, file), "");
    EXPECT_EQ(paths::userDirectory(UserDirectory::Templates, file), home + "X");
    // A line that does not start with XDG_ is not read.
    EXPECT_EQ(paths::userDirectory(UserDirectory::PublicShare, file), home + "/Public");
    std::filesystem::remove(file);

    EXPECT_EQ(paths::userDirectory(UserDirectory::Pictures, "/nonexistent/user-dirs.dirs"), home + "/Pictures");
    EXPECT_EQ(paths::userDirectory(UserDirectory::Videos, "/nonexistent/user-dirs.dirs"), home + "/Videos");
}
