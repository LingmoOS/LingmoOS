// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef ENABLE_DEEPINANYTHING

#include "global/grandsearch_global.h"
#include "searcher/file/filenameworker.h"
#include "searcher/file/filenameworker_p.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"
#include "configuration/configer_p.h"
#include "utils/specialtools.h"
#include "searcher/file/filesearchutils.h"

#include "interfaces/anything_interface.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QFile>
#include <QDBusPendingReply>
#include <QDir>

#define worker_ptr worker->d_ptr

GRANDSEARCH_USE_NAMESPACE

class FileNameWorkerTest : public testing::Test
{
public:
    FileNameWorkerTest() : testing::Test()
    {
        stub_ext::StubExt st;
        auto fs = ConfigerPrivate::fileSearcher();
        st.set_lamda(&Configer::group, [fs](){
            return fs;
        });
        worker.reset(new FileNameWorker(GRANDSEARCH_CLASS_FILE_DEEPIN, false));
    }

    QSharedPointer<FileNameWorker> worker = nullptr;
};

TEST_F(FileNameWorkerTest, ut_setContext)
{
    worker->setContext("");
    EXPECT_TRUE(worker_ptr->m_searchInfo.keyword.isEmpty());

    worker->setContext("test");
    EXPECT_TRUE(worker_ptr->m_searchInfo.keyword == "test");
}

TEST_F(FileNameWorkerTest, ut_isAsync)
{
    EXPECT_FALSE(worker->isAsync());
}

TEST_F(FileNameWorkerTest, ut_working_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&ComDeepinAnythingInterface::isValid, [](){ return false; });
    EXPECT_FALSE(worker->working(nullptr));

    st.clear();
    bool (QAtomicInt::*testAndSetRelease_addr)(int, int) = &QAtomicInt::testAndSetRelease;
    st.set_lamda(testAndSetRelease_addr, [](){ return false; });
    EXPECT_FALSE(worker->working(nullptr));
}

TEST_F(FileNameWorkerTest, ut_working_1)
{
    worker->setContext("test");

    stub_ext::StubExt st;
    st.set_lamda(&ComDeepinAnythingInterface::isValid, [](){ return true; });
    st.set_lamda(&ComDeepinAnythingInterface::hasLFT, [](){
        return QDBusPendingReply<bool>();
    });
    bool (QFile::*exists_addr)() const = &QFile::exists;
    st.set_lamda(exists_addr, [](){ return false; });
    st.set_lamda(&FileNameWorkerPrivate::searchUserPath, [](){ return false; });

    EXPECT_FALSE(worker->working(nullptr));
}

TEST_F(FileNameWorkerTest, ut_working_2)
{
    worker->setContext("test");

    stub_ext::StubExt st;
    st.set_lamda(&ComDeepinAnythingInterface::isValid, [](){ return true; });
    st.set_lamda(&ComDeepinAnythingInterface::hasLFT, [](){
        return QDBusPendingReply<bool>();
    });
    bool (QFile::*exists_addr)() const = &QFile::exists;
    st.set_lamda(exists_addr, [](){ return true; });
    st.set_lamda(&FileNameWorkerPrivate::searchUserPath, [](){ return true; });
    st.set_lamda(VADDR(FileNameWorker, hasItem), [](){ return true; });

    EXPECT_TRUE(worker->working(nullptr));
}

TEST_F(FileNameWorkerTest, ut_terminate)
{
    worker->terminate();
    EXPECT_EQ(worker_ptr->m_status.loadAcquire(), ProxyWorker::Terminated);
}

TEST_F(FileNameWorkerTest, ut_status)
{
    auto status = worker->status();
    EXPECT_EQ(status, ProxyWorker::Ready);
}

TEST_F(FileNameWorkerTest, ut_hasItem_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&GrandSearch::MatchedItems::isEmpty, [](){ return false; });
    EXPECT_TRUE(worker->hasItem());
}

TEST_F(FileNameWorkerTest, ut_hasItem_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&GrandSearch::MatchedItems::isEmpty, [](){ return true; });
    EXPECT_FALSE(worker->hasItem());
}

TEST_F(FileNameWorkerTest, ut_takeAll)
{
    GrandSearch::MatchedItem item;
    worker_ptr->m_items[FileSearchUtils::Folder].append(item);
    auto result = worker->takeAll();

    EXPECT_FALSE(result.isEmpty());
}

//FileNameWorkerPrivate
TEST_F(FileNameWorkerTest, ut_traverseDirAndFile)
{
    stub_ext::StubExt st;
    bool (QDir::*exists_addr)() const = &QDir::exists;
    st.set_lamda(exists_addr, [](){ return false; });
    QFileInfoList list = worker_ptr->traverseDirAndFile("/home");
    EXPECT_TRUE(list.isEmpty());

    st.clear();
    QStringList homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    list = worker_ptr->traverseDirAndFile(homePaths.first());
    EXPECT_TRUE(!list.isEmpty());
}

