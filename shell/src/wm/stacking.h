// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_STACKING_H_
#define LINGMO_WM_STACKING_H_

#include "wm/types.h"

namespace Lingmo::WM {

class Window;

// The one place role-aware z-order is decided. Nothing outside this header may
// hardcode a layer number, and no raise or activation may move a window out of
// the layer its role puts it in.
//
// Bottom to top:
//
//   Desktop < Normal applications < AlwaysOnTop < DesktopPopup < Panel < Launcher < SystemDialog < Dock < Notification < Osd < Capture
//
// SystemDialog holds the session's own dialogs - permission prompts and the
// like, asked by trusted processes on another application's behalf - which no
// application window may cover, and which still sit under the dock.
//
// Osd is the level a volume or brightness key shows, which must not hide
// behind a banner that happens to be up.
//
// Capture is the screen capture interface, which has to reach every pixel the
// user can point at, the panels and banners included.
//
// The cursor is not a window; it composites above all of this from its own
// scene container.
enum class StackingLayer {
    Desktop = 0,
    Normal = 1,
    AlwaysOnTop = 2,
    DesktopPopup = 3,
    Panel = 4,
    Launcher = 5,
    SystemDialog = 6,
    Dock = 7,
    Notification = 8,
    Osd = 9,
    Picker = 10,
    Capture = 11,
};

// A panel whose behavior is WindowsCanCover drops into the normal layer.
StackingLayer layerFor(WindowRole role, PanelBehavior behavior);
StackingLayer layerFor(const Window *window);

} // namespace Lingmo::WM

#endif // LINGMO_WM_STACKING_H_
