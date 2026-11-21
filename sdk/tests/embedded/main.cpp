// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "base/event_loop.h"
#include "ui/application.h"

Lingmo::Base::EventLoop* g_hostLoop = nullptr;

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    // As the compositor does: the host's loop exists first and is shared.
    Lingmo::Base::EventLoop hostLoop;
    g_hostLoop = &hostLoop;
    Lingmo::UI::Application application(argc, argv,
                                          Lingmo::UI::Application::Platform::Embedded);
    return RUN_ALL_TESTS();
}
