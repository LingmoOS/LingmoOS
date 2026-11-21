// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <fcntl.h>
#include <stdlib.h>

#include <cerrno>
#include <memory>

#include "backend/session/libseat_session.h"

using namespace Lingmo;

namespace {

bool isOpen(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

// libseat's noop backend opens devices itself, so no seat manager is needed;
// like logind and seatd it leaves the descriptor it hands out to the caller.
std::unique_ptr<backend::LibseatSession> createSession()
{
    setenv("LIBSEAT_BACKEND", "noop", 1);
    return backend::LibseatSession::create();
}

} // namespace

TEST(LibseatSession, ClosingADeviceClosesItsDescriptor)
{
    auto session = createSession();
    ASSERT_TRUE(session);
    int deviceId = -1;
    const int fd = session->openDevice("/dev/null", &deviceId);
    ASSERT_GE(fd, 0);
    ASSERT_GE(deviceId, 0);
    EXPECT_TRUE(isOpen(fd));

    session->closeDevice(deviceId);
    EXPECT_FALSE(isOpen(fd));
}

TEST(LibseatSession, ClosingOneDeviceLeavesTheOthersOpen)
{
    auto session = createSession();
    ASSERT_TRUE(session);
    int firstId = -1;
    int secondId = -1;
    const int first = session->openDevice("/dev/null", &firstId);
    const int second = session->openDevice("/dev/zero", &secondId);
    ASSERT_GE(first, 0);
    ASSERT_GE(second, 0);

    session->closeDevice(firstId);
    EXPECT_FALSE(isOpen(first));
    EXPECT_TRUE(isOpen(second));

    session->closeDevice(secondId);
    EXPECT_FALSE(isOpen(second));
}
