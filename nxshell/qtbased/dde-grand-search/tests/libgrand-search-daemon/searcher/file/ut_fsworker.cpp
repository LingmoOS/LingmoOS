// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef ENABLE_FSEARCH

#include "global/grandsearch_global.h"
#include "searcher/file/fsworker.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"
#include "configuration/configer_p.h"
#include "utils/specialtools.h"
#include "searcher/file/filesearchutils.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

class FsWorkerTest : public testing::Test
{
public:
    FsWorkerTest() : testing::Test()
    {
        stub_ext::StubExt st;
        auto fs = ConfigerPrivate::fileSearcher();
        st.set_lamda(&Configer::group, [fs](){
            return fs;
        });
        worker.reset(new FsWorker(GRANDSEARCH_CLASS_FILE_FSEARCH));
    }

    QSharedPointer<FsWorker> worker = nullptr;
};

TEST_F(FsWorkerTest, ut_setContext)
{
    EXPECT_NO_FATAL_FAILURE(worker->setContext(""));
}

TEST_F(FsWorkerTest, ut_isAsync)
{
    EXPECT_FALSE(worker->isAsync());
}

TEST_F(FsWorkerTest, ut_working_1)
{
    worker->setContext("test");
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));

    stub_ext::StubExt st;
    st.set_lamda(&FsWorker::searchLocalFile, [](){ return false; });
    EXPECT_FALSE(worker->working(nullptr));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

TEST_F(FsWorkerTest, ut_working_2)
{
    worker->setContext("test");
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));

    stub_ext::StubExt st;
    st.set_lamda(&FsWorker::searchLocalFile, [](){ return true; });
    bool (QAtomicInt::*testAndSetRelease_addr)(int, int) = &QAtomicInt::testAndSetRelease;
    st.set_lamda(testAndSetRelease_addr, [](){ return true; });
    st.set_lamda(VADDR(FsWorker, hasItem), [](){ return true; });
    EXPECT_TRUE(worker->working(nullptr));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

TEST_F(FsWorkerTest, ut_terminate)
{
    worker->terminate();
    EXPECT_EQ(worker->m_status.loadAcquire(), ProxyWorker::Terminated);
}

TEST_F(FsWorkerTest, ut_status)
{
    auto status = worker->status();
    EXPECT_EQ(status, ProxyWorker::Ready);
}

TEST_F(FsWorkerTest, ut_hasItem_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&GrandSearch::MatchedItems::isEmpty, [](){ return false; });
    EXPECT_TRUE(worker->hasItem());
}

TEST_F(FsWorkerTest, ut_hasItem_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&GrandSearch::MatchedItems::isEmpty, [](){ return true; });
    EXPECT_FALSE(worker->hasItem());
}

TEST_F(FsWorkerTest, ut_takeAll)
{
    GrandSearch::MatchedItem item;
    worker->m_items[FileSearchUtils::Folder].append(item);
    auto result = worker->takeAll();

    EXPECT_FALSE(result.isEmpty());
}

TEST_F(FsWorkerTest, ut_setFsearchApp)
{
    EXPECT_NO_FATAL_FAILURE(worker->setFsearchApp(nullptr));

    worker->m_status.storeRelease(FsWorker::Runing);
    EXPECT_NO_FATAL_FAILURE(worker->setFsearchApp(nullptr));
}

TEST_F(FsWorkerTest, ut_tryNotify)
{
    stub_ext::StubExt st;
    st.set_lamda(VADDR(FsWorker, hasItem), [](){ return true; });
    st.set_lamda(&QTime::elapsed, [](){ return 500; });

    EXPECT_NO_FATAL_FAILURE(worker->tryNotify());
}

TEST_F(FsWorkerTest, ut_appendSearchResult_0)
{
    stub_ext::StubExt st;
    bool (QSet<QString>::*contains_addr)(const QString &) const = &QSet<QString>::contains;
    st.set_lamda(contains_addr, [](){ return true; });

    EXPECT_FALSE(worker->appendSearchResult("test"));
}

TEST_F(FsWorkerTest, ut_appendSearchResult_1)
{
    stub_ext::StubExt st;
    st.set_lamda(FileSearchUtils::getGroupByName, [](){ return FileSearchUtils::Folder; });
    st.set_lamda(&QHash<FileSearchUtils::Group, quint32>::contains, []() { return false; });

    EXPECT_FALSE(worker->appendSearchResult("test"));
}

