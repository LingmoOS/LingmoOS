// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/audio/backend.h"

#include <algorithm>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <cstring>
#include <mutex>
#include <sys/epoll.h>

#include <spa/param/audio/raw.h>
#include <spa/param/profile.h>
#include <spa/param/props.h>
#include <spa/param/route.h>
#include <spa/pod/builder.h>
#include <spa/utils/json.h>

#include "base/event_loop.h"
#include "framework_i18n.h"

using namespace std::chrono_literals;
using Lingmo::framework::tr;
using Lingmo::i18n::format;
using Lingmo::i18n::trNoop;

namespace Lingmo::Audio {

// The "name" of a metadata value such as {"name":"alsa_output.pci"}.
static std::string jsonName(const char* value)
{
    if (!value)
        return {};
    spa_json document;
    spa_json object;
    spa_json_init(&document, value, std::strlen(value));
    if (spa_json_enter_object(&document, &object) <= 0)
        return {};
    char key[256];
    while (spa_json_get_string(&object, key, sizeof(key)) > 0) {
        const char* item = nullptr;
        const int length = spa_json_next(&object, &item);
        if (length <= 0)
            break;
        if (std::strcmp(key, "name") == 0 && spa_json_is_string(item, length)) {
            std::string name(size_t(length) + 1, '\0');
            if (spa_json_parse_stringn(item, length, name.data(), int(name.size())) < 0)
                return {};
            name.resize(std::strlen(name.c_str()));
            return name;
        }
        if (spa_json_is_container(item, length) && spa_json_container_len(&object, item, length) <= 0)
            break;
    }
    return {};
}

// {"name":"<name>"}, escaped as a compact JSON document has it.
static std::string jsonNameDocument(const std::string& name)
{
    std::string document = "{\"name\":\"";
    for (const unsigned char c : name) {
        switch (c) {
        case '"':
            document += "\\\"";
            break;
        case '\\':
            document += "\\\\";
            break;
        case '\b':
            document += "\\b";
            break;
        case '\f':
            document += "\\f";
            break;
        case '\n':
            document += "\\n";
            break;
        case '\r':
            document += "\\r";
            break;
        case '\t':
            document += "\\t";
            break;
        default:
            if (c < 0x20) {
                char escaped[8];
                std::snprintf(escaped, sizeof(escaped), "\\u%04x", c);
                document += escaped;
            } else {
                document += char(c);
            }
        }
    }
    return document + "\"}";
}

static bool parseId(const char* text, uint32_t& value)
{
    if (!text || !*text)
        return false;
    const char* end = text + std::strlen(text);
    const auto result = std::from_chars(text, end, value);
    return result.ec == std::errc() && result.ptr == end;
}

const pw_core_events Backend::coreEvents = [] {
    pw_core_events events{};
    events.version = PW_VERSION_CORE_EVENTS;
    events.info = [](void* data, const pw_core_info*) {
        auto* self = static_cast<Backend*>(data);
        if (!self->m_connected) {
            self->m_connected = true;
            self->connectedChanged.emit();
        }
    };
    events.error = [](void* data, uint32_t id, int, int result, const char* message) {
        auto* self = static_cast<Backend*>(data);
        self->operationFailed.emit(message ? message : "");
        if (id == PW_ID_CORE && (result == -EPIPE || result == -ECONNRESET)) {
            if (!self->m_failurePending) {
                self->m_failurePending = true;
                // Not from inside the daemon's own event.
                self->m_failure.start(0ms);
            }
        }
    };
    return events;
}();

const pw_registry_events Backend::registryEvents = [] {
    pw_registry_events events{};
    events.version = PW_VERSION_REGISTRY_EVENTS;
    events.global = [](void* data, uint32_t id, uint32_t permissions, const char* type, uint32_t version,
                       const spa_dict* properties) {
        static_cast<Backend*>(data)->addGlobal(id, permissions, type, version, properties);
    };
    events.global_remove = [](void* data, uint32_t id) { static_cast<Backend*>(data)->removeGlobal(id); };
    return events;
}();

const pw_node_events Backend::nodeEvents = [] {
    pw_node_events events{};
    events.version = PW_VERSION_NODE_EVENTS;
    events.info = [](void* data, const pw_node_info* info) {
        auto* binding = static_cast<Binding*>(data);
        // The object may be dropped by what its signals run.
        const std::shared_ptr<AudioObject> object = binding->object;
        Backend* backend = binding->backend;
        object->updateProperties(info->props);
        if (info->change_mask & PW_NODE_CHANGE_MASK_STATE) {
            switch (info->state) {
            case PW_NODE_STATE_RUNNING:
                object->m_state = AudioObject::RunningState;
                break;
            case PW_NODE_STATE_IDLE:
                object->m_state = AudioObject::IdleState;
                break;
            case PW_NODE_STATE_SUSPENDED:
                object->m_state = AudioObject::SuspendedState;
                break;
            default:
                object->m_state = AudioObject::UnknownState;
            }
        }
        if (info->change_mask & PW_NODE_CHANGE_MASK_PARAMS) {
            object->m_writable = false;
            for (uint32_t i = 0; i < info->n_params; ++i) {
                if (info->params[i].id == SPA_PARAM_Props) {
                    object->m_writable = (info->params[i].flags & SPA_PARAM_INFO_WRITE)
                                         && (binding->permissions & PW_PERM_W);
                }
            }
        }
        object->changed.emit();
        backend->updateDefaults();
    };
    events.param = [](void* data, int, uint32_t id, uint32_t, uint32_t, const spa_pod* param) {
        auto* binding = static_cast<Binding*>(data);
        // The card's cached route still holds the previous volume at this
        // point; applying it here would flash the old value.
        if (id == SPA_PARAM_Props)
            binding->object->updateVolume(param);
    };
    return events;
}();

const pw_device_events Backend::deviceEvents = [] {
    pw_device_events events{};
    events.version = PW_VERSION_DEVICE_EVENTS;
    events.info = [](void* data, const pw_device_info* info) {
        auto* binding = static_cast<Binding*>(data);
        binding->object->updateProperties(info->props);
        if (!(info->change_mask & PW_DEVICE_CHANGE_MASK_PARAMS))
            return;
        for (uint32_t i = 0; i < info->n_params; ++i) {
            const spa_param_info& param = info->params[i];
            const auto known = binding->paramFlags.find(param.id);
            if (known != binding->paramFlags.end() && known->second == param.flags)
                continue;
            binding->paramFlags[param.id] = param.flags;
            if (!(param.flags & SPA_PARAM_INFO_READ))
                continue;
            if (param.id == SPA_PARAM_EnumProfile)
                binding->object->m_profiles.clear();
            else if (param.id == SPA_PARAM_EnumRoute)
                binding->object->m_routes.clear();
            else if (param.id != SPA_PARAM_Route && param.id != SPA_PARAM_Profile)
                continue;
            pw_device_enum_params(reinterpret_cast<pw_device*>(binding->proxy), ++binding->enumSequence, param.id, 0,
                                  UINT32_MAX, nullptr);
        }
        binding->object->changed.emit();
    };
    events.param = [](void* data, int, uint32_t id, uint32_t, uint32_t, const spa_pod* param) {
        auto* binding = static_cast<Binding*>(data);
        if (!param)
            return;
        if (id == SPA_PARAM_EnumProfile || id == SPA_PARAM_Profile) {
            binding->object->updateProfile(param, id == SPA_PARAM_Profile);
        } else if (id == SPA_PARAM_EnumRoute || id == SPA_PARAM_Route) {
            const int device = binding->object->updateRoute(param, id == SPA_PARAM_Route);
            if (device < 0)
                return;
            // Only that device's nodes: the card's other routes may still hold
            // the previous volume of a node that has already changed.
            for (const auto& node : binding->backend->objects()) {
                if (node->kind() != AudioObject::Card && node->cardIndex() == binding->object->index()
                    && node->profileDevice() == device)
                    binding->backend->updateRouteVolume(node.get());
            }
        }
    };
    return events;
}();

const pw_metadata_events Backend::metadataEvents = [] {
    pw_metadata_events events{};
    events.version = PW_VERSION_METADATA_EVENTS;
    events.property = [](void* data, uint32_t subject, const char* key, const char*, const char* value) {
        auto* self = static_cast<Backend*>(data);
        if (subject != PW_ID_CORE)
            return 0;
        const std::string name = jsonName(value);
        if (!key || std::strcmp(key, "default.audio.sink") == 0)
            self->m_defaultSink = name;
        if (!key || std::strcmp(key, "default.audio.source") == 0)
            self->m_defaultSource = name;
        self->updateDefaults();
        return 0;
    };
    return events;
}();

// The last backend's loop and context, for the next one on the thread. A
// context's modules allocate on load what unloading them never frees, so a
// page that comes and goes would otherwise leak with each context.
struct SpareContext
{
    pw_loop* loop = nullptr;
    pw_context* context = nullptr;
};
static thread_local SpareContext s_spare;

std::shared_ptr<Backend> Backend::shared()
{
    static std::weak_ptr<Backend> instance;
    std::shared_ptr<Backend> backend = instance.lock();
    if (!backend) {
        backend = std::make_shared<Backend>();
        instance = backend;
    }
    return backend;
}

Backend::Backend()
    : m_eventLoop(base::EventLoop::current())
{
    // Once per process, never undone: the spare context outlives every backend.
    static std::once_flag initialized;
    std::call_once(initialized, [] { pw_init(nullptr, nullptr); });
    m_retry.setSingleShot(true);
    m_retry.onTimeout = [this] { connectToDaemon(); };
    m_failure.setSingleShot(true);
    m_failure.onTimeout = [this] { fail(); };
    // The first attempt right away; retries a second apart.
    m_retry.start(0ms);
    m_retry.setInterval(1s);
}

Backend::~Backend()
{
    m_retry.stop();
    m_failure.stop();
    disconnectFromDaemon();
    if (m_context && !s_spare.context) {
        s_spare = {std::exchange(m_loop, nullptr), std::exchange(m_context, nullptr)};
        return;
    }
    releaseContext();
}

std::vector<std::shared_ptr<AudioObject>> Backend::objects() const
{
    std::vector<std::shared_ptr<AudioObject>> result;
    for (const auto& [id, binding] : m_bindings)
        result.push_back(binding->object);
    return result;
}

std::shared_ptr<AudioObject> Backend::object(uint32_t id) const
{
    const auto found = m_bindings.find(id);
    return found == m_bindings.end() ? nullptr : found->second->object;
}

std::shared_ptr<AudioObject> Backend::defaultNode(AudioObject::Kind kind) const
{
    if (kind != AudioObject::Sink && kind != AudioObject::Source)
        return nullptr;
    const std::string& name = kind == AudioObject::Sink ? m_defaultSink : m_defaultSource;
    for (const auto& [id, binding] : m_bindings) {
        if (binding->object->kind() == kind && binding->object->name() == name)
            return binding->object;
    }
    return nullptr;
}

void Backend::connectToDaemon()
{
    if (m_core)
        return;
    m_failurePending = false;
    if (!m_loop && s_spare.context) {
        m_loop = std::exchange(s_spare.loop, nullptr);
        m_context = std::exchange(s_spare.context, nullptr);
    }
    // A context loads its modules and configuration when created; kept across
    // attempts, a retry while the daemon is away costs only the connect.
    if (!m_loop) {
        m_loop = pw_loop_new(nullptr);
        if (m_loop)
            pw_loop_enter(m_loop);
    }
    if (m_loop && !m_context)
        m_context = pw_context_new(m_loop, nullptr, 0);
    if (m_context) {
        m_core = pw_context_connect(
            m_context, pw_properties_new(PW_KEY_APP_NAME, "Cutefish Audio", PW_KEY_APP_ID, "org.lingmo.Audio", nullptr),
            0);
    }
    if (!m_core) {
        fail();
        return;
    }
    pw_core_add_listener(m_core, &m_coreListener, &coreEvents, this);
    m_registry = pw_core_get_registry(m_core, PW_VERSION_REGISTRY, 0);
    if (!m_registry) {
        fail();
        return;
    }
    pw_registry_add_listener(m_registry, &m_registryListener, &registryEvents, this);
    if (m_eventLoop)
        m_watch = m_eventLoop->watch(pw_loop_get_fd(m_loop), EPOLLIN, [this](uint32_t) { iterate(); });
}

void Backend::iterate()
{
    if (!m_loop || m_iterating)
        return;
    m_iterating = true;
    const int result = pw_loop_iterate(m_loop, 0);
    m_iterating = false;
    if (result < 0 && result != -EINTR) {
        // A loop that failed is not reused.
        disconnectFromDaemon();
        releaseContext();
        m_retry.start();
    }
}

void Backend::fail()
{
    disconnectFromDaemon();
    m_retry.start();
}

void Backend::disconnectFromDaemon()
{
    disconnecting.emit();
    if (m_eventLoop && m_watch) {
        m_eventLoop->unwatch(m_watch);
        m_watch = 0;
    }
    std::vector<uint32_t> ids;
    for (const auto& [id, binding] : m_bindings)
        ids.push_back(id);
    for (uint32_t id : ids)
        removeGlobal(id);
    releaseMetadata();
    if (m_registry) {
        spa_hook_remove(&m_registryListener);
        pw_proxy_destroy(reinterpret_cast<pw_proxy*>(m_registry));
        m_registry = nullptr;
    }
    if (m_core) {
        spa_hook_remove(&m_coreListener);
        pw_core_disconnect(m_core);
        m_core = nullptr;
    }
    m_links.clear();
    m_defaultSink.clear();
    m_defaultSource.clear();
    if (m_connected) {
        m_connected = false;
        connectedChanged.emit();
    }
    updateDefaults();
}

void Backend::releaseContext()
{
    if (m_context) {
        pw_context_destroy(m_context);
        m_context = nullptr;
    }
    if (m_loop) {
        pw_loop_leave(m_loop);
        pw_loop_destroy(m_loop);
        m_loop = nullptr;
    }
}

void Backend::releaseMetadata()
{
    if (m_metadata) {
        spa_hook_remove(&m_metadataListener);
        pw_proxy_destroy(reinterpret_cast<pw_proxy*>(m_metadata));
        m_metadata = nullptr;
    }
    m_metadataId = AudioObject::InvalidId;
    m_metadataPermissions = 0;
}

void Backend::addGlobal(uint32_t id, uint32_t permissions, const char* type, uint32_t version,
                        const spa_dict* properties)
{
    if (!properties)
        return;
    if (std::strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0) {
        const char* name = spa_dict_lookup(properties, PW_KEY_METADATA_NAME);
        if (m_metadata || !name || std::strcmp(name, "default") != 0)
            return;
        m_metadata = static_cast<pw_metadata*>(
            pw_registry_bind(m_registry, id, type, std::min(version, uint32_t(PW_VERSION_METADATA)), 0));
        if (m_metadata) {
            m_metadataId = id;
            m_metadataPermissions = permissions;
            pw_metadata_add_listener(m_metadata, &m_metadataListener, &metadataEvents, this);
        }
        return;
    }
    if (std::strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
        uint32_t outputId = 0;
        uint32_t inputId = 0;
        if (parseId(spa_dict_lookup(properties, PW_KEY_LINK_OUTPUT_NODE), outputId)
            && parseId(spa_dict_lookup(properties, PW_KEY_LINK_INPUT_NODE), inputId)) {
            m_links[id] = {outputId, inputId};
            updateLinks();
        }
        return;
    }
    const char* mediaClass = spa_dict_lookup(properties, PW_KEY_MEDIA_CLASS);
    if (!mediaClass || m_bindings.contains(id))
        return;
    const char* nodeName = spa_dict_lookup(properties, PW_KEY_NODE_NAME);
    if (nodeName && std::string_view(nodeName).starts_with("lingmo.internal."))
        return;
    // WirePlumber's "Dummy Output", which stands in while no real sink exists.
    if (nodeName && std::strcmp(nodeName, "auto_null") == 0)
        return;
    AudioObject::Kind kind;
    uint32_t bindVersion;
    if (std::strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        if (std::strcmp(mediaClass, "Audio/Sink") == 0)
            kind = AudioObject::Sink;
        else if (std::strcmp(mediaClass, "Audio/Source") == 0 || std::strcmp(mediaClass, "Audio/Source/Virtual") == 0)
            kind = AudioObject::Source;
        else if (std::strcmp(mediaClass, "Stream/Output/Audio") == 0)
            kind = AudioObject::Playback;
        else if (std::strcmp(mediaClass, "Stream/Input/Audio") == 0)
            kind = AudioObject::Capture;
        else
            return;
        bindVersion = PW_VERSION_NODE;
    } else if (std::strcmp(type, PW_TYPE_INTERFACE_Device) == 0 && std::strcmp(mediaClass, "Audio/Device") == 0) {
        kind = AudioObject::Card;
        bindVersion = PW_VERSION_DEVICE;
    } else {
        return;
    }
    auto* proxy = static_cast<pw_proxy*>(pw_registry_bind(m_registry, id, type, std::min(version, bindVersion), 0));
    if (!proxy)
        return;
    auto object = std::make_shared<AudioObject>(this, id, kind);
    object->updateProperties(properties);
    auto binding = std::make_unique<Binding>();
    binding->backend = this;
    binding->object = object;
    binding->proxy = proxy;
    binding->permissions = permissions;
    Binding* added = binding.get();
    m_bindings[id] = std::move(binding);
    if (kind == AudioObject::Card) {
        pw_device_add_listener(reinterpret_cast<pw_device*>(proxy), &added->listener, &deviceEvents, added);
        added->cardChanged = object->changed.connect([this, id] {
            for (const auto& node : objects()) {
                if (node->kind() != AudioObject::Card && node->cardIndex() == id)
                    node->changed.emit();
            }
        });
    } else {
        pw_node_add_listener(reinterpret_cast<pw_node*>(proxy), &added->listener, &nodeEvents, added);
        uint32_t params[] = {SPA_PARAM_Props};
        pw_node_subscribe_params(reinterpret_cast<pw_node*>(proxy), params, 1);
    }
    objectAdded.emit(object);
    updateDefaults();
    updateLinks();
}

void Backend::removeGlobal(uint32_t id)
{
    if (id == m_metadataId && m_metadata) {
        releaseMetadata();
        m_defaultSink.clear();
        m_defaultSource.clear();
        updateDefaults();
    }
    if (const auto found = m_bindings.find(id); found != m_bindings.end()) {
        std::unique_ptr<Binding> binding = std::move(found->second);
        m_bindings.erase(found);
        spa_hook_remove(&binding->listener);
        pw_proxy_destroy(binding->proxy);
        updateDefaults();
        objectRemoved.emit(binding->object);
        binding->object->m_backend = nullptr;
        for (const auto& node : objects()) {
            if (node->cardIndex() == id)
                node->changed.emit();
        }
    }
    m_links.erase(id);
    updateLinks();
}

void Backend::updateDefaults()
{
    defaultsChanged.emit();
    for (const auto& object : objects()) {
        if (object->kind() == AudioObject::Sink || object->kind() == AudioObject::Source)
            object->changed.emit();
    }
}

void Backend::updateLinks()
{
    for (const auto& stream : objects()) {
        if (stream->kind() != AudioObject::Playback && stream->kind() != AudioObject::Capture)
            continue;
        uint32_t target = AudioObject::InvalidId;
        const bool playback = stream->kind() == AudioObject::Playback;
        for (const auto& [id, link] : m_links) {
            if ((playback ? link.first : link.second) != stream->index())
                continue;
            const std::shared_ptr<AudioObject> node = object(playback ? link.second : link.first);
            if (node && node->kind() == (playback ? AudioObject::Sink : AudioObject::Source)) {
                target = node->index();
                break;
            }
        }
        if (stream->m_deviceId != target) {
            stream->m_deviceId = target;
            stream->changed.emit();
        }
    }
}

void Backend::checkResult(int result, const char* operation)
{
    if (result < 0)
        operationFailed.emit(
            // TRANSLATORS: a sound setting that failed, then why, such as "Set device volume: Permission denied".
            format(tr("{operation}: {error}"), {{"operation", tr(operation)}, {"error", std::strerror(-result)}}));
}

void Backend::updateRouteVolume(AudioObject* node)
{
    const std::shared_ptr<AudioObject> card = object(node->cardIndex());
    if (!card || card->kind() != AudioObject::Card)
        return;
    const auto props = card->m_routeProperties.find(node->profileDevice());
    if (props != card->m_routeProperties.end() && !props->second.empty())
        node->updateVolume(reinterpret_cast<const spa_pod*>(props->second.data()));
}

void Backend::setNodeParam(AudioObject* node, const spa_pod* param)
{
    const auto found = m_bindings.find(node->index());
    if (found == m_bindings.end() || found->second->object.get() != node || !node->m_writable
        || node->kind() == AudioObject::Card)
        return;
    const auto cardBinding = m_bindings.find(node->cardIndex());
    const int device = node->profileDevice();
    if (cardBinding != m_bindings.end()) {
        Binding* card = cardBinding->second.get();
        const auto route = card->object->m_activeRoutes.find(device);
        if (card->object->m_routeProperties.contains(device) && route != card->object->m_activeRoutes.end()) {
            if (!(card->permissions & PW_PERM_W)) {
                checkResult(-EACCES, trNoop("Set device volume"));
                return;
            }
            uint8_t buffer[2048];
            spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
            const auto* routeParam = static_cast<spa_pod*>(spa_pod_builder_add_object(
                &builder, SPA_TYPE_OBJECT_ParamRoute, SPA_PARAM_Route, SPA_PARAM_ROUTE_index,
                SPA_POD_Int(route->second), SPA_PARAM_ROUTE_device, SPA_POD_Int(device), SPA_PARAM_ROUTE_props,
                SPA_POD_Pod(param), SPA_PARAM_ROUTE_save, SPA_POD_Bool(true)));
            checkResult(pw_device_set_param(reinterpret_cast<pw_device*>(card->proxy), SPA_PARAM_Route, 0, routeParam),
                        trNoop("Set device volume"));
            return;
        }
    }
    checkResult(pw_node_set_param(reinterpret_cast<pw_node*>(found->second->proxy), SPA_PARAM_Props, 0, param),
                trNoop("Set audio properties"));
}

void Backend::setVolume(AudioObject* node, const std::vector<float>& volumes)
{
    if (volumes.empty() || volumes.size() > SPA_AUDIO_MAX_CHANNELS)
        return;
    uint8_t buffer[1024];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const spa_pod* param;
    if (node->m_channelVolume) {
        param = static_cast<spa_pod*>(spa_pod_builder_add_object(
            &builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_channelVolumes,
            SPA_POD_Array(sizeof(float), SPA_TYPE_Float, volumes.size(), volumes.data())));
    } else {
        param = static_cast<spa_pod*>(spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                                                                 SPA_PROP_volume, SPA_POD_Float(volumes.front())));
    }
    setNodeParam(node, param);
}

