// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/signal.h"
#include "pages/input/compositorservice.h"

namespace settings {

// Client of the compositor's org.lingmo.Mouse1 service.
class Mouse
{
public:
    Mouse();

    // Whether the service runs and a mouse is connected.
    bool isAvailable() const;

    bool locatorAvailable() const;
    bool shakeToLocate() const;
    void setShakeToLocate(bool enabled);

    bool leftHanded() const;
    void setLeftHanded(bool enabled);

    bool acceleration() const;
    void setAcceleration(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    // libinput's pointer speed, -1 to 1.
    double pointerAcceleration() const;
    void setPointerAcceleration(double value);

    Lingmo::base::Signal<>& changed() { return m_service.changed; }

private:
    CompositorService m_service;
};

} // namespace settings
