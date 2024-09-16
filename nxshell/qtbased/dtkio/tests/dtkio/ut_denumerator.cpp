// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DEnumerator>
#include <DFileInfo>

#include <gtest/gtest.h>
#include <stubext.h>

#include "denumerator_p.h"
DIO_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDEnumerator : public testing::Test
{
public:
    void SetUp() override
    {
        m_enumerator = new DEnumerator(testUrl);
    }
    void TearDown() override
    {
        delete m_enumerator;
        m_stub.clear();
    }

    stub_ext::StubExt m_stub;
    QUrl testUrl { "/tmp" };
    DEnumerator *m_enumerator { nullptr };
};

TEST_F(TestDEnumerator, url)
{
    EXPECT_EQ(m_enumerator->url(), testUrl);
}

TEST_F(TestDEnumerator, timeout)
{
    m_enumerator->d->timeout = 55555;
    EXPECT_EQ(m_enumerator->timeout(), 55555);
}

TEST_F(TestDEnumerator, dirfilters)
{
    m_enumerator->d->dirFilters = DirFilter::NoFilter;
    EXPECT_EQ(m_enumerator->dirFilters(), DirFilters(DirFilter::NoFilter));
}

TEST_F(TestDEnumerator, hasnext)
{
    //test false
    {
        m_enumerator->d->enumeratorInited = false;
        m_stub.set_lamda(ADDR(DEnumeratorPrivate, createEnumerator), []() {
            __DBG_STUB_INVOKE__
            return false;
        });
        EXPECT_FALSE(m_enumerator->hasNext().value());
    }
    //test false
    {
        m_enumerator->d->enumeratorInited = true;
        m_stub.set_lamda(ADDR(QStack<GFileEnumerator *>, isEmpty), []() {
            __DBG_STUB_INVOKE__
            return true;
        });
        EXPECT_FALSE(m_enumerator->hasNext().value());
    }
    //test true
    {
        m_enumerator->d->enumeratorInited = false;
        m_stub.set_lamda(ADDR(DEnumeratorPrivate, createEnumerator), []() {
            __DBG_STUB_INVOKE__
            return true;
        });
        m_stub.set_lamda(ADDR(QStack<GFileEnumerator *>, isEmpty), []() {
            __DBG_STUB_INVOKE__
            return false;
        });

        m_enumerator->d->iteratorflags = IteratorFlag::NoIteratorFlags;
        m_stub.set_lamda(g_file_enumerator_iterate, [](GFileEnumerator *direnum, GFileInfo **out_info, GFile **out_child, GCancellable *cancellable, GError **error) {
            Q_UNUSED(direnum);
            Q_UNUSED(cancellable);
            Q_UNUSED(error);
            *out_info = reinterpret_cast<GFileInfo *>(0x12345678);
            *out_child = reinterpret_cast<GFile *>(0x12345675);
            __DBG_STUB_INVOKE__
            return true;
        });

        m_stub.set_lamda(g_file_get_path, [=]() {
            char *ret = (char *)malloc(5 * sizeof(char));
            strcpy(ret, "/tmp");
            return static_cast<char *>(ret);
        });
        m_stub.set_lamda(ADDR(DFileInfo, setQueryAttributes), []() {
            __DBG_STUB_INVOKE__
            return;
        });
        m_stub.set_lamda(ADDR(DFileInfo, setQueryInfoFlags), []() {
            __DBG_STUB_INVOKE__
            return;
        });
        m_stub.set_lamda(ADDR(DFileInfo, initQuerier), []() {
            return true;
        });
        m_stub.set_lamda(ADDR(DEnumeratorPrivate, checkFilter), []() {
            return true;
        });

        EXPECT_TRUE(m_enumerator->hasNext().value());
    }
}

TEST_F(TestDEnumerator, filecount)
{
    m_enumerator->d->enumeratorInited = true;
    m_stub.set_lamda(ADDR(DEnumeratorPrivate, createEnumerator), []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    m_stub.set_lamda(ADDR(DEnumerator, hasNext), []() {
        __DBG_STUB_INVOKE__
        return false;
    });
    EXPECT_EQ(m_enumerator->fileCount(), 0);
}

TEST_F(TestDEnumerator, privateCheckFileter)
{
    //readble
    {
        m_enumerator->d->fileInfo = reinterpret_cast<DFileInfo *>(0x1234);
        m_enumerator->d->dirFilters = DirFilter::Readable;
        auto func = static_cast<DExpected<QVariant> (DFileInfo::*)(AttributeID, bool *) const>(&DFileInfo::attribute);
        m_stub.set_lamda(func, []() {
            __DBG_STUB_INVOKE__
            return QVariant("true");
        });
        EXPECT_TRUE(m_enumerator->d->checkFilter());
        m_enumerator->d->fileInfo = nullptr;
    }
    //writeble
    {
        m_enumerator->d->fileInfo = reinterpret_cast<DFileInfo *>(0x1234);
        m_enumerator->d->dirFilters = DirFilter::Writable;
        auto func = static_cast<DExpected<QVariant> (DFileInfo::*)(AttributeID, bool *) const>(&DFileInfo::attribute);
        m_stub.set_lamda(func, []() {
            __DBG_STUB_INVOKE__
            return QVariant("true");
        });
        EXPECT_TRUE(m_enumerator->d->checkFilter());
        m_enumerator->d->fileInfo = nullptr;
    }
    //executable
    {
        m_enumerator->d->fileInfo = reinterpret_cast<DFileInfo *>(0x1234);
        m_enumerator->d->dirFilters = DirFilter::Executable;
        auto func = static_cast<DExpected<QVariant> (DFileInfo::*)(AttributeID, bool *) const>(&DFileInfo::attribute);
        m_stub.set_lamda(func, []() {
            __DBG_STUB_INVOKE__
            return QVariant("true");
        });
        EXPECT_TRUE(m_enumerator->d->checkFilter());
        m_enumerator->d->fileInfo = nullptr;
    }
    //false
    {
        m_enumerator->d->fileInfo = reinterpret_cast<DFileInfo *>(0x1234);
        m_enumerator->d->dirFilters = DirFilter::Dirs;
        auto func = static_cast<DExpected<QVariant> (DFileInfo::*)(AttributeID, bool *) const>(&DFileInfo::attribute);
        m_stub.set_lamda(func, []() {
            __DBG_STUB_INVOKE__
            return QVariant("false");
        });
        EXPECT_FALSE(m_enumerator->d->checkFilter());
        m_enumerator->d->fileInfo = nullptr;
    }
}
