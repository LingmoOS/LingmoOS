// Copyright 2019 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/wm_shadow_controller_delegate.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_WM_WM_SHADOW_CONTROLLER_DELEGATE_H_
#define LINGMO_WM_WM_SHADOW_CONTROLLER_DELEGATE_H_

#include "wm/shadow_controller_delegate.h"

namespace Lingmo::WM {

// Lingmo's shadow policy. Ash's delegate suppresses shadows for windows in
// overview, in a window-cycle list and in the PIP; Lingmo has none of those,
// and instead has to answer a question Ash never faces, because ChromeOS has no
// client-side decoration: whether the client already draws its own exterior.
class WmShadowControllerDelegate : public ShadowControllerDelegate
{
public:
    bool shouldShowShadowForWindow(const Window* window) override;
    ShadowElevation elevationForWindow(const Window* window) override;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WM_SHADOW_CONTROLLER_DELEGATE_H_
