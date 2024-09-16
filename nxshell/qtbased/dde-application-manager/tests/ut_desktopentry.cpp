// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopentry.h"
#include <gtest/gtest.h>
#include <QTextStream>
#include <QSharedPointer>
#include <QLocale>
#include <QDir>
#include <QFile>

class TestDesktopEntry : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        env = qgetenv("XDG_DATA_DIRS");
        auto curDir = QDir::current();
        QByteArray fakeXDG = (curDir.absolutePath() + QDir::separator() + "data").toLocal8Bit();
        ASSERT_TRUE(qputenv("XDG_DATA_DIRS", fakeXDG));
        ParserError err;
        auto file = DesktopFile::searchDesktopFileById("deepin-editor", err);
        if (!file.has_value()) {
            qWarning() << "search failed:" << err;
            return;
        }
        m_file.reset(new DesktopFile{std::move(file).value()});
    }

    static void TearDownTestCase() { qputenv("XDG_DATA_DIRS", env); }

    QSharedPointer<DesktopFile> file() { return m_file; }

private:
    static inline QSharedPointer<DesktopFile> m_file;
    static inline QByteArray env;
};

TEST_F(TestDesktopEntry, desktopFile)
{
    const auto &fileptr = file();
    ASSERT_FALSE(fileptr.isNull());
    const auto &exampleFile = file();
    auto curDir = QDir::current();
    QString path{curDir.absolutePath() + QDir::separator() + "data" + QDir::separator() + "applications" + QDir::separator() +
                 "deepin-editor.desktop"};
    EXPECT_EQ(exampleFile->sourcePath(), path);
    EXPECT_EQ(exampleFile->desktopId().toStdString(), "deepin-editor");
}

TEST_F(TestDesktopEntry, prase)
{
    const auto &exampleFile = file();
    ASSERT_FALSE(exampleFile.isNull());
    DesktopEntry entry;
    QFile in{exampleFile->sourcePath()};
    ASSERT_TRUE(in.open(QFile::ExistingOnly | QFile::ReadOnly | QFile::Text));
    QTextStream fs{&in};
    auto err = entry.parse(fs);
    ASSERT_EQ(err, ParserError::NoError);

    auto group = entry.group("Desktop Entry");
    ASSERT_TRUE(group);

    auto name = group->constFind("Name");
    ASSERT_NE(name, group->cend());

    const auto &nameVal = *name;

    bool ok{true};
    toBoolean(nameVal, ok);
    EXPECT_FALSE(ok);

    toNumeric(nameVal, ok);
    EXPECT_FALSE(ok);

    EXPECT_TRUE(nameVal.canConvert<QStringMap>());
    auto defaultName = toString(nameVal);  // get default locale value.
    EXPECT_TRUE(defaultName == "Text Editor");

    const auto &localeMap = nameVal.value<QStringMap>();
    auto localeString = toLocaleString(nameVal.value<QStringMap>(), QLocale{"zh_CN"});
    EXPECT_TRUE(localeString == "文本编辑器");
}
