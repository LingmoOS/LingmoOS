// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <pipewire/pipewire.h>
#include <spa/monitor/utils.h>
#include <spa/param/audio/raw.h>
#include <spa/param/profile.h>
#include <spa/param/props.h>
#include <spa/param/route.h>
#include <spa/pod/iter.h>
#include <spa/pod/parser.h>

class FakeAudioDevice
{
public:
    FakeAudioDevice()
    {
        spa_hook_list_init(&m_hooks);
        m_device.iface = SPA_INTERFACE_INIT(SPA_TYPE_INTERFACE_Device, SPA_VERSION_DEVICE, &methods, this);
        m_params[0] = SPA_PARAM_INFO(SPA_PARAM_EnumProfile, SPA_PARAM_INFO_READ);
        m_params[1] = SPA_PARAM_INFO(SPA_PARAM_Profile, SPA_PARAM_INFO_READWRITE);
        m_params[2] = SPA_PARAM_INFO(SPA_PARAM_EnumRoute, SPA_PARAM_INFO_READ);
        m_params[3] = SPA_PARAM_INFO(SPA_PARAM_Route, SPA_PARAM_INFO_READWRITE);
    }

    ~FakeAudioDevice()
    {
        if (m_proxy)
            pw_proxy_destroy(m_proxy);
        if (m_core)
            pw_core_disconnect(m_core);
        spa_hook_list_clean(&m_hooks);
    }

    bool exportTo(pw_core *core)
    {
        // Device enumeration can pause the requesting client until the owner replies.
        m_core = pw_context_connect(pw_core_get_context(core), nullptr, 0);
        if (!m_core)
            return false;
        auto *properties = pw_properties_new(PW_KEY_DEVICE_NAME, "test-card",
            PW_KEY_DEVICE_DESCRIPTION, "Test Audio Card", PW_KEY_MEDIA_CLASS, "Audio/Device", nullptr);
        m_proxy = static_cast<pw_proxy *>(pw_core_export(m_core, SPA_TYPE_INTERFACE_Device,
            &properties->dict, &m_device, 0));
        pw_properties_free(properties);
        return m_proxy != nullptr;
    }

    void notify()
    {
        m_params[1].flags ^= SPA_PARAM_INFO_SERIAL;
        m_params[3].flags ^= SPA_PARAM_INFO_SERIAL;
        const spa_device_info info {SPA_VERSION_DEVICE_INFO, SPA_DEVICE_CHANGE_MASK_PARAMS,
                                   0, nullptr, m_params, 4};
        spa_device_emit_info(&m_hooks, &info);
    }

