// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/display/screen.h"

#include <algorithm>
#include <chrono>

#include "framework_i18n.h"

using namespace std::chrono_literals;
using Lingmo::framework::tr;

namespace Lingmo::Display {

Screen::Screen(std::string serviceName)
    : m_client(std::move(serviceName))
{
    m_countdown.setInterval(250ms);
    m_countdown.onTimeout = [this] { stateChanged.emit(); };
    m_subscriptions.push_back(m_client.availabilityChanged.connect([this] { stateChanged.emit(); }));
    m_subscriptions.push_back(m_client.stateChanged.connect([this] { updateState(); }));
    m_subscriptions.push_back(m_outputModel.dirtyChanged.connect([this] {
        stateChanged.emit();
        if (dirty())
            apply();
    }));
    m_subscriptions.push_back(m_client.operationFinished.connect([this](DisplayClient::Operation operation) {
        m_busy = false;
        m_error = operation.succeeded && !operation.degraded ? std::string() : operation.message;
        m_outputModel.setState(m_client.state());
        stateChanged.emit();
    }));
    m_outputModel.setState(m_client.state());
    updateState();
}

Screen::~Screen() = default;

bool Screen::canApply() const
{
    return available() && !m_busy && dirty() && !pendingConfirmation();
}

int Screen::confirmationSeconds() const
{
    if (!pendingConfirmation())
        return 0;
    const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    return int(std::max<int64_t>(0, (int64_t(m_client.state().previewDeadlineMs) - now + 999) / 1000));
}

void Screen::updateState()
{
    const DisplayClient::State& state = m_client.state();
    if (m_client.loaded() && !m_client.available()) {
        m_busy = false;
        m_error = tr("The display service is unavailable.");
    }
    const bool topologyChanged = !m_instanceId.empty()
                                 && (m_instanceId != state.instanceId
                                     || m_topologyGeneration != state.topologyGeneration);
    if (!m_outputModel.dirty() || topologyChanged || m_busy || pendingConfirmation())
        m_outputModel.setState(state);
    if (topologyChanged)
        m_error = tr("Displays changed. The previous draft was discarded.");
    m_instanceId = state.instanceId;
    m_topologyGeneration = state.topologyGeneration;
    if (pendingConfirmation()) {
        if (!m_countdown.isActive())
            m_countdown.start();
    } else {
        m_countdown.stop();
    }
    stateChanged.emit();
}

void Screen::apply()
{
    if (!canApply())
        return;
    m_busy = true;
    m_error.clear();
    stateChanged.emit();
    if (!m_client.apply(m_outputModel.requests(), m_outputModel.primary(), m_outputModel.placements(),
                        m_outputModel.mirrorGroups(), m_outputModel.needsConfirmation())) {
        m_busy = false;
        m_error = tr("The display service is unavailable.");
        stateChanged.emit();
    }
}

void Screen::discard()
{
    if (!m_busy) {
        m_outputModel.discard();
        m_error.clear();
        stateChanged.emit();
    }
}

void Screen::keep()
{
    if (pendingConfirmation())
        m_client.confirm(m_client.state().previewToken);
}

void Screen::revert()
{
    if (pendingConfirmation())
        m_client.revert(m_client.state().previewToken);
}

} // namespace Lingmo::Display