void Backend::setMute(AudioObject* node, bool mute)
{
    uint8_t buffer[128];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const auto* param = static_cast<spa_pod*>(
        spa_pod_builder_add_object(&builder, SPA_TYPE_OBJECT_Props, SPA_PARAM_Props, SPA_PROP_mute, SPA_POD_Bool(mute)));
    setNodeParam(node, param);
}

void Backend::setDefault(AudioObject* node, bool enable)
{
    if (!m_metadata || !(m_metadataPermissions & PW_PERM_W) || object(node->index()).get() != node)
        return;
    const char* key = node->kind() == AudioObject::Sink ? "default.configured.audio.sink"
                                                        : "default.configured.audio.source";
    const std::string value = jsonNameDocument(node->name());
    if (enable || node->isDefault())
        checkResult(pw_metadata_set_property(m_metadata, PW_ID_CORE, key, "Spa:String:JSON",
                                             enable ? value.c_str() : nullptr),
                    trNoop("Set default device"));
}

void Backend::setProfile(AudioObject* card, int profile)
{
    const auto found = m_bindings.find(card->index());
    if (found == m_bindings.end() || card->kind() != AudioObject::Card || !(found->second->permissions & PW_PERM_W))
        return;
    uint8_t buffer[128];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const auto* param = static_cast<spa_pod*>(spa_pod_builder_add_object(
        &builder, SPA_TYPE_OBJECT_ParamProfile, SPA_PARAM_Profile, SPA_PARAM_PROFILE_index, SPA_POD_Int(profile),
        SPA_PARAM_PROFILE_save, SPA_POD_Bool(true)));
    checkResult(pw_device_set_param(reinterpret_cast<pw_device*>(found->second->proxy), SPA_PARAM_Profile, 0, param),
                trNoop("Set device profile"));
}

