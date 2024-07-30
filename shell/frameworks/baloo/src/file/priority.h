/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_LINUX_PRIORITY_H_
#define BALOO_LINUX_PRIORITY_H_

bool lowerIOPriority();
/// Sets the priority to batch
bool lowerSchedulingPriority();
bool setIdleSchedulingPriority();
bool lowerPriority();

#endif // BALOO_LINUX_PRIORITY_H_
