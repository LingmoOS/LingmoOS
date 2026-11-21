// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "fake_native_display_delegate.h"

using namespace Lingmo;

TEST(NativeDisplayDelegate, FakeCompletesEveryOperation)
{
    test::FakeNativeDisplayDelegate delegate;
    bool controlCompleted = false;
    delegate.takeDisplayControl([&](bool success) { controlCompleted = success; });
    EXPECT_TRUE(controlCompleted);

    bool queryCompleted = false;
    delegate.queryHardware([&](auto snapshot, auto error) {
        queryCompleted = true;
        EXPECT_FALSE(snapshot);
        EXPECT_EQ(error.code, display::DisplayErrorCode::Unavailable);
    });
    EXPECT_TRUE(queryCompleted);

    bool configurationCompleted = false;
    display::DisplayConfiguration configuration;
    delegate.configure(configuration, display::NativeDisplayDelegate::ConfigureMode::TestOnly,
                       [&](display::ConfigurationResult result) {
                           configurationCompleted = true;
                           EXPECT_TRUE(result.succeeded);
                       });
    EXPECT_TRUE(configurationCompleted);
    EXPECT_EQ(delegate.configureCount(), 1);
    EXPECT_EQ(delegate.lastConfigureMode(),
              display::NativeDisplayDelegate::ConfigureMode::TestOnly);

    bool powerCompleted = false;
    delegate.setDisplayPower({}, display::NativeDisplayDelegate::PowerState::Off,
                             [&](display::ConfigurationResult result) {
                                 powerCompleted = true;
                                 EXPECT_TRUE(result.succeeded);
                             });
    EXPECT_TRUE(powerCompleted);
}