    int profile = 7;
    int route = 9;
    float volumes[2] {1.0f, 1.0f};
    bool muted = false;
    bool saved = false;
    int writes = 0;

private:
    int enumerate(int seq, uint32_t id, uint32_t start, uint32_t maximum)
    {
        const uint32_t count = id == SPA_PARAM_EnumProfile || id == SPA_PARAM_EnumRoute ? 2 : 1;
        for (uint32_t i = start; i < count && i - start < maximum; ++i) {
            uint8_t buffer[2048];
            spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
            spa_pod *param = nullptr;
            if (id == SPA_PARAM_EnumProfile || id == SPA_PARAM_Profile) {
                const int index = id == SPA_PARAM_Profile ? profile : 7 + int(i);
                param = static_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                    SPA_TYPE_OBJECT_ParamProfile, id,
                    SPA_PARAM_PROFILE_index, SPA_POD_Int(index),
                    SPA_PARAM_PROFILE_name, SPA_POD_String(index == 7 ? "stereo" : "surround"),
                    SPA_PARAM_PROFILE_description, SPA_POD_String(index == 7 ? "Stereo" : "Surround"),
                    SPA_PARAM_PROFILE_priority, SPA_POD_Int(index == 7 ? 100 : 50),
                    SPA_PARAM_PROFILE_available, SPA_POD_Id(SPA_PARAM_AVAILABILITY_yes)));
            } else if (id == SPA_PARAM_EnumRoute || id == SPA_PARAM_Route) {
                const int index = id == SPA_PARAM_Route ? route : 9 + int(i);
                const uint32_t channels[] = {SPA_AUDIO_CHANNEL_FL, SPA_AUDIO_CHANNEL_FR};
                const auto *props = static_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                    SPA_TYPE_OBJECT_Props, SPA_PARAM_Props,
                    SPA_PROP_channelVolumes, SPA_POD_Array(sizeof(float), SPA_TYPE_Float, 2, volumes),
                    SPA_PROP_channelMap, SPA_POD_Array(sizeof(uint32_t), SPA_TYPE_Id, 2, channels),
                    SPA_PROP_mute, SPA_POD_Bool(muted)));
                const int devices[] = {3};
                const int profiles[] = {7, 8};
                param = static_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                    SPA_TYPE_OBJECT_ParamRoute, id,
                    SPA_PARAM_ROUTE_index, SPA_POD_Int(index),
                    SPA_PARAM_ROUTE_device, SPA_POD_Int(3),
                    SPA_PARAM_ROUTE_name, SPA_POD_String(index == 9 ? "speakers" : "headphones"),
                    SPA_PARAM_ROUTE_description, SPA_POD_String(index == 9 ? "Speakers" : "Headphones"),
                    SPA_PARAM_ROUTE_direction, SPA_POD_Id(SPA_DIRECTION_OUTPUT),
                    SPA_PARAM_ROUTE_priority, SPA_POD_Int(index == 9 ? 100 : 50),
                    SPA_PARAM_ROUTE_available, SPA_POD_Id(SPA_PARAM_AVAILABILITY_yes),
                    SPA_PARAM_ROUTE_devices, SPA_POD_Array(sizeof(int), SPA_TYPE_Int, 1, devices),
                    SPA_PARAM_ROUTE_profiles, SPA_POD_Array(sizeof(int), SPA_TYPE_Int, 2, profiles),
                    SPA_PARAM_ROUTE_props, SPA_POD_Pod(props)));
            } else {
                return -ENOENT;
            }
            const spa_result_device_params result {id, i, i + 1, param};
            spa_device_emit_result(&m_hooks, seq, 0, SPA_RESULT_TYPE_DEVICE_PARAMS, &result);
        }
        return 0;
    }

    int setParam(uint32_t id, const spa_pod *param)
    {
        if (id == SPA_PARAM_Profile) {
            if (spa_pod_parse_object(param, SPA_TYPE_OBJECT_ParamProfile, nullptr,
                    SPA_PARAM_PROFILE_index, SPA_POD_Int(&profile),
                    SPA_PARAM_PROFILE_save, SPA_POD_OPT_Bool(&saved)) < 0)
                return -EINVAL;
        } else if (id == SPA_PARAM_Route) {
            int device = -1;
            spa_pod *props = nullptr;
            if (spa_pod_parse_object(param, SPA_TYPE_OBJECT_ParamRoute, nullptr,
                    SPA_PARAM_ROUTE_index, SPA_POD_Int(&route),
                    SPA_PARAM_ROUTE_device, SPA_POD_Int(&device),
                    SPA_PARAM_ROUTE_props, SPA_POD_OPT_Pod(&props),
                    SPA_PARAM_ROUTE_save, SPA_POD_OPT_Bool(&saved)) < 0 || device != 3)
                return -EINVAL;
            if (props) {
                const spa_pod_prop *property = spa_pod_find_prop(props, nullptr, SPA_PROP_channelVolumes);
                if (property)
                    spa_pod_copy_array(&property->value, SPA_TYPE_Float, volumes, 2);
                property = spa_pod_find_prop(props, nullptr, SPA_PROP_mute);
                if (property)
                    spa_pod_get_bool(&property->value, &muted);
            }
        } else {
            return -ENOENT;
        }
        ++writes;
        notify();
        return 0;
    }

    spa_device m_device {};
    spa_hook_list m_hooks {};
    spa_param_info m_params[4] {};
    pw_proxy *m_proxy = nullptr;
    pw_core *m_core = nullptr;

    static const spa_device_methods methods;

};

inline const spa_device_methods FakeAudioDevice::methods = [] {
    spa_device_methods methods {};
    methods.version = SPA_VERSION_DEVICE_METHODS;
    methods.add_listener = [](void *object, spa_hook *listener, const spa_device_events *events, void *data) {
        auto *self = static_cast<FakeAudioDevice *>(object);
        spa_hook_list_append(&self->m_hooks, listener, events, data);
        const spa_device_info info {SPA_VERSION_DEVICE_INFO, SPA_DEVICE_CHANGE_MASK_PARAMS,
                                   0, nullptr, self->m_params, 4};
        if (events->info)
            events->info(data, &info);
        return 0;
    };
    methods.sync = [](void *object, int seq) {
        auto *self = static_cast<FakeAudioDevice *>(object);
        spa_device_emit_result(&self->m_hooks, seq, 0, 0, nullptr);
        return 0;
    };
    methods.enum_params = [](void *object, int seq, uint32_t id, uint32_t start, uint32_t count, const spa_pod *) {
        return static_cast<FakeAudioDevice *>(object)->enumerate(seq, id, start, count);
    };
    methods.set_param = [](void *object, uint32_t id, uint32_t, const spa_pod *param) {
        return static_cast<FakeAudioDevice *>(object)->setParam(id, param);
    };
    return methods;
}();
