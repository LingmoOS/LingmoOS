// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Adapted from chromium/src/components/exo/shared_memory_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include <gtest/gtest.h>

#include <unistd.h>

#include <cstring>
#include <limits>
#include <vector>

#include "compositor/buffer.h"
#include "compositor/shm_pool.h"
#include "shm_test_util.h"

using namespace Lingmo;
using namespace Lingmo::Compositor;
using Lingmo::testsupport::makeMemfd;
using Lingmo::testsupport::makePool;

namespace {

constexpr render::PixelFormat kFormat = render::PixelFormat::Bgra8888;

class ShmPoolTest : public testing::Test
{
protected:
    std::shared_ptr<ShmBuffer> create(const base::Size& size, int32_t offset, int32_t stride,
                                      ShmBufferError* error = nullptr)
    {
        ShmBufferError local = ShmBufferError::None;
        auto result = m_pool->createBuffer(size, offset, stride, kFormat,
                                           error ? error : &local);
        return result;
    }

    std::shared_ptr<ShmPool> m_pool = makePool(256 * 256 * 4);
};

TEST_F(ShmPoolTest, CreatePoolRejectsNonPositiveSizes)
{
    EXPECT_EQ(nullptr, ShmPool::create(makeMemfd(4096), 0));
    EXPECT_EQ(nullptr, ShmPool::create(makeMemfd(4096), -1));
}

TEST_F(ShmPoolTest, CreatesAValidBuffer)
{
    ASSERT_TRUE(m_pool);
    ShmBufferError error = ShmBufferError::None;
    std::shared_ptr<ShmBuffer> buffer = create(base::Size(256, 256), 0, 256 * 4, &error);
    ASSERT_TRUE(buffer);
    EXPECT_EQ(ShmBufferError::None, error);
    EXPECT_EQ(base::Size(256, 256), buffer->size());
    EXPECT_EQ(256 * 4, buffer->stride());
    EXPECT_EQ(kFormat, buffer->format());
    EXPECT_EQ(m_pool->data(), buffer->pixels());
}

TEST_F(ShmPoolTest, AcceptsAnOffsetAndAPaddedStride)
{
    std::shared_ptr<ShmBuffer> buffer = create(base::Size(16, 16), 1024, 16 * 4 + 32);
    ASSERT_TRUE(buffer);
    EXPECT_EQ(m_pool->data() + 1024, buffer->pixels());
}

TEST_F(ShmPoolTest, RejectsANegativeOffset)
{
    ShmBufferError error = ShmBufferError::None;
    EXPECT_EQ(nullptr, create(base::Size(16, 16), -4, 16 * 4, &error));
    EXPECT_EQ(ShmBufferError::InvalidStride, error);
}

TEST_F(ShmPoolTest, RejectsInvalidDimensions)
{
    EXPECT_EQ(nullptr, create(base::Size(0, 16), 0, 64));
    EXPECT_EQ(nullptr, create(base::Size(16, 0), 0, 64));
    EXPECT_EQ(nullptr, create(base::Size(-1, 16), 0, 64));
    EXPECT_EQ(nullptr, create(base::Size(16, -1), 0, 64));
}

TEST_F(ShmPoolTest, RejectsAStrideNarrowerThanTheRow)
{
    ShmBufferError error = ShmBufferError::None;
    EXPECT_EQ(nullptr, create(base::Size(16, 16), 0, 16 * 4 - 1, &error));
    EXPECT_EQ(ShmBufferError::InvalidStride, error);
    EXPECT_EQ(nullptr, create(base::Size(16, 16), 0, 0));
    EXPECT_EQ(nullptr, create(base::Size(16, 16), 0, -4));
}

TEST_F(ShmPoolTest, RejectsABufferThatLeavesThePool)
{
    // One pixel past the end of the last row.
    EXPECT_EQ(nullptr, create(base::Size(256, 256), 4, 256 * 4));
    EXPECT_EQ(nullptr, create(base::Size(256, 257), 0, 256 * 4));
}

TEST_F(ShmPoolTest, TheLastRowNeedsOnlyItsOwnPixels)
{
    // A pool that is exactly (height - 1) * stride + width * 4 bytes long is
    // enough; the padding of the final row need not be mapped.
    std::shared_ptr<ShmPool> pool = makePool(3 * 40 + 32);
    ASSERT_TRUE(pool);
    EXPECT_TRUE(pool->createBuffer(base::Size(8, 4), 0, 40, kFormat, nullptr));
    EXPECT_EQ(nullptr, pool->createBuffer(base::Size(9, 4), 0, 40, kFormat, nullptr));
}

TEST_F(ShmPoolTest, RejectsGeometryThatWouldOverflow)
{
    constexpr int32_t kMax = std::numeric_limits<int32_t>::max();
    EXPECT_EQ(nullptr, create(base::Size(kMax, kMax), 0, kMax));
    EXPECT_EQ(nullptr, create(base::Size(kMax / 4 + 1, 2), 0, kMax));
    EXPECT_EQ(nullptr, create(base::Size(16, 16), kMax, 64));
    EXPECT_EQ(nullptr, create(base::Size(1, kMax), 0, 4));
}

TEST_F(ShmPoolTest, GrowsButNeverShrinks)
{
    const int fd = makeMemfd(4096);
    ASSERT_GE(fd, 0);
    const int spare = dup(fd);
    std::shared_ptr<ShmPool> pool = ShmPool::create(fd, 4096);
    ASSERT_TRUE(pool);
    EXPECT_EQ(4096u, pool->size());

    EXPECT_FALSE(pool->createBuffer(base::Size(64, 32), 0, 64 * 4, kFormat, nullptr));
    // The client grows the file before asking the pool to follow.
    ASSERT_EQ(0, ftruncate(spare, 8192));
    close(spare);
    ASSERT_TRUE(pool->resize(8192));
    EXPECT_EQ(8192u, pool->size());
    EXPECT_TRUE(pool->createBuffer(base::Size(64, 32), 0, 64 * 4, kFormat, nullptr));

    EXPECT_FALSE(pool->resize(4096));
    EXPECT_FALSE(pool->resize(0));
    EXPECT_FALSE(pool->resize(-1));
    EXPECT_EQ(8192u, pool->size());
    // Resizing to the current size is a no-op, not a failure.
    EXPECT_TRUE(pool->resize(8192));
}

TEST_F(ShmPoolTest, RejectsAPoolLargerThanItsFile)
{
    EXPECT_EQ(nullptr, ShmPool::create(makeMemfd(4096), 8192));
    EXPECT_NE(nullptr, ShmPool::create(makeMemfd(8192), 4096));
}

TEST_F(ShmPoolTest, GrowsOnlyAsFarAsTheFileHasGrown)
{
    const int fd = makeMemfd(4096);
    ASSERT_GE(fd, 0);
    const int spare = dup(fd);
    std::shared_ptr<ShmPool> pool = ShmPool::create(fd, 4096);
    ASSERT_TRUE(pool);

    EXPECT_FALSE(pool->resize(8192));
    EXPECT_EQ(4096u, pool->size());

    ASSERT_EQ(0, ftruncate(spare, 8192));
    EXPECT_TRUE(pool->resize(8192));
    EXPECT_EQ(8192u, pool->size());
    close(spare);
}

TEST_F(ShmPoolTest, AReadIsUnaffectedByAnIntactFile)
{
    std::shared_ptr<ShmPool> pool = makePool(8192);
    ASSERT_TRUE(pool);
    std::vector<uint8_t> copy(8192);

    pool->beginAccess();
    pool->beginAccess();
    std::memcpy(copy.data(), pool->data(), copy.size());
    EXPECT_TRUE(pool->endAccess());
    EXPECT_TRUE(pool->endAccess());
    EXPECT_FALSE(pool->faulted());
}

TEST_F(ShmPoolTest, SurvivesTheFileShrinkingUnderARead)
{
    const int fd = makeMemfd(8192);
    ASSERT_GE(fd, 0);
    const int spare = dup(fd);
    std::shared_ptr<ShmPool> pool = ShmPool::create(fd, 8192);
    ASSERT_TRUE(pool);
    std::shared_ptr<ShmPool> other = makePool(4096);
    ASSERT_TRUE(other);

    // The client shrinks the file after create_pool; the second page is now
    // beyond EOF and touching it raises SIGBUS.
    ASSERT_EQ(0, ftruncate(spare, 4096));
    close(spare);

    std::vector<uint8_t> copy(8192, 0xff);
    pool->beginAccess();
    std::memcpy(copy.data(), pool->data(), copy.size());
    EXPECT_FALSE(pool->endAccess());
    EXPECT_TRUE(pool->faulted());
    EXPECT_EQ(0, copy[4096]);
    EXPECT_EQ(0, copy[8191]);

    // The pool is dead but harmless: it stays readable and cannot grow.
    pool->beginAccess();
    std::memcpy(copy.data(), pool->data(), copy.size());
    EXPECT_FALSE(pool->endAccess());
    EXPECT_FALSE(pool->resize(16384));

    // Other pools are untouched.
    other->beginAccess();
    std::memcpy(copy.data(), other->data(), 4096);
    EXPECT_TRUE(other->endAccess());
    EXPECT_FALSE(other->faulted());
}

TEST_F(ShmPoolTest, AFaultedBufferReportsOnce)
{
    const int fd = makeMemfd(8192);
    ASSERT_GE(fd, 0);
    const int spare = dup(fd);
    std::shared_ptr<ShmPool> pool = ShmPool::create(fd, 8192);
    ASSERT_TRUE(pool);
    std::shared_ptr<ShmBuffer> buffer = pool->createBuffer(base::Size(32, 64), 0, 128, kFormat,
                                                           nullptr);
    ASSERT_TRUE(buffer);

    int faults = 0;
    buffer->setFaultCallback([&faults] { ++faults; });

    buffer->beginPixelAccess();
    std::vector<uint8_t> copy(8192);
    std::memcpy(copy.data(), buffer->pixels(), copy.size());
    EXPECT_TRUE(buffer->endPixelAccess());
    EXPECT_EQ(0, faults);

    ASSERT_EQ(0, ftruncate(spare, 4096));
    close(spare);

    buffer->beginPixelAccess();
    std::memcpy(copy.data(), buffer->pixels(), copy.size());
    EXPECT_FALSE(buffer->endPixelAccess());
    EXPECT_EQ(1, faults);

    buffer->beginPixelAccess();
    std::memcpy(copy.data(), buffer->pixels(), copy.size());
    EXPECT_FALSE(buffer->endPixelAccess());
    EXPECT_EQ(1, faults);
}

TEST_F(ShmPoolTest, WritesTightRowsInOneRange)
{
    std::shared_ptr<ShmPool> pool = makePool(4096);
    ASSERT_TRUE(pool);
    std::vector<uint8_t> source(16 * 8 * 4);
    for (size_t i = 0; i < source.size(); ++i)
        source[i] = uint8_t(i);

    ASSERT_TRUE(pool->writeRows(256, 16 * 4, 8, 16 * 4, source.data(), 16 * 4));
    EXPECT_EQ(0, std::memcmp(pool->data() + 256, source.data(), source.size()));
    // Nothing outside the range is touched.
    EXPECT_EQ(0, pool->data()[255]);
    EXPECT_EQ(0, pool->data()[256 + source.size()]);
}

TEST_F(ShmPoolTest, WritesPaddedRowsWithoutTouchingTheGaps)
{
    std::shared_ptr<ShmPool> pool = makePool(8192);
    ASSERT_TRUE(pool);
    // Dirty the pool first so untouched bytes are distinguishable.
    std::vector<uint8_t> dirt(8192, 0xaa);
    ASSERT_TRUE(pool->write(0, dirt.data(), dirt.size()));

    const size_t rowBytes = 10 * 4, stride = rowBytes + 24, dataStride = rowBytes + 8;
    std::vector<uint8_t> source(dataStride * 6, 0x11);
    for (size_t row = 0; row < 6; ++row)
        std::fill_n(source.data() + row * dataStride, rowBytes, uint8_t(row + 1));

    ASSERT_TRUE(pool->writeRows(64, stride, 6, rowBytes, source.data(), dataStride));
    for (size_t row = 0; row < 6; ++row) {
        const uint8_t *line = pool->data() + 64 + row * stride;
        for (size_t i = 0; i < rowBytes; ++i)
            ASSERT_EQ(uint8_t(row + 1), line[i]) << "row " << row << " byte " << i;
        for (size_t i = rowBytes; i < stride; ++i)
            ASSERT_EQ(0xaa, line[i]) << "row " << row << " padding " << i;
    }
    EXPECT_EQ(0xaa, pool->data()[63]);
}

TEST_F(ShmPoolTest, WriteRowsCopiesALargeTightFrame)
{
    const size_t rowBytes = 1920 * 4, rows = 1080;
    std::shared_ptr<ShmPool> pool = makePool(rows * rowBytes);
    ASSERT_TRUE(pool);
    std::vector<uint8_t> source(rows * rowBytes);
    for (size_t i = 0; i < source.size(); ++i)
        source[i] = uint8_t(i * 7);

    ASSERT_TRUE(pool->writeRows(0, rowBytes, rows, rowBytes, source.data(), rowBytes));
    EXPECT_EQ(0, std::memcmp(pool->data(), source.data(), source.size()));
}

TEST_F(ShmPoolTest, WriteRowsNeedsOnlyTheLastRowsPixels)
{
    std::shared_ptr<ShmPool> pool = makePool(3 * 40 + 32);
    ASSERT_TRUE(pool);
    std::vector<uint8_t> source(4 * 32, 0x5a);
    EXPECT_TRUE(pool->writeRows(0, 40, 4, 32, source.data(), 32));
    EXPECT_FALSE(pool->writeRows(0, 40, 4, 36, source.data(), 36));
    EXPECT_FALSE(pool->writeRows(4, 40, 4, 32, source.data(), 32));
}

TEST_F(ShmPoolTest, WriteRowsRejectsBadLayoutsAndAShrunkenFile)
{
    const int fd = makeMemfd(8192);
    ASSERT_GE(fd, 0);
    const int spare = dup(fd);
    std::shared_ptr<ShmPool> pool = ShmPool::create(fd, 8192);
    ASSERT_TRUE(pool);
    std::vector<uint8_t> source(8192);

    EXPECT_TRUE(pool->writeRows(0, 64, 0, 64, source.data(), 64));
    EXPECT_FALSE(pool->writeRows(0, 60, 4, 64, source.data(), 64));
    EXPECT_FALSE(pool->writeRows(0, 64, 4, 64, source.data(), 60));
    EXPECT_FALSE(pool->writeRows(0, std::numeric_limits<size_t>::max() / 2, 4, 64,
                                 source.data(), 64));

    ASSERT_EQ(0, ftruncate(spare, 4096));
    close(spare);
    EXPECT_TRUE(pool->writeRows(0, 64, 64, 64, source.data(), 64));
    EXPECT_FALSE(pool->writeRows(0, 64, 65, 64, source.data(), 64));
    EXPECT_FALSE(pool->write(4096, source.data(), 1));
}

TEST_F(ShmPoolTest, BuffersKeepThePoolAlive)
{
    std::shared_ptr<ShmPool> pool = makePool(4096);
    ASSERT_TRUE(pool);
    std::shared_ptr<ShmBuffer> buffer = pool->createBuffer(base::Size(16, 16), 0, 64, kFormat,
                                                           nullptr);
    ASSERT_TRUE(buffer);

    const ShmPool* raw = pool.get();
    pool.reset();

    // The client destroyed wl_shm_pool; the mapping is still readable.
    EXPECT_EQ(raw, buffer->pool());
    EXPECT_NE(nullptr, buffer->pixels());
}

} // namespace
