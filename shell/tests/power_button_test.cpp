// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fake_logind.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "session/power_button.h"
#include "session/session_controller.h"

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

using namespace Lingmo;

namespace {

struct Keys : input::EventSink {
    std::vector<uint32_t> keys;
    std::vector<uint32_t> secureKeys;

    bool onKey(input::Timestamp, uint32_t evdevCode, bool pressed) override
    {
        if (pressed)
            keys.push_back(evdevCode);
        return false;
    }

    bool onSecureKey(input::Timestamp, uint32_t evdevCode, bool pressed) override
    {
        if (pressed)
            secureKeys.push_back(evdevCode);
        return true;
    }
};

class PowerButtonTest : public testing::Test
{
protected:
    test::FakeLogind logind;
    scene::Node root;
    session::SecureContainer secure {&root};
    session::SessionController controller {&secure};
    Keys next;
    session::PowerButton button {&controller, &next, base::dbus::Connection::session(core::mainLoop())};

    PowerButtonTest() { controller.setOutputs({{"one", base::Rect(0, 0, 800, 600)}}); }

    bool waitFor(const std::function<bool()> &done) { return test::runUntil(done); }

    void unlock()
    {
        controller.notifyOutputPresented("one");
        EXPECT_TRUE(controller.requestUnlock());
        controller.notifyOutputPresented("one");
        EXPECT_EQ(controller.state(), session::SessionState::Active);
    }

    void tap()
    {
        EXPECT_TRUE(button.onKey(input::Timestamp {}, KEY_POWER, true));
        EXPECT_TRUE(button.onKey(input::Timestamp {}, KEY_POWER, false));
    }
};

} // namespace

TEST_F(PowerButtonTest, ATapLocks)
{
    ASSERT_TRUE(logind.isValid());
    ASSERT_TRUE(button.start());
    ASSERT_TRUE(waitFor([&] { return button.holdsInhibitor(); }));
    ASSERT_EQ(logind.lastInhibit.size(), 4u);
    EXPECT_EQ(logind.lastInhibit[0], "handle-power-key");
    EXPECT_EQ(logind.lastInhibit[3], "block");

    tap();
    EXPECT_EQ(controller.state(), session::SessionState::Locking);
    EXPECT_TRUE(next.keys.empty());
}

TEST_F(PowerButtonTest, TheKeyPassesOnUntilLogindLetsGo)
{
    button.onKey(input::Timestamp {}, KEY_POWER, true);
    button.onKey(input::Timestamp {}, KEY_POWER, false);
    button.onKey(input::Timestamp {}, KEY_A, true);
    EXPECT_EQ(next.keys, (std::vector<uint32_t> {KEY_POWER, KEY_A}));
    EXPECT_EQ(controller.state(), session::SessionState::Active);
}

TEST_F(PowerButtonTest, KeysForTheLockScreenPassOn)
{
    // The accelerators behind it take the volume and brightness keys there.
    EXPECT_TRUE(button.onSecureKey(input::Timestamp {}, KEY_VOLUMEUP, true));
    EXPECT_TRUE(button.onSecureKey(input::Timestamp {}, KEY_VOLUMEUP, false));
    EXPECT_EQ(next.secureKeys, (std::vector<uint32_t> {KEY_VOLUMEUP}));
}

TEST_F(PowerButtonTest, ThePressThatWokeTheSystemDoesNotLock)
{
    ASSERT_TRUE(logind.isValid());
    ASSERT_TRUE(button.start());
    ASSERT_TRUE(waitFor([&] { return button.holdsInhibitor(); }));
    logind.prepareForSleep(true);
    logind.prepareForSleep(false);
    test::runFor(std::chrono::milliseconds(100));
    tap();
    EXPECT_EQ(controller.state(), session::SessionState::Active);
}

TEST_F(PowerButtonTest, RapidTapsActOnce)
{
    ASSERT_TRUE(logind.isValid());
    ASSERT_TRUE(button.start());
    ASSERT_TRUE(waitFor([&] { return button.holdsInhibitor(); }));
    tap();
    unlock();
    tap();
    EXPECT_EQ(controller.state(), session::SessionState::Active);
    test::runFor(std::chrono::milliseconds(600));
    tap();
    EXPECT_EQ(controller.state(), session::SessionState::Locking);
}
