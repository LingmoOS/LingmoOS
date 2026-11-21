// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/idle_inhibition.h"

#include <cassert>

namespace Lingmo::Wayland {

void IdleInhibition::acquire()
{
    if (++m_count == 1 && !m_ignored)
        inhibitedChanged();
}

void IdleInhibition::release()
{
    assert(m_count);
    if (--m_count == 0 && !m_ignored)
        inhibitedChanged();
}

void IdleInhibition::setIgnored(bool ignored)
{
    const bool wasInhibited = inhibited();
    m_ignored = ignored;
    if (inhibited() != wasInhibited)
        inhibitedChanged();
}
} // namespace Lingmo::Wayland
