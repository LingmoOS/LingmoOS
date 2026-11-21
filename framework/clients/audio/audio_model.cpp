// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/audio/audio_model.h"

#include <algorithm>

#include "lingmo/audio/backend.h"

namespace Lingmo::Audio {

AudioModel::AudioModel(AudioObject::Kind kind, std::shared_ptr<Backend> backend)
    : m_backend(std::move(backend))
    , m_kind(kind)
{
    m_subscriptions.push_back(
        m_backend->objectAdded.connect([this](std::shared_ptr<AudioObject> object) { addObject(object); }));
    m_subscriptions.push_back(
        m_backend->objectRemoved.connect([this](std::shared_ptr<AudioObject> object) { removeObject(object); }));
    m_subscriptions.push_back(m_backend->defaultsChanged.connect([this] { defaultNodeChanged.emit(); }));
    for (const auto& object : m_backend->objects())
        addObject(object);
}

std::shared_ptr<AudioObject> AudioModel::defaultNode() const
{
    return m_backend->defaultNode(m_kind);
}

void AudioModel::addObject(const std::shared_ptr<AudioObject>& object)
{
    if (object->kind() != m_kind)
        return;
    m_objects.push_back(object);
    m_objectSubscriptions.push_back(object->changed.connect([this] { dataChanged.emit(); }));
    rowsChanged.emit();
}

void AudioModel::removeObject(const std::shared_ptr<AudioObject>& object)
{
    const auto found = std::find(m_objects.begin(), m_objects.end(), object);
    if (found == m_objects.end())
        return;
    const auto row = found - m_objects.begin();
    m_objects.erase(found);
    m_objectSubscriptions.erase(m_objectSubscriptions.begin() + row);
    rowsChanged.emit();
}

} // namespace Lingmo::Audio
