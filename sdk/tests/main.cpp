// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstring>

#include <gtest/gtest.h>

#include "ui/application.h"
#include "ui/test_input.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    // --verify-layout: every pass is checked against a full one.
    bool verifiesLayout = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--verify-layout") == 0)
            verifiesLayout = true;
    }
    Lingmo::UI::Application application(argc, argv,
                                          Lingmo::UI::Application::Platform::Headless);
    Lingmo::UI::TestInput::setVerifiesLayout(verifiesLayout);
    const int result = RUN_ALL_TESTS();
    if (verifiesLayout)
        printf("%d views laid out differently in full\n", Lingmo::UI::TestInput::layoutMismatches());
    return result;
}
