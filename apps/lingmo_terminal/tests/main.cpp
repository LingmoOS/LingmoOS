// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <clocale>

int main(int argc, char** argv)
{
    // Character widths follow the locale, as in the application.
    std::setlocale(LC_ALL, "");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
