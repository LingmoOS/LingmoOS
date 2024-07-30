/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TIMESTAMPS_H
#define TIMESTAMPS_H

#include <chrono>

// This is a helper header to simplify some of the std::chrono usages.
namespace TimeStamp
{
using MicroSeconds = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
using Seconds = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>;
}

#endif