TEST_F(FileNameWorkerTest, ut_appendSearchResult_0)
{
    stub_ext::StubExt st;
    bool (QSet<QString>::*contains_addr)(const QString &) const = &QSet<QString>::contains;
    st.set_lamda(contains_addr, [](){ return true; });

    EXPECT_FALSE(worker_ptr->appendSearchResult("test"));
}

TEST_F(FileNameWorkerTest, ut_appendSearchResult_1)
{
    stub_ext::StubExt st;
    st.set_lamda(FileSearchUtils::getGroupByName, [](){ return FileSearchUtils::Folder; });
    st.set_lamda(&QHash<FileSearchUtils::Group, quint32>::contains, []() { return false; });

    EXPECT_FALSE(worker_ptr->appendSearchResult("test"));
}

TEST_F(FileNameWorkerTest, ut_appendSearchResult_2)
{
    stub_ext::StubExt st;
    st.set_lamda(FileSearchUtils::getGroupByName, [](){ return FileSearchUtils::File; });
    st.set_lamda(&QHash<FileSearchUtils::Group, quint32>::contains, []() { return false; });

    EXPECT_FALSE(worker_ptr->appendSearchResult("test"));
}

TEST_F(FileNameWorkerTest, ut_appendSearchResult_3)
{
    stub_ext::StubExt st;
    st.set_lamda(FileSearchUtils::getGroupByName, [](){ return FileSearchUtils::File; });
    st.set_lamda(&QHash<FileSearchUtils::Group, quint32>::contains, []() { return true; });
    worker_ptr->m_resultCountHash[FileSearchUtils::File] = 100;

    EXPECT_FALSE(worker_ptr->appendSearchResult("test"));
}

TEST_F(FileNameWorkerTest, ut_searchUserPath_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&FileNameWorkerPrivate::traverseDirAndFile, [](){
        return QFileInfoList{QFileInfo("/home/test")};
    });

    EXPECT_FALSE(worker_ptr->searchUserPath());
}

TEST_F(FileNameWorkerTest, ut_searchUserPath_1)
{
    worker->setContext("test");
    stub_ext::StubExt st;
    st.set_lamda(&FileNameWorkerPrivate::traverseDirAndFile, [](){
        return QFileInfoList{QFileInfo("/home/test")};
    });
    st.set_lamda(&QFileInfo::isDir, [](){ return true; });
    st.set_lamda(&FileNameWorkerPrivate::isResultLimit, [](){ return true; });
    st.set_lamda(&Configer::group, [](){ return ConfigerPrivate::tailerData(); });
    worker_ptr->m_status.storeRelease(ProxyWorker::Runing);

    EXPECT_TRUE(worker_ptr->searchUserPath());
}

TEST_F(FileNameWorkerTest, ut_searchByAnything_0)
{
    worker_ptr->m_searchDirList << "/home";
    EXPECT_FALSE(worker_ptr->searchByAnything());
}

TEST_F(FileNameWorkerTest, ut_searchByAnything_1)
{
    worker_ptr->m_searchDirList << "/home";
    worker_ptr->m_status.storeRelease(ProxyWorker::Runing);

    using SearchType = QDBusPendingReply<QStringList, uint, uint>;
    stub_ext::StubExt st;
    SearchType (ComDeepinAnythingInterface::*search_addr)(int, qlonglong, uint, uint, const QString &, const QString &, bool) = &ComDeepinAnythingInterface::search;
    st.set_lamda(search_addr, [](){ return SearchType(); });

    EXPECT_TRUE(worker_ptr->searchByAnything());
}

TEST_F(FileNameWorkerTest, ut_searchByAnything_2)
{
    worker_ptr->m_searchDirList << "/home";
    worker_ptr->m_status.storeRelease(ProxyWorker::Runing);
    worker_ptr->m_hasTransformed = true;

    using SearchType = QDBusPendingReply<QStringList, uint, uint>;
    stub_ext::StubExt st;
    SearchType (ComDeepinAnythingInterface::*search_addr)(int, qlonglong, uint, uint, const QString &, const QString &, bool) = &ComDeepinAnythingInterface::search;
    st.set_lamda(search_addr, [](){ return SearchType(); });
    st.set_lamda(&QDBusPendingCall::error, [](){ return QDBusError(QDBusError::NoError, ""); });
    QStringList (QStringList::*filter_addr)(const QRegExp &) const = &QStringList::filter;
    st.set_lamda(filter_addr, [](){ return QStringList{"/data/home/test.txt", "/home/test.png", "/home/test.mp4", "/home/test.mp3"}; });
    st.set_lamda(&Configer::group, [](){ return ConfigerPrivate::tailerData(); });

    EXPECT_TRUE(worker_ptr->searchByAnything());
}

TEST_F(FileNameWorkerTest, ut_tryNotify)
{
    stub_ext::StubExt st;
    st.set_lamda(VADDR(FileNameWorker, hasItem), [](){ return true; });
    st.set_lamda(&QTime::elapsed, [](){ return 500; });

    EXPECT_NO_FATAL_FAILURE(worker_ptr->tryNotify());
}

#endif
