// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DFile>

#include <gtest/gtest.h>
#include <stubext.h>

#include "dfile_p.h"
#include "dfilehelper.h"

DIO_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDFile : public testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        m_stub.clear();
    }

    stub_ext::StubExt m_stub;
};

TEST_F(TestDFile, url)
{
    QUrl testUrl { QUrl::fromLocalFile("/tmp") };
    DFile file { testUrl };
    EXPECT_EQ(testUrl, file.url());
}

TEST_F(TestDFile, open)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));
    // unexpected
    {
        file.d->isOpen = true;
        auto ret = file.open(OpenFlag::ReadOnly);
        EXPECT_FALSE(ret.hasValue());

        file.d->isOpen = false;
        m_stub.set_lamda(ADDR(DFilePrivate, checkOpenFlags), [](DFilePrivate *, OpenFlags *) {
            __DBG_STUB_INVOKE__
            return false;
        });
        ret = file.open(OpenFlag::ReadOnly);
        EXPECT_FALSE(ret.hasValue());
    }

    // TODO: expected
}

TEST_F(TestDFile, close)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));
    file.d->iStream = nullptr;
    file.d->oStream = nullptr;
    file.d->ioStream = nullptr;
    EXPECT_TRUE(file.close());
}
TEST_F(TestDFile, isOpen)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));
    file.d->isOpen = true;
    auto ret { file.isOpen() };
    EXPECT_TRUE(ret);

    file.d->isOpen = false;
    ret = file.isOpen();
    EXPECT_FALSE(ret);
}

TEST_F(TestDFile, read)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    QByteArray data;
    // unexpected: input error
    {
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return nullptr;
        });
        auto ret { file.read(&data, 2048) };
        EXPECT_FALSE(ret.hasValue());
    }

    // unexpected: runtime error
    {
        static GInputStream stream;
        static GError err;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });

        m_stub.set_lamda(g_input_stream_read, [](GInputStream *, void *, gsize, GCancellable *, GError **error) {
            __DBG_STUB_INVOKE__
            *error = &err;
            return 0;
        });

        auto ret { file.read(&data, 2048) };
        EXPECT_FALSE(ret.hasValue());
    }

    // expected
    {
        static GInputStream stream;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_input_stream_read, [](GInputStream *, void *, gsize maxSize, GCancellable *, GError **) {
            __DBG_STUB_INVOKE__
            return maxSize;
        });
        auto ret { file.read(&data, 2048) };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_EQ(ret, 2048);
    }
}

TEST_F(TestDFile, readAll)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // unpexpected: input error
    {
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return nullptr;
        });
        auto ret { file.readAll() };
        EXPECT_FALSE(ret.hasValue());
    }

    // unpexpected: runtime error
    {
        static GInputStream stream;
        static GError err;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_input_stream_read_all, [](GInputStream *, void *, gsize, gsize *, GCancellable *, GError **error) {
            __DBG_STUB_INVOKE__
            *error = &err;
            return FALSE;
        });
        auto ret { file.readAll() };
        EXPECT_FALSE(ret.hasValue());
    }

    // expected
    {
        static GInputStream stream;
        static size_t curSize { 0 };
        constexpr size_t kFileSize = 8192 * 3;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_input_stream_read_all, [](GInputStream *, void *, gsize, gsize *bytesRead, GCancellable *, GError **) {
            __DBG_STUB_INVOKE__
            if (curSize >= kFileSize) {
                *bytesRead = 0;
                return TRUE;
            }
            *bytesRead = 8192;
            curSize += *bytesRead;
            return TRUE;
        });
        auto ret { file.readAll() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_EQ(ret.value().size(), 8192 * 3);
    }
}

TEST_F(TestDFile, writeWithLen)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // unexpected: input error
    {
        m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
            __DBG_STUB_INVOKE__
            return nullptr;
        });

        auto ret { file.write(QByteArray {}, 10) };
        EXPECT_FALSE(ret.hasValue());
    }

    // unexpected: runtime error
    {
        static GOutputStream stream;
        static GError err;
        m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_output_stream_write, [](GOutputStream *, const void *, gsize, GCancellable *, GError **error) {
            __DBG_STUB_INVOKE__
            *error = &err;
            return 0;
        });
        auto ret { file.write(QByteArray {}, 10) };
        EXPECT_FALSE(ret.hasValue());
    }

    // expected
    {
        static GOutputStream stream;
        m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_output_stream_write, [](GOutputStream *, const void *, gsize count, GCancellable *, GError **) {
            __DBG_STUB_INVOKE__
            return count;
        });
        auto ret { file.write(QByteArray {}, 10) };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_EQ(ret.value(), 10);
    }
}

TEST_F(TestDFile, write)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));
    QByteArray data { 1024, 'a' };
    static GOutputStream stream;

    m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
        __DBG_STUB_INVOKE__
        return &stream;
    });
    m_stub.set_lamda(g_output_stream_write, [](GOutputStream *, const void *, gsize count, GCancellable *, GError **) {
        __DBG_STUB_INVOKE__
        return count;
    });

    auto ret { file.write(data) };
    EXPECT_TRUE(ret.hasValue());
    EXPECT_EQ(ret.value(), data.size());
}

