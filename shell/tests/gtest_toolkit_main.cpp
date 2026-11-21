// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <clocale>

#include <gtest/gtest.h>

#include "core/main_loop.h"
#include "core/preferences.h"
#include "temporary_directory.h"

// The main loop as the compositor's main() sets it up, so the timers, fd
// watches and posted tasks of the code under test run while a test waits
// (loop_test_util.h). Preferences go to a scratch directory, never the
// user's.
int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    testing::InitGoogleTest(&argc, argv);
    const Lingmo::test::TemporaryDirectory configHome;
    Lingmo::setConfigHomeForTesting(configHome.path());
    const Lingmo::core::MainLoop loop;
    const int result = RUN_ALL_TESTS();
    Lingmo::setConfigHomeForTesting({});
    return result;
}
