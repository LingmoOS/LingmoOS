// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/frame_client.h"

#include "ui/window.h"

namespace Lingmo::ui {

FrameClient::FrameClient(Window* window)
    : m_window(window)
{
    if (m_window)
        m_window->addFrameClient(this);
}

FrameClient::~FrameClient()
{
    if (m_window)
        m_window->removeFrameClient(this);
}

void FrameClient::startTicking()
{
    if (m_ticking)
        return;
    m_ticking = true;
    if (m_window)
        m_window->scheduleFrame();
}

void FrameClient::stopTicking()
{
    m_ticking = false;
}

} // namespace Lingmo::ui