TEST_F(TestDFile, seek)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // unexpected: input error
    {
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return nullptr;
        });
        auto ret { file.seek(10, SeekType::Begin) };
        EXPECT_FALSE(ret.hasValue());
    }

    // unexpected: runtime error
    {
        static GInputStream stream;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(ADDR(DFilePrivate, checkSeekable), []() {
            __DBG_STUB_INVOKE__
            return false;
        });
        auto ret { file.seek(10, SeekType::Begin) };
        EXPECT_FALSE(ret.hasValue());

        m_stub.set_lamda(ADDR(DFilePrivate, checkSeekable), []() {
            __DBG_STUB_INVOKE__
            return true;
        });

        static GError err;
        m_stub.set_lamda(g_seekable_seek, [](GSeekable *, goffset, GSeekType, GCancellable *, GError **error) {
            __DBG_STUB_INVOKE__
            *error = &err;
            return FALSE;
        });
        ret = file.seek(10, SeekType::Begin);
        EXPECT_FALSE(ret.hasValue());
    }

    // expected
    {
        static GInputStream stream;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_seekable_seek, [](GSeekable *, goffset, GSeekType, GCancellable *, GError **) {
            __DBG_STUB_INVOKE__
            return TRUE;
        });
        auto ret { file.seek(10, SeekType::Begin) };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFile, pos)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // unexpected
    {
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return nullptr;
        });
        auto ret { file.pos() };
        EXPECT_FALSE(ret.hasValue());

        m_stub.set_lamda(ADDR(DFilePrivate, checkSeekable), []() {
            __DBG_STUB_INVOKE__
            return true;
        });
        ret = file.pos();
        EXPECT_FALSE(ret.hasValue());
    }

    // expected
    {
        static GInputStream stream;
        m_stub.set_lamda(ADDR(DFilePrivate, inputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });

        m_stub.set_lamda(g_seekable_tell, [](GSeekable *seekable) {
            __DBG_STUB_INVOKE__
            return TRUE;
        });
    }
}

TEST_F(TestDFile, flush)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // unexpected
    {
        m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
            __DBG_STUB_INVOKE__
            return nullptr;
        });
        auto ret { file.flush() };
        EXPECT_FALSE(ret.hasValue());
    }

    // unexpected: runtime
    {
        static GOutputStream stream;
        static GError err;
        m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_output_stream_flush, [](GOutputStream *, GCancellable *, GError **error) {
            __DBG_STUB_INVOKE__
            *error = &err;
            return FALSE;
        });
        auto ret { file.flush() };
        EXPECT_FALSE(ret.hasValue());
    }

    // expected
    {
        static GOutputStream stream;
        m_stub.set_lamda(ADDR(DFilePrivate, outputStream), []() {
            __DBG_STUB_INVOKE__
            return &stream;
        });
        m_stub.set_lamda(g_output_stream_flush, [](GOutputStream *, GCancellable *, GError **) {
            __DBG_STUB_INVOKE__
            return TRUE;
        });
        auto ret { file.flush() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDFile, size)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));
    // unexpected
    {
        static GError err;
        m_stub.set_lamda(g_file_query_info, [](GFile *, const char *, GFileQueryInfoFlags, GCancellable *, GError **error) {
            *error = &err;
            return nullptr;
        });
        auto ret { file.size() };
        EXPECT_FALSE(ret.hasValue());
    }

    // TODO: expected
}

TEST_F(TestDFile, exists)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    m_stub.set_lamda(ADDR(DFilePrivate, exists), []() {
        return true;
    });
    EXPECT_TRUE(file.exists());
}

TEST_F(TestDFile, permissions)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // unexpected
    {
        static GError err;
        m_stub.set_lamda(g_file_query_info, [](GFile *, const char *, GFileQueryInfoFlags, GCancellable *, GError **error) {
            *error = &err;
            return nullptr;
        });
        auto ret { file.permissions() };
        EXPECT_FALSE(ret.hasValue());
    }

    // TODO: expected
}

TEST_F(TestDFile, setPermissions)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    Permissions perms;
    perms.setFlag(Permission::ReadOwner);
    perms.setFlag(Permission::WriteOwner);
    perms.setFlag(Permission::ReadGroup);

    // unexpected
    {
        static GError err;
        m_stub.set_lamda(ADDR(DFileHelper, setAttribute),
                         [](GFile *, const char *, AttributeType, const QVariant &, GFileQueryInfoFlags, GCancellable *, GError **error) {
                             *error = &err;
                             return false;
                         });
        auto ret { file.setPermissions(perms) };
        EXPECT_FALSE(ret.hasValue());
    }

    // TODO: expected
}

TEST_F(TestDFile, setAttributeByID)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // TODO: imple me!
}

TEST_F(TestDFile, setAttribute)
{
    DFile file(QUrl::fromLocalFile("/a/b/c"));

    // TODO: imple me!
}
