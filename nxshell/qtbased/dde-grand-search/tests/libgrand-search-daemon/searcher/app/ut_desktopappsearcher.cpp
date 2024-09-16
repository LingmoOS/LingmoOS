// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/app/desktopappsearcher.h"
#include "searcher/app/desktopappsearcher_p.h"
#include "global/builtinsearch.h"
#include "searcher/app/desktopappworker.h"
#include "utils/chineseletterhelper.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

DCORE_USE_NAMESPACE
GRANDSEARCH_USE_NAMESPACE

TEST(DesktopAppSearcherTest, ut_dir_0)
{
    const char *old = getenv("XDG_DATA_DIRS");
    putenv("XDG_DATA_DIRS=");
    DesktopAppSearcher das;
    auto dir = das.d->m_appDirs;
    EXPECT_TRUE(dir.contains("/usr/share/applications"));
    EXPECT_TRUE(dir.contains("/usr/local/share/applications"));
    EXPECT_TRUE(dir.contains(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).first()));
    setenv("XDG_DATA_DIRS", old ? old : "", 0);
}

TEST(DesktopAppSearcherTest, ut_dir_1)
{
    const char *old = getenv("XDG_DATA_DIRS");
    putenv("XDG_DATA_DIRS=/etc:/etc/apt");
    DesktopAppSearcher das;
    auto dir = das.d->m_appDirs;
    EXPECT_FALSE(dir.contains("/usr/share/applications"));
    EXPECT_FALSE(dir.contains("/usr/local/share/applications"));
    EXPECT_TRUE(dir.contains(QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).first()));
    EXPECT_TRUE(dir.contains("/etc/applications"));
    EXPECT_TRUE(dir.contains("/etc/apt/applications"));
    setenv("XDG_DATA_DIRS", old ? old : "", 0);
}

TEST(DesktopAppSearcherTest, ut_asyncInit_0)
{
    DesktopAppSearcher das;
    das.d->m_inited = true;
    EXPECT_NO_FATAL_FAILURE(das.asyncInit());
}

TEST(DesktopAppSearcherTest, ut_asyncInit_1)
{
    stub_ext::StubExt st;
    st.set_lamda(DesktopAppSearcherPrivate::createIndex, [](){ return; });

    DesktopAppSearcher das;
    EXPECT_NO_FATAL_FAILURE(das.asyncInit());
}

TEST(DesktopAppSearcherTest, ut_name)
{
    DesktopAppSearcher das;
    EXPECT_TRUE(das.name() == GRANDSEARCH_CLASS_APP_DESKTOP);
}

TEST(DesktopAppSearcherTest, ut_isActive)
{
    DesktopAppSearcher das;
    EXPECT_FALSE(das.isActive());
}

TEST(DesktopAppSearcherTest, ut_activate)
{
    DesktopAppSearcher das;
    EXPECT_FALSE(das.activate());
}

TEST(DesktopAppSearcherTest, ut_createWorker)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppWorker::setIndexTable, [](){ return; });

    DesktopAppSearcher das;
    auto worker = das.createWorker();
    EXPECT_TRUE(worker != nullptr);
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(DesktopAppSearcherTest, ut_action)
{
    DesktopAppSearcher das;
    EXPECT_FALSE(das.action("", ""));
}

TEST(DesktopAppSearcherTest, ut_onDirectoryChanged)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppSearcherPrivate::updateIndex, [](){ return; });

    DesktopAppSearcher das;
    EXPECT_NO_FATAL_FAILURE(das.onDirectoryChanged(""));

    st.set_lamda(&QFuture<void>::isRunning, [](){ return true; });
    EXPECT_NO_FATAL_FAILURE(das.onDirectoryChanged(""));
}

//DesktopAppSearcherPrivate
TEST(DesktopAppSearcherTest, ut_createIndex_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppSearcherPrivate::scanDesktopFile, [](){
        QMap<QString, DesktopEntryPointer> entrys;
        QString path("/usr/share/applications/dde-file-manager.desktop");
        DesktopEntryPointer pointer(new DDesktopEntry(path));
        entrys.insert(path, pointer);
        return entrys;
    });

    DesktopAppSearcher das;
    EXPECT_NO_FATAL_FAILURE(das.d->createIndex(das.d));

    das.d->m_creating = true;
    st.set_lamda(&DesktopAppSearcherPrivate::desktopIndex, [](){ return QSet<QString>(); });
    EXPECT_NO_FATAL_FAILURE(das.d->createIndex(das.d));
}

TEST(DesktopAppSearcherTest, ut_createIndex_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppSearcherPrivate::scanDesktopFile, [](){
        QMap<QString, DesktopEntryPointer> entrys;
        QString path("/usr/share/applications/dde-file-manager.desktop");
        DesktopEntryPointer pointer(new DDesktopEntry(path));
        entrys.insert(path, pointer);
        entrys.insert("/test", pointer);
        return entrys;
    });
    st.set_lamda(&DesktopAppSearcherPrivate::desktopIndex, [](){ return QSet<QString>({"test"}); });

    DesktopAppSearcher das;
    das.d->m_creating = true;
    EXPECT_NO_FATAL_FAILURE(das.d->createIndex(das.d));
}

