// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/{data_source,data_offer,data_device,seat}.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "clipboard/selection.h"

#include <algorithm>
#include <unistd.h>
#include <utility>

namespace Lingmo::Clipboard {

bool DataSource::offer(const std::string &mime)
{
    if (!m_alive || mime.empty() || mime.size() > 1024 || m_formats.size() >= 128)
        return false;
    if (!supports(mime))
        m_formats.push_back(mime);
    return true;
}

bool DataSource::supports(const std::string &mime) const
{
    return std::find(m_formats.begin(), m_formats.end(), mime) != m_formats.end();
}

void DataSource::invalidate()
{
    if (!std::exchange(m_alive, false))
        return;
    invalidated();
}

void DataSource::cancel()
{
    if (!m_alive)
        return;
    const auto alive = m_weakFactory.getWeakPtr();
    invalidate();
    if (alive)
        cancelled();
}

SelectionController::~SelectionController()
{
    m_destroying = true;
    m_sourceConnection.reset();
    auto previous = std::exchange(m_source, nullptr);
    if (previous)
        previous->cancel();
    destroyed();
}

void SelectionController::setRecipient(Recipient recipient)
{
    if (m_recipient == recipient)
        return;
    m_recipient = recipient;
    ++m_focusGeneration;
    m_serials.clear();
    recipientChanged();
}

void SelectionController::recordSerial(Recipient recipient, uint32_t serial)
{
    if (!recipient || recipient != m_recipient)
        return;
    m_serials.push_back({serial, ++m_sequence});
    if (m_serials.size() > 256)
        m_serials.pop_front();
}

bool SelectionController::authorize(Recipient recipient, uint32_t serial)
{
    return authorize(recipient, serial, *this);
}

bool SelectionController::authorize(Recipient recipient, uint32_t serial,
                                    SelectionController &selection)
{
    if (m_isolated || selection.m_isolated)
        return false;
    if (!recipient || recipient != m_recipient)
        return false;
    for (auto it = m_serials.rbegin(); it != m_serials.rend(); ++it) {
        if (it->value == serial) {
            if (it->sequence < selection.m_lastAccepted)
                return false;
            selection.m_lastAccepted = it->sequence;
            return true;
        }
    }
    return false;
}

void SelectionController::setIsolated(bool isolated)
{
    if (m_isolated == isolated)
        return;
    m_isolated = isolated;
    if (isolated) {
        m_savedSource = m_source;
        m_savedRecipient = m_recipient;
        // Not cancelled: the session's source stays alive and unused, it is
        // simply not the current selection while the screen is secure.
        m_sourceConnection.reset();
        m_source = nullptr;
        ++m_generation;
        setRecipient(nullptr);
        selectionChanged();
        return;
    }

    auto restored = std::exchange(m_savedSource, nullptr);
    const Recipient recipient = std::exchange(m_savedRecipient, nullptr);
    // Nothing to drop from the secure screen: setSource refused it all, so the
    // secure clipboard was empty for its whole life.
    m_source = restored && restored->alive() ? std::move(restored) : nullptr;
    if (m_source) {
        m_sourceConnection = m_source->invalidated.connect([this, expected = m_source.get()] {
            sourceDestroyed(expected);
        });
    }
    ++m_generation;
    setRecipient(recipient);
    selectionChanged();
}

bool SelectionController::setSource(std::shared_ptr<DataSource> source)
{
    // Refused while isolated, in both directions: nothing set behind the secure
    // screen becomes the session's selection, and nothing set by the session
    // replaces what the secure screen sees.
    if (m_isolated)
        return false;
    if (m_destroying || (source && (!source->alive() || source->used())))
        return false;
    m_sourceConnection.reset();
    auto previous = std::exchange(m_source, std::move(source));
    const auto generation = ++m_generation;
    if (m_source) {
        m_source->markUsed();
        m_sourceConnection = m_source->invalidated.connect([this, expected = m_source.get()] {
            sourceDestroyed(expected);
        });
    }
    const auto alive = m_weakFactory.getWeakPtr();
    if (previous)
        previous->cancel();
    if (!alive)
        return false;
    if (generation != m_generation)
        return false;
    selectionChanged();
    return alive && generation == m_generation;
}

void SelectionController::sourceDestroyed(DataSource *source)
{
    if (m_source.get() == source)
        setSource(nullptr);
}

DataOffer::DataOffer(SelectionController *controller)
    : m_controller(controller->weakPtr())
    , m_source(controller->source())
    , m_formats(controller->source() ? controller->source()->formats() : std::vector<std::string>())
    , m_generation(controller->generation())
    , m_focusGeneration(controller->focusGeneration())
{
    const auto changed = [this] {
        if (!valid())
            invalidate();
    };
    m_selectionChanged = controller->selectionChanged.connect(changed);
    m_recipientChanged = controller->recipientChanged.connect(changed);
}

void DataOffer::invalidate()
{
    m_eligible = false;
}

bool DataOffer::valid() const
{
    auto source = m_source.lock();
    return m_eligible && m_controller && m_controller->recipient() && source && source->alive()
            && m_controller->generation() == m_generation
            && m_controller->focusGeneration() == m_focusGeneration;
}

void DataOffer::receive(const std::string &mime, int fd)
{
    auto source = m_source.lock();
    if (!valid() || std::find(m_formats.begin(), m_formats.end(), mime) == m_formats.end()) {
        close(fd);
        return;
    }
    source->send(mime, fd);
}

} // namespace Lingmo::Clipboard
