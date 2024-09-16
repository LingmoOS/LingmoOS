// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DFile>
#include <DFileOperator>
#include <DFileInfo>

#include <gtest/gtest.h>
#include <gio/gio.h>
#include <stubext.h>

#include "dfileinfo_p.h"

DIO_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDFileOperator : public testing::Test
{
public:
    void SetUp() override
    {
        m_operator = new DFileOperator(QUrl("test"));
    }
    void TearDown() override
    {
        delete m_operator;
        m_stub.clear();
    }

    DFileOperator *m_operator;
    stub_ext::StubExt m_stub;
};

TEST_F(TestDFileOperator, url)
{
    auto ret = m_operator->url();
    EXPECT_TRUE(ret.hasValue());
    EXPECT_EQ(ret.value(), QUrl("test"));
}

TEST_F(TestDFileOperator, renameFile)
{
    // unexpected
    {
        static GError err;
        m_stub.set_lamda(g_file_set_display_name, [](GFile *file, const char *display_name, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            *error = &err;
            return nullptr;
        });
        auto ret { m_operator->renameFile("test666") };
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_set_display_name, [](GFile *file, const char *display_name, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            return file;
        });
        auto ret { m_operator->renameFile("test666") };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, copyFile)
{
    // unexpected
    {
        auto ret = m_operator->copyFile(QUrl("/tmp"), CopyFlag::None);
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_copy, [](GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error) {
            __DBG_STUB_INVOKE__
            return true;
        });
        auto ret { m_operator->copyFile(QUrl("/tmp"), CopyFlag::None) };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, moveFile)
{
    // unexpected
    {
        auto ret = m_operator->moveFile(QUrl("/tmp"), CopyFlag::None);
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_move, [](GFile *source, GFile *destination, GFileCopyFlags flags, GCancellable *cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError **error) {
            __DBG_STUB_INVOKE__
            return true;
        });
        auto ret { m_operator->moveFile(QUrl("/tmp"), CopyFlag::None) };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, trashFile)
{
    // unexpected
    {
        auto ret = m_operator->trashFile();
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_trash, [](GFile *file, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            return true;
        });
        auto ret { m_operator->trashFile() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, deleteFile)
{
    // unexpected
    {
        auto ret = m_operator->deleteFile();
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_delete, [](GFile *file, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            return true;
        });
        auto ret { m_operator->deleteFile() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, touchFile)
{
    // unexpected
    {
        auto ret = m_operator->touchFile();
        EXPECT_FALSE(ret);
    }

    // expected
    {
        static GFileOutputStream mock;
        m_stub.set_lamda(g_file_create, [](GFile *file, GFileCreateFlags flags, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            return &mock;
        });
        auto ret { m_operator->touchFile() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, makeDirectory)
{
    // unexpected
    {
        auto ret = m_operator->makeDirectory();
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_make_directory, [](GFile *file, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            return true;
        });
        auto ret { m_operator->makeDirectory() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, createLink)
{
    // unexpected
    {
        auto ret = m_operator->createLink(QUrl("/tmp"));
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(g_file_make_symbolic_link, [](GFile *file, const char *symlink_value, GCancellable *cancellable, GError **error) {
            __DBG_STUB_INVOKE__
            return true;
        });
        auto ret { m_operator->createLink(QUrl("/tmp")) };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFileOperator, restoreFile)
{
    m_stub.set_lamda(ADDR(DFileInfo, setQueryAttributes),
                     [](DFileInfo *, const QByteArray &attributes) {
                         __DBG_STUB_INVOKE__
                     });
    //failed
    {
        auto ret = m_operator->restoreFile();
        EXPECT_TRUE(ret.hasValue());
        EXPECT_FALSE(ret.value());
    }
    //success
    {
        m_stub.set_lamda(ADDR(DFileInfo, initQuerier),
                         []() {
                             __DBG_STUB_INVOKE__
                             return true;
                         });

        auto func = static_cast<DExpected<bool> (DFileInfo::*)(AttributeID) const>(&DFileInfo::hasAttribute);
        m_stub.set_lamda(func, [](DFileInfo *, AttributeID id) {
            __DBG_STUB_INVOKE__
            return true;
        });

        auto func2 = static_cast<DExpected<QVariant> (DFileInfo::*)(AttributeID, bool *) const>(&DFileInfo::attribute);
        m_stub.set_lamda(func2, [](DFileInfo *, AttributeID id, bool *success) {
            __DBG_STUB_INVOKE__
            return QVariant("true");
        });

        m_stub.set_lamda(ADDR(DFileOperator, moveFile),
                         [](DFileOperator *, const QUrl &destUrl, CopyFlag flag) {
                             __DBG_STUB_INVOKE__
                             return true;
                         });
        auto ret = m_operator->restoreFile();
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}
