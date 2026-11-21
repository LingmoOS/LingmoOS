// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <filesystem>

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>

#include "base/file.h"
#include "base/paths.h"
#include "testutil.h"
#include "userdirectories.h"

using Lingmo::Base::file::TemporaryDirectory;
namespace fs = std::filesystem;

static QStringList directoryNames(const QString &path)
{
    QStringList names;
    for (const auto &item : fs::directory_iterator(path.toStdString())) {
        if (item.is_directory())
            names << QString::fromStdString(item.path().filename().string());
    }
    std::sort(names.begin(), names.end());
    return names;
}

TEST(UserDirectories, firstLogin)
{
    TemporaryDirectory home(SessionTest::temporaryParent().toStdString());
    const QString homePath = QString::fromStdString(home.path());
    const QString config   = homePath + QStringLiteral("/.config");

    Lingmo::UserDirectories::ensure(homePath, config);

    const QStringList expected = {
        QStringLiteral(".config"),  QStringLiteral("Desktop"),
        QStringLiteral("Documents"), QStringLiteral("Downloads"),
        QStringLiteral("Music"),    QStringLiteral("Pictures"),
        QStringLiteral("Public"),   QStringLiteral("Templates"),
        QStringLiteral("Videos"),
    };
    EXPECT_EQ(directoryNames(homePath), expected);

    EXPECT_EQ(SessionTest::readFile(config + QStringLiteral("/user-dirs.dirs")),
              QStringLiteral(
                  "XDG_DESKTOP_DIR=\"$HOME/Desktop\"\n"
                  "XDG_DOWNLOAD_DIR=\"$HOME/Downloads\"\n"
                  "XDG_TEMPLATES_DIR=\"$HOME/Templates\"\n"
                  "XDG_PUBLICSHARE_DIR=\"$HOME/Public\"\n"
                  "XDG_DOCUMENTS_DIR=\"$HOME/Documents\"\n"
                  "XDG_MUSIC_DIR=\"$HOME/Music\"\n"
                  "XDG_PICTURES_DIR=\"$HOME/Pictures\"\n"
                  "XDG_VIDEOS_DIR=\"$HOME/Videos\"\n"));

    // Verify paths::userDirectory still works with the written file.
    EXPECT_EQ(
        QString::fromStdString(Lingmo::Base::paths::userDirectory(
            Lingmo::Base::paths::UserDirectory::Downloads,
            (config + QStringLiteral("/user-dirs.dirs")).toStdString())),
        QString::fromStdString(Lingmo::Base::paths::home()) + QStringLiteral("/Downloads"));
}

TEST(UserDirectories, existingFileIsKept)
{
    TemporaryDirectory home(SessionTest::temporaryParent().toStdString());
    const QString homePath = QString::fromStdString(home.path());
    const QString config   = homePath + QStringLiteral("/.config");
    const QString contents = QStringLiteral("XDG_DESKTOP_DIR=\"$HOME/Schreibtisch\"\n");

    ASSERT_TRUE(SessionTest::writeFile(config + QStringLiteral("/user-dirs.dirs"), contents));

    Lingmo::UserDirectories::ensure(homePath, config);

    EXPECT_EQ(directoryNames(homePath), QStringList{QStringLiteral(".config")});
    EXPECT_EQ(SessionTest::readFile(config + QStringLiteral("/user-dirs.dirs")), contents);
}