TEST_F(FsWorkerTest, ut_appendSearchResult_2)
{
    stub_ext::StubExt st;
    st.set_lamda(FileSearchUtils::getGroupByName, [](){ return FileSearchUtils::File; });
    st.set_lamda(&QHash<FileSearchUtils::Group, quint32>::contains, []() { return false; });

    EXPECT_FALSE(worker->appendSearchResult("test"));
}

TEST_F(FsWorkerTest, ut_appendSearchResult_3)
{
    stub_ext::StubExt st;
    st.set_lamda(FileSearchUtils::getGroupByName, [](){ return FileSearchUtils::File; });
    st.set_lamda(&QHash<FileSearchUtils::Group, quint32>::contains, []() { return true; });
    worker->m_resultCountHash[FileSearchUtils::File] = 100;

    EXPECT_FALSE(worker->appendSearchResult("test"));
}

TEST_F(FsWorkerTest, ut_searchLocalFile_0)
{
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));

    stub_ext::StubExt st;
    st.set_lamda(db_search_results_clear, [](){ return; });
    st.set_lamda(db_try_lock, [](){ return false; });
    EXPECT_TRUE(worker->searchLocalFile());
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

TEST_F(FsWorkerTest, ut_searchLocalFile_1)
{
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));
    worker->m_app->search = static_cast<DatabaseSearch *>(calloc (1, sizeof (DatabaseSearch)));
    worker->m_app->config = static_cast<FsearchConfig *>(calloc(1, sizeof(FsearchConfig)));

    stub_ext::StubExt st;
    st.set_lamda(db_search_results_clear, [](){ return; });
    st.set_lamda(db_try_lock, [](){ return true; });
    st.set_lamda(db_search_update, [](){ return; });
    st.set_lamda(db_get_entries, [](){ return nullptr; });
    st.set_lamda(db_get_num_entries, [](){ return 0; });
    st.set_lamda(db_perform_search, [](){ return; });
    bool (QWaitCondition::*wait_addr)(QMutex *, unsigned long) = &QWaitCondition::wait;
    st.set_lamda(wait_addr, [](){ return true; });
    st.set_lamda(db_unlock, [](){ return; });

    EXPECT_TRUE(worker->searchLocalFile());
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app->search));
    EXPECT_NO_FATAL_FAILURE(config_free(worker->m_app->config));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

TEST_F(FsWorkerTest, ut_callbackReceiveResults_0)
{
    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(calloc (1, sizeof (DatabaseSearchResult)));
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));

    EXPECT_NO_FATAL_FAILURE(worker->callbackReceiveResults(result, worker.get()));
    //free
    if (result->results)
        g_ptr_array_free(result->results, TRUE);
    EXPECT_NO_FATAL_FAILURE(free(result));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

TEST_F(FsWorkerTest, ut_callbackReceiveResults_1)
{
    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(calloc (1, sizeof (DatabaseSearchResult)));
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));
    worker->m_app->search = static_cast<DatabaseSearch *>(calloc (1, sizeof (DatabaseSearch)));
    result->results = g_ptr_array_sized_new (10);
    result->results->len = 10;

    EXPECT_NO_FATAL_FAILURE(worker->callbackReceiveResults(result, worker.get()));
    //free
    if (result->results)
        g_ptr_array_free(result->results, TRUE);
    EXPECT_NO_FATAL_FAILURE(free(result));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app->search));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

TEST_F(FsWorkerTest, ut_callbackReceiveResults_2)
{
    worker->m_status.storeRelease(FsWorker::Runing);
    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(calloc (1, sizeof (DatabaseSearchResult)));
    worker->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));
    worker->m_app->search = static_cast<DatabaseSearch *>(calloc (1, sizeof (DatabaseSearch)));
    result->results = g_ptr_array_sized_new (1);
    g_ptr_array_add (result->results, nullptr);

    stub_ext::StubExt st;
    st.set_lamda(&Configer::group, [](){ return ConfigerPrivate::tailerData(); });

    EXPECT_NO_FATAL_FAILURE(worker->callbackReceiveResults(result, worker.get()));
    //free
    if (result->results)
        g_ptr_array_free(result->results, TRUE);
    EXPECT_NO_FATAL_FAILURE(free(result));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app->search));
    EXPECT_NO_FATAL_FAILURE(free(worker->m_app));
}

#endif