TEST(DesktopAppSearcherTest, ut_updateIndex)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppSearcherPrivate::createIndex, [](){ return; });

    DesktopAppSearcher das;
    das.d->m_needUpdateIndex = true;
    EXPECT_NO_FATAL_FAILURE(das.d->updateIndex(das.d));
}

TEST(DesktopAppSearcherTest, ut_scanDesktopFile)
{
    stub_ext::StubExt st;
    st.set_lamda(&QDir::isReadable, [](){ return false; });

    DesktopAppSearcher das;
    QString appPath = "/usr/share/applications";
    EXPECT_TRUE(das.d->scanDesktopFile({""}, das.d->m_creating).isEmpty());

    st.reset(&QDir::isReadable);
    EXPECT_TRUE(das.d->scanDesktopFile({appPath}, das.d->m_creating).isEmpty());

    das.d->m_creating = true;
    EXPECT_TRUE(!das.d->scanDesktopFile({appPath}, das.d->m_creating).isEmpty());
}

TEST(DesktopAppSearcherTest, ut_desktopIndex)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppSearcherPrivate::desktopName, [](const DesktopEntryPointer &, const QString &locale, bool){
        return locale.isEmpty() ? "File Manager" : "文件管理器";
    });

    st.set_lamda(&GrandSearch::ChineseLetterHelper::convertChinese2Pinyin, [](GrandSearch::ChineseLetterHelper*, const QString &, QString &outFirstPy, QString &outFullPy){
        outFirstPy = "wjglq";
        outFullPy = "wenjianguanliqi";
        return true;
    });

    DesktopAppSearcher das;
    EXPECT_TRUE(das.d->desktopIndex(nullptr, "").isEmpty());

    QString path("/usr/share/applications/dde-file-manager.desktop");
    DesktopEntryPointer pointer(new DDesktopEntry(path));
    QString locale = "zh_CN";
    EXPECT_TRUE(!das.d->desktopIndex(pointer, locale).isEmpty());

    locale = "zh_HK";
    EXPECT_TRUE(!das.d->desktopIndex(pointer, locale).isEmpty());
}

TEST(DesktopAppSearcherTest, ut_desktopName_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&DDesktopEntry::stringValue, [](){ return ""; });

    DesktopAppSearcher das;
    EXPECT_TRUE(das.d->desktopName(nullptr, "", false).isEmpty());

    QString path("/usr/share/applications/dde-file-manager.desktop");
    DesktopEntryPointer pointer(new DDesktopEntry(path));
    EXPECT_TRUE(das.d->desktopName(pointer, "", true).isEmpty());
}

TEST(DesktopAppSearcherTest, ut_desktopName_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&DDesktopEntry::stringValue, [](){ return ""; });
    st.set_lamda(&DesktopAppSearcherPrivate::splitLocaleName, [](){ return "test"; });

    DesktopAppSearcher das;
    QString path("/usr/share/applications/dde-file-manager.desktop");
    DesktopEntryPointer pointer(new DDesktopEntry(path));
    EXPECT_TRUE(das.d->desktopName(pointer, "zh_CN", true).isEmpty());
}

TEST(DesktopAppSearcherTest, ut_splitLocaleName)
{
    DesktopAppSearcher das;
    EXPECT_TRUE(!das.d->splitLocaleName("zh_CN").isEmpty());
}

TEST(DesktopAppSearcherTest, ut_isHidden)
{
    // no display
    DesktopEntryPointer entry(new DTK_CORE_NAMESPACE::DDesktopEntry(""));
    stub_ext::StubExt st;
    QString display;
    QString notShowIn;
    QString onlyShowIn;
    QString hidden;
    st.set_lamda(&DDesktopEntry::stringValue, [&display,&notShowIn,&onlyShowIn,&hidden](DTK_CORE_NAMESPACE::DDesktopEntry *,
                 const QString &key, const QString &section, const QString &defaultValue){
        if (key == "NoDisplay")
            return display;
        else if (key == "NotShowIn")
            return notShowIn;
        else if (key == "OnlyShowIn")
            return onlyShowIn;
        else if (key == "Hidden")
            return hidden;

        return QString("");
    });

    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "no";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "false";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "FALSE";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "False";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "true";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "True";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "TRUE";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden = "TRUE;";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    hidden.clear();

    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "no";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "false";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "FALSE";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "False";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "true";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "True";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "TRUE";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    display = "TRUE;";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    display.clear();

    if (QString(qgetenv("XDG_CURRENT_DESKTOP")) != "Deepin")
        return;

    notShowIn = "uos";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn = ";";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn = "deepin";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn = "deepin;";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn = "Deepin";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn = "Deepin;";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn = "Deepin;uos";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    notShowIn.clear();

    onlyShowIn = ";";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = " ";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = "''";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = "*";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = "deepin";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = "Deepin";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = "Deepin;";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));
    onlyShowIn = "Deepin;uos";
    EXPECT_FALSE(DesktopAppSearcherPrivate::isHidden(entry));

    onlyShowIn = "Deepin";
    notShowIn = "Deepin";
    EXPECT_TRUE(DesktopAppSearcherPrivate::isHidden(entry));
}
