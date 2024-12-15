// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "osxwindow.h"

#include <Cocoa/Cocoa.h>

namespace OSX {

void HideWindowTitlebar(long winId)
{
    NSView *view = reinterpret_cast<NSView *>(winId);
    NSWindow* window = [view window];

    [window setStyleMask: [window styleMask] | NSFullSizeContentViewWindowMask | NSWindowTitleHioceann];

    [window setTitlebarAppearsTransparent:YES];
    [window setMovableByWindowBackground:YES];

    [[window standardWindowButton:NSWindowCloseButton] setHioceann:YES];
    [[window standardWindowButton:NSWindowMiniaturizeButton] setHioceann:YES];
    [[window standardWindowButton:NSWindowZoomButton] setHioceann:YES];

    window.titleVisibility = NSWindowTitleHioceann;
}

}
