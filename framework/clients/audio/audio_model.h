// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <vector>

#include "base/signal.h"
#include "lingmo/audio/audio_object.h"

namespace Lingmo::Audio {

class Backend;

// The backend's objects of one kind: those there when the model was made, by
// id, then the ones added later, at the end.
class AudioModel
{
public:
    AudioModel(AudioObject::Kind kind, std::shared_ptr<Backend> backend);

    AudioObject::Kind kind() const { return m_kind; }
    const std::shared_ptr<Backend>& backend() const { return m_backend; }
    int count() const { return int(m_objects.size()); }
    const std::vector<std::shared_ptr<AudioObject>>& objects() const { return m_objects; }
    // The default sink or source.
    std::shared_ptr<AudioObject> defaultNode() const;

    // An object came or went.
    base::Signal<> rowsChanged;
    // A listed object's values changed.
    base::Signal<> dataChanged;
    base::Signal<> defaultNodeChanged;

private:
    void addObject(const std::shared_ptr<AudioObject>& object);
    void removeObject(const std::shared_ptr<AudioObject>& object);

    std::shared_ptr<Backend> m_backend;
    AudioObject::Kind m_kind;
    std::vector<std::shared_ptr<AudioObject>> m_objects;
    std::vector<base::Subscription> m_objectSubscriptions;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Audio
