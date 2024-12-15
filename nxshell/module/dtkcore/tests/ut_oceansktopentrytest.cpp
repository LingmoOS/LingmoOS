// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>
#include <QString>
#include <QTemporaryFile>
#include <DDesktopEntry>
#include <gtest/gtest.h>
#include <QLocale>

DCORE_USE_NAMESPACE

const QString testFileContent = { QStringLiteral(R"desktop(# A. Example Desktop Entry File
[Desktop Entry]
Version=1.0
Type=Application
Name=Foo Viewer
Name[zh_CN]=福查看器
Comment=The best viewer for Foo objects available!
# Next line have an extra " character
Comment[zh_CN]=最棒的 "福 查看器！
TryExec=fooview
Exec=fooview %F
Icon=fooview
MimeType=image/x-foo;
Actions=Gallery;Create;

[Desktop Action Gallery]
Exec=fooview --gallery
Name=Browse Gallery

[Desktop Action Create]
Exec=fooview --create-new
Name=Create a new Foo!
Icon=fooview-new
)desktop") };

class ut_DesktopEntry : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        //qDebug() << "*****************" << __FUNCTION__;
    }
    static void TearDownTestCase()
    {
        //qDebug() << "*****************" << __FUNCTION__;
    }
    virtual void SetUp();
    virtual void TearDown();
};
void ut_DesktopEntry::SetUp()
{

}
void ut_DesktopEntry::TearDown()
{

}

TEST_F(ut_DesktopEntry, ParseFile)
{
    QTemporaryFile file("testReadXXXXXX.desktop");
    ASSERT_TRUE(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << testFileContent;
    file.close();
    ASSERT_TRUE(QFile::exists(fileName));

    QScopedPointer<DDesktopEntry> desktopFile(new DDesktopEntry(fileName));
    QStringList allGroups = desktopFile->allGroups();
    ASSERT_EQ(allGroups.count(), 3);
    ASSERT_TRUE(allGroups.contains("Desktop Entry") &&
            allGroups.contains("Desktop Action Gallery") &&
            allGroups.contains("Desktop Action Create"));
    ASSERT_EQ(desktopFile->allGroups(true)[0], QStringLiteral("Desktop Entry"));
    ASSERT_EQ(desktopFile->localizedValue("Name", "zh_CN"), QStringLiteral("福查看器"));
    ASSERT_EQ(desktopFile->localizedValue("Name", "empty"), QStringLiteral("Foo Viewer"));
    ASSERT_EQ(desktopFile->keys("Desktop Entry"),
             QStringList({"Actions", "Comment", "Comment[zh_CN]", "Exec", "Icon", "MimeType", "Name", "Name[zh_CN]", "TryExec", "Type", "Version"}));

    {
        struct RestoreLocale {
            ~RestoreLocale() { QLocale::setDefault(QLocale::system()); }
        } restoreLocale;
        Q_UNUSED(restoreLocale);

        QLocale::setDefault(QLocale("zh_CN"));
        ASSERT_EQ(desktopFile->localizedValue("Name"), QStringLiteral("福查看器"));

        QLocale::setDefault(QLocale::c());
        ASSERT_EQ(desktopFile->localizedValue("Name"), QStringLiteral("Foo Viewer"));
    }

    ASSERT_EQ(desktopFile->stringValue("Name"), QStringLiteral("Foo Viewer"));
    ASSERT_EQ(desktopFile->setRawValue("Bar Viewer", "Name"), true);
    ASSERT_EQ(desktopFile->stringValue("Name"), QStringLiteral("Bar Viewer"));
    ASSERT_EQ(desktopFile->setLocalizedValue("霸查看器", "zh_CN", "Name"), true);
    ASSERT_EQ(desktopFile->localizedValue("Name", "zh_CN"), QStringLiteral("霸查看器"));
    ASSERT_EQ(desktopFile->contains("Semicolon"), false);
    ASSERT_EQ(desktopFile->setRawValue(";grp\\;2;grp3;", "Semicolon"), true);
    ASSERT_EQ(desktopFile->stringListValue("Semicolon"), QStringList({"", "grp;2", "grp3"}));
    ASSERT_EQ(desktopFile->contains("Semicolon"), true);
    ASSERT_EQ(desktopFile->removeEntry("Semicolon"), true);
    ASSERT_EQ(desktopFile->contains("Semicolon"), false);

    //qDebug() << desktopFile->save();
    //qDebug() << fileName;
}

TEST_F(ut_DesktopEntry, escape)
{
    QString slash("\\\\");
    ASSERT_TRUE(DDesktopEntry::escapeExec(slash) == slash);
    ASSERT_TRUE(DDesktopEntry::unescapeExec(slash) == slash);
}
