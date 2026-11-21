// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/audio/audio_object.h"

#include <algorithm>
#include <charconv>
#include <cmath>

#include <spa/debug/types.h>
#include <spa/param/audio/raw-types.h>
#include <spa/param/profile.h>
#include <spa/param/props.h>
#include <spa/param/route.h>
#include <spa/pod/parser.h>
#include <spa/pod/iter.h>

#include "lingmo/audio/backend.h"

namespace Lingmo::Audio {

template<typename T>
static std::vector<T> arrayValues(const spa_pod* pod, uint32_t type)
{
    if (!spa_pod_is_array(pod) || SPA_POD_ARRAY_VALUE_TYPE(pod) != type || SPA_POD_ARRAY_VALUE_SIZE(pod) != sizeof(T))
        return {};
    const auto* values = static_cast<const T*>(SPA_POD_ARRAY_VALUES(pod));
    const auto count = std::min<uint32_t>(SPA_POD_ARRAY_N_VALUES(pod), SPA_AUDIO_MAX_CHANNELS);
    return std::vector<T>(values, values + count);
}

static std::vector<int> intValues(const spa_pod* object, uint32_t key)
{
    const spa_pod_prop* prop = spa_pod_find_prop(object, nullptr, key);
    return prop ? arrayValues<int>(&prop->value, SPA_TYPE_Int) : std::vector<int>();
}

static int availability(uint32_t value)
{
    if (value == SPA_PARAM_AVAILABILITY_yes)
        return AudioObject::Available;
    if (value == SPA_PARAM_AVAILABILITY_no)
        return AudioObject::Unavailable;
    return AudioObject::Unknown;
}

static void updateChoice(std::vector<AudioObject::Choice>& list, AudioObject::Choice choice)
{
    for (AudioObject::Choice& item : list) {
        if (item.index == choice.index) {
            item = std::move(choice);
            return;
        }
    }
    list.push_back(std::move(choice));
}

// A whole number that fills the text: digits, with a leading '-' for a signed type.
template<typename T>
static bool parseNumber(const std::string& text, T& value)
{
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(text.data(), end, value);
    return !text.empty() && result.ec == std::errc() && result.ptr == end;
}

AudioObject::AudioObject(Backend* backend, uint32_t id, Kind kind)
    : m_backend(backend)
    , m_id(id)
    , m_kind(kind)
{
}

std::string AudioObject::property(const std::string& key) const
{
    const auto found = m_properties.find(key);
    return found == m_properties.end() ? std::string() : found->second;
}

std::string AudioObject::name() const
{
    return property(m_kind == Card ? "device.name" : "node.name");
}

std::string AudioObject::description() const
{
    static const char* const cardKeys[] = {"device.description", "device.nick", "device.name"};
    static const char* const nodeKeys[] = {"node.description", "node.nick", "media.name", "application.name",
                                           "node.name"};
    if (m_kind == Card) {
        for (const char* key : cardKeys) {
            if (const std::string value = property(key); !value.empty())
                return value;
        }
    } else {
        for (const char* key : nodeKeys) {
            if (const std::string value = property(key); !value.empty())
                return value;
        }
    }
    return name();
}

std::string AudioObject::displayName() const
{
    const std::string cardBus = bus();
    if ((m_kind == Sink || m_kind == Source) && !isVirtualDevice() && cardBus != "usb" && cardBus != "bluetooth") {
        const std::vector<Choice> routes = ports();
        const int active = activePortIndex();
        if (active >= 0 && active < int(routes.size()) && !routes[active].description.empty())
            return routes[active].description;
    }
    return description();
}

std::string AudioObject::iconName() const
{
    if (m_properties.contains("device.icon-name"))
        return property("device.icon-name");
    if (m_properties.contains("application.icon-name"))
        return property("application.icon-name");
    return m_kind == Source || m_kind == Capture ? "audio-input-microphone" : "audio-card";
}

double AudioObject::fromLinear(float volume)
{
    return std::isfinite(volume) ? std::cbrt(std::max(0.0f, volume)) : 0.0;
}

float AudioObject::toLinear(double volume)
{
    if (!std::isfinite(volume))
        return 0.0f;
    const double bounded = std::clamp(volume, 0.0, MaximalVolume);
    return static_cast<float>(bounded * bounded * bounded);
}

double AudioObject::volume() const
{
    return m_volumes.empty() ? 0.0 : fromLinear(*std::max_element(m_volumes.begin(), m_volumes.end()));
}

std::vector<double> AudioObject::channelVolumes() const
{
    std::vector<double> values;
    for (float volume : m_volumes)
        values.push_back(fromLinear(volume));
    return values;
}

std::vector<std::string> AudioObject::channels() const
{
    std::vector<std::string> names;
    for (uint32_t channel : m_channels) {
        const char* name = spa_debug_type_find_short_name(spa_type_audio_channel, channel);
        names.push_back(name ? name : "UNKNOWN");
    }
    return names;
}

bool AudioObject::isDefault() const
{
    return m_backend && m_backend->defaultNode(m_kind).get() == this;
}

bool AudioObject::isVirtualDevice() const
{
    return property("node.virtual") == "true" || !m_properties.contains("device.id");
}

uint32_t AudioObject::cardIndex() const
{
    uint32_t id = 0;
    return parseNumber(property("device.id"), id) ? id : InvalidId;
}

const AudioObject* AudioObject::cardObject() const
{
    return m_backend ? m_backend->object(cardIndex()).get() : nullptr;
}

std::string AudioObject::bus() const
{
    const AudioObject* card = cardObject();
    return card ? card->property("device.bus") : std::string();
}

int AudioObject::profileDevice() const
{
    int id = 0;
    return parseNumber(property("card.profile.device"), id) ? id : -1;
}

std::vector<AudioObject::Choice> AudioObject::ports() const
{
    const AudioObject* card = cardObject();
    if (!card || (m_kind != Sink && m_kind != Source))
        return {};
    std::vector<Choice> result;
    const int device = profileDevice();
    for (const Choice& route : card->m_routes) {
        if (route.direction != (m_kind == Sink ? uint32_t(SPA_DIRECTION_OUTPUT) : uint32_t(SPA_DIRECTION_INPUT)))
            continue;
        if (!route.devices.empty() && std::find(route.devices.begin(), route.devices.end(), device) == route.devices.end())
            continue;
        if (!route.profiles.empty()
            && std::find(route.profiles.begin(), route.profiles.end(), card->m_profile) == route.profiles.end())
            continue;
        result.push_back(route);
    }
    return result;
}

int AudioObject::activePortIndex() const
{
    const AudioObject* card = cardObject();
    if (!card)
        return -1;
    const auto found = card->m_activeRoutes.find(profileDevice());
    const int active = found == card->m_activeRoutes.end() ? -1 : found->second;
    const std::vector<Choice> routes = ports();
    for (int i = 0; i < int(routes.size()); ++i) {
        if (routes[i].index == active)
            return i;
    }
    return -1;
}

int AudioObject::activeProfileIndex() const
{
    for (int i = 0; i < int(m_profiles.size()); ++i) {
        if (m_profiles[i].index == m_profile)
            return i;
    }
    return -1;
}

void AudioObject::setVolume(double volume)
{
    if (!m_backend || !volumeWritable() || !std::isfinite(volume))
        return;
    const float value = toLinear(volume);
    const float maximum = *std::max_element(m_volumes.begin(), m_volumes.end());
    std::vector<float> volumes = m_volumes;
    for (float& channel : volumes)
        channel = maximum > 0.0f ? channel / maximum * value : value;
    m_backend->setVolume(this, volumes);
}

void AudioObject::setMuted(bool muted)
{
    if (m_backend && m_writable)
        m_backend->setMute(this, muted);
}

void AudioObject::setChannelVolumes(const std::vector<double>& values)
{
    if (!m_backend || !volumeWritable() || values.size() != m_volumes.size())
        return;
    std::vector<float> volumes;
    for (double volume : values) {
        if (!std::isfinite(volume))
            return;
        volumes.push_back(toLinear(volume));
    }
    m_backend->setVolume(this, volumes);
}

void AudioObject::setChannelVolume(int channel, double volume)
{
    if (channel < 0 || channel >= int(m_volumes.size()) || !std::isfinite(volume))
        return;
    std::vector<double> values = channelVolumes();
    values[channel] = volume;
    setChannelVolumes(values);
}

void AudioObject::setDefault(bool enable)
{
    if (m_backend && (m_kind == Sink || m_kind == Source))
        m_backend->setDefault(this, enable);
}

void AudioObject::setActivePortIndex(int index)
{
    const std::vector<Choice> routes = ports();
    if (m_backend && index >= 0 && index < int(routes.size()) && routes[index].availability != Unavailable)
        m_backend->setRoute(this, routes[index].index);
}

void AudioObject::setActiveProfileIndex(int index)
{
    if (m_backend && m_kind == Card && index >= 0 && index < int(m_profiles.size())
        && m_profiles[index].availability != Unavailable)
        m_backend->setProfile(this, m_profiles[index].index);
}

void AudioObject::setDeviceIndex(uint32_t id)
{
    if (m_backend && (m_kind == Playback || m_kind == Capture))
        m_backend->moveStream(this, id);
}

void AudioObject::switchStreams()
{
    if (!m_backend || (m_kind != Sink && m_kind != Source))
        return;
    for (const auto& object : m_backend->objects()) {
        if (object->kind() == (m_kind == Sink ? Playback : Capture))
            object->setDeviceIndex(m_id);
    }
}

void AudioObject::updateProperties(const spa_dict* properties)
{
    if (!properties)
        return;
    const spa_dict_item* item;
    spa_dict_for_each(item, properties)
    {
        if (item->value)
            m_properties.insert_or_assign(item->key, item->value);
        else
            m_properties.erase(item->key);
    }
    changed.emit();
}

void AudioObject::updateVolume(const spa_pod* param)
{
    if (!param || !spa_pod_is_object(param))
        return;
    const spa_pod_prop* prop;
    if ((prop = spa_pod_find_prop(param, nullptr, SPA_PROP_channelVolumes))) {
        const std::vector<float> volumes = arrayValues<float>(&prop->value, SPA_TYPE_Float);
        if (!volumes.empty()) {
            m_volumes = volumes;
            for (float& value : m_volumes)
                value = std::isfinite(value) ? std::max(0.0f, value) : 0.0f;
            m_channelVolume = true;
        }
    }
    if (!m_channelVolume && (prop = spa_pod_find_prop(param, nullptr, SPA_PROP_volume))) {
        float volume = 0.0f;
        if (spa_pod_get_float(&prop->value, &volume) == 0 && std::isfinite(volume))
            m_volumes = {std::max(0.0f, volume)};
    }
    if ((prop = spa_pod_find_prop(param, nullptr, SPA_PROP_mute)))
        spa_pod_get_bool(&prop->value, &m_muted);
    if ((prop = spa_pod_find_prop(param, nullptr, SPA_PROP_channelMap)))
        m_channels = arrayValues<uint32_t>(&prop->value, SPA_TYPE_Id);
    changed.emit();
}

void AudioObject::updateProfile(const spa_pod* param, bool current)
{
    int index = -1;
    int priority = 0;
    uint32_t available = SPA_PARAM_AVAILABILITY_unknown;
    const char* name = "";
    const char* description = "";
    if (spa_pod_parse_object(param, SPA_TYPE_OBJECT_ParamProfile, nullptr, SPA_PARAM_PROFILE_index, SPA_POD_Int(&index),
                             SPA_PARAM_PROFILE_name, SPA_POD_OPT_String(&name), SPA_PARAM_PROFILE_description,
                             SPA_POD_OPT_String(&description), SPA_PARAM_PROFILE_priority, SPA_POD_OPT_Int(&priority),
                             SPA_PARAM_PROFILE_available, SPA_POD_OPT_Id(&available))
        < 0)
        return;
    if (current) {
        if (m_profile != index) {
            m_activeRoutes.clear();
            m_routeProperties.clear();
        }
        m_profile = index;
    } else {
        Choice choice;
        choice.index = index;
        choice.name = name ? name : "";
        choice.description = description ? description : "";
        choice.priority = priority;
        choice.availability = availability(available);
        updateChoice(m_profiles, std::move(choice));
    }
    changed.emit();
}

int AudioObject::updateRoute(const spa_pod* param, bool current)
{
    int index = -1;
    int device = -1;
    int priority = 0;
    uint32_t direction = SPA_DIRECTION_OUTPUT;
    uint32_t available = SPA_PARAM_AVAILABILITY_unknown;
    const char* name = "";
    const char* description = "";
    if (spa_pod_parse_object(param, SPA_TYPE_OBJECT_ParamRoute, nullptr, SPA_PARAM_ROUTE_index, SPA_POD_Int(&index),
                             SPA_PARAM_ROUTE_device, SPA_POD_OPT_Int(&device), SPA_PARAM_ROUTE_direction,
                             SPA_POD_OPT_Id(&direction), SPA_PARAM_ROUTE_name, SPA_POD_OPT_String(&name),
                             SPA_PARAM_ROUTE_description, SPA_POD_OPT_String(&description),
                             SPA_PARAM_ROUTE_priority, SPA_POD_OPT_Int(&priority), SPA_PARAM_ROUTE_available,
                             SPA_POD_OPT_Id(&available))
        < 0)
        return -1;
    if (current) {
        m_activeRoutes[device] = index;
        const spa_pod_prop* props = spa_pod_find_prop(param, nullptr, SPA_PARAM_ROUTE_props);
        if (props && spa_pod_is_object(&props->value)) {
            const auto* bytes = reinterpret_cast<const uint8_t*>(&props->value);
            m_routeProperties[device] = std::vector<uint8_t>(bytes, bytes + SPA_POD_SIZE(&props->value));
        }
    } else {
        Choice choice;
        choice.index = index;
        choice.name = name ? name : "";
        choice.description = description ? description : "";
        choice.priority = priority;
        choice.availability = availability(available);
        choice.direction = direction;
        choice.devices = intValues(param, SPA_PARAM_ROUTE_devices);
        choice.profiles = intValues(param, SPA_PARAM_ROUTE_profiles);
        updateChoice(m_routes, std::move(choice));
    }
    changed.emit();
    return current ? device : -1;
}

} // namespace Lingmo::Audio