void Backend::setRoute(AudioObject* node, int route)
{
    const auto found = m_bindings.find(node->cardIndex());
    if (found == m_bindings.end() || found->second->object->kind() != AudioObject::Card || node->profileDevice() < 0
        || !(found->second->permissions & PW_PERM_W))
        return;
    uint8_t buffer[256];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const auto* param = static_cast<spa_pod*>(spa_pod_builder_add_object(
        &builder, SPA_TYPE_OBJECT_ParamRoute, SPA_PARAM_Route, SPA_PARAM_ROUTE_index, SPA_POD_Int(route),
        SPA_PARAM_ROUTE_device, SPA_POD_Int(node->profileDevice()), SPA_PARAM_ROUTE_save, SPA_POD_Bool(true)));
    checkResult(pw_device_set_param(reinterpret_cast<pw_device*>(found->second->proxy), SPA_PARAM_Route, 0, param),
                trNoop("Set device route"));
}

void Backend::moveStream(AudioObject* stream, uint32_t target)
{
    const std::shared_ptr<AudioObject> node = object(target);
    if (!node || !m_metadata || !(m_metadataPermissions & PW_PERM_W) || object(stream->index()).get() != stream
        || (stream->kind() != AudioObject::Playback && stream->kind() != AudioObject::Capture)
        || node->kind() != (stream->kind() == AudioObject::Playback ? AudioObject::Sink : AudioObject::Source))
        return;
    const std::string serial = node->property(PW_KEY_OBJECT_SERIAL);
    if (!serial.empty())
        checkResult(pw_metadata_set_property(m_metadata, stream->index(), "target.object", "Spa:Id", serial.c_str()),
                    trNoop("Move audio stream"));
}

} // namespace Lingmo::Audio
