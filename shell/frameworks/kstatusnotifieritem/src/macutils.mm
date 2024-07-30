/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2023 Hannah von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#import <AppKit/NSApplication.h>
#import <AppKit/NSDockTile.h>

#include <QString>
namespace MacUtils
{
void setBadgeLabelText(const QString &text)
{
    [[[NSApplication sharedApplication] dockTile] setBadgeLabel:text.toNSString()];
}
}