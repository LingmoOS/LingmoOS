// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "pages/input/compositorservice.h"

namespace settings {

// Client of the compositor's org.lingmo.Touchpad1 service. Without the
// service every value is the compositor's default.
class Touchpad
{
public:
    Touchpad();

    // Whether the service runs and a touchpad is there.
    bool isAvailable() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool disableWithMouse() const;
    void setDisableWithMouse(bool enabled);

    bool disableWhileTyping() const;
    void setDisableWhileTyping(bool enabled);

    bool tapToClick() const;
    void setTapToClick(bool enabled);

    bool tapDragging() const;
    void setTapDragging(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    // libinput's pointer speed, -1 to 1.
    double pointerAcceleration() const;
    void setPointerAcceleration(double value);

    double scrollSpeed() const;
    void setScrollSpeed(double value);

    // Pads with physical buttons, or only one click method, have no choice.
    bool secondaryClickConfigurable() const;
    bool twoFingerSecondaryClick() const;
    void setTwoFingerSecondaryClick(bool enabled);

    Lingmo::base::Signal<>& changed() { return m_service.changed; }

private:
    CompositorService m_service;
};

} // namespace settings
