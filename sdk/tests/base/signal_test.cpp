// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// As a host toolkit's headers may leave them: empty macros.
#define emit
#define slots
#define signals public

#include <gtest/gtest.h>

#include "base/signal.h"

using namespace Lingmo::Base;

TEST(SignalTest, CompilesAfterKeywordMacros)
{
    Signal<int> signal;
    int total = 0;
    Subscription subscription = signal.connect([&](int value) { total += value; });
    signal(2);
    signal(3);
    EXPECT_EQ(total, 5);
    subscription.reset();
    signal(4);
    EXPECT_EQ(total, 5);
    EXPECT_TRUE(signal.isEmpty());
}
