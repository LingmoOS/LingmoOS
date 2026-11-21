// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pipewire/extensions/metadata.h>
#include <pipewire/pipewire.h>

#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/audio/audio_object.h"

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::Audio {

// The PipeWire daemon's audio cards, nodes and streams, read on the calling
// thread's event loop, with the default sink and source from its metadata.
// Reconnects a second after the daemon goes.
class Backend
{
public:
    // Shared by everyone in the process who asks while one is alive.
    static std::shared_ptr<Backend> shared();

    Backend();
    ~Backend();

    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;

    bool isConnected() const { return m_connected; }
    // By id.
    std::vector<std::shared_ptr<AudioObject>> objects() const;
    std::shared_ptr<AudioObject> object(uint32_t id) const;
    std::shared_ptr<AudioObject> defaultNode(AudioObject::Kind kind) const;
    pw_core* core() const { return m_core; }

    void setVolume(AudioObject* object, const std::vector<float>& volumes);
    void setMute(AudioObject* object, bool mute);
    void setDefault(AudioObject* object, bool enable);
    void setProfile(AudioObject* object, int profile);
    void setRoute(AudioObject* object, int route);
    void moveStream(AudioObject* object, uint32_t target);

    base::Signal<std::shared_ptr<AudioObject>> objectAdded;
    // Before the backend lets go of the object.
    base::Signal<std::shared_ptr<AudioObject>> objectRemoved;
    base::Signal<> defaultsChanged;
    base::Signal<> connectedChanged;
    base::Signal<> disconnecting;
    base::Signal<std::string> operationFailed;

private:
    struct Binding
    {
        Backend* backend = nullptr;
        std::shared_ptr<AudioObject> object;
        pw_proxy* proxy = nullptr;
        spa_hook listener{};
        uint32_t permissions = 0;
        int enumSequence = 0;
        std::map<uint32_t, uint32_t> paramFlags;
        base::Subscription cardChanged;
    };

    void connectToDaemon();
    void disconnectFromDaemon();
    void releaseContext();
    void releaseMetadata();
    void fail();
    void iterate();
    void addGlobal(uint32_t id, uint32_t permissions, const char* type, uint32_t version, const spa_dict* properties);
    void removeGlobal(uint32_t id);
    void updateDefaults();
    void updateLinks();
    void updateRouteVolume(AudioObject* object);
    void setNodeParam(AudioObject* object, const spa_pod* param);
    void checkResult(int result, const char* operation);

    static const pw_core_events coreEvents;
    static const pw_registry_events registryEvents;
    static const pw_node_events nodeEvents;
    static const pw_device_events deviceEvents;
    static const pw_metadata_events metadataEvents;

    base::EventLoop* m_eventLoop;
    pw_loop* m_loop = nullptr;
    pw_context* m_context = nullptr;
    pw_core* m_core = nullptr;
    pw_registry* m_registry = nullptr;
    pw_metadata* m_metadata = nullptr;
    spa_hook m_coreListener{};
    spa_hook m_registryListener{};
    spa_hook m_metadataListener{};
    uint32_t m_metadataId = AudioObject::InvalidId;
    uint32_t m_metadataPermissions = 0;
    int m_watch = 0;
    base::Timer m_retry;
    base::Timer m_failure;
    std::map<uint32_t, std::unique_ptr<Binding>> m_bindings;
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> m_links;
    std::string m_defaultSink;
    std::string m_defaultSource;
    bool m_connected = false;
    bool m_iterating = false;
    bool m_failurePending = false;
};

} // namespace Lingmo::Audio
