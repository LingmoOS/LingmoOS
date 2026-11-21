// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/drm/drm_properties.h"

#include <xf86drmMode.h>

namespace Lingmo::Backend {

DrmPropertyMap DrmPropertyMap::fromObject(int fd, uint32_t objectId, uint32_t objectType)
{
    DrmPropertyMap map;

    drmModeObjectProperties* props = drmModeObjectGetProperties(fd, objectId, objectType);
    if (!props)
        return map;

    for (uint32_t i = 0; i < props->count_props; ++i) {
        drmModePropertyRes* prop = drmModeGetProperty(fd, props->props[i]);
        if (!prop)
            continue;
        map.insert(prop->name, prop->prop_id, props->prop_values[i]);
        drmModeFreeProperty(prop);
    }

    drmModeFreeObjectProperties(props);
    return map;
}

void DrmPropertyMap::insert(const std::string& name, uint32_t id, uint64_t value)
{
    m_properties.insert_or_assign(name, Property{id, value});
}

std::optional<DrmPropertyMap::Property> DrmPropertyMap::find(const std::string& name) const
{
    const auto it = m_properties.find(name);
    if (it == m_properties.end())
        return std::nullopt;
    return it->second;
}

uint32_t DrmPropertyMap::id(const std::string& name) const
{
    const auto it = m_properties.find(name);
    return it == m_properties.end() ? 0 : it->second.id;
}

uint64_t DrmPropertyMap::value(const std::string& name, uint64_t fallback) const
{
    const auto it = m_properties.find(name);
    return it == m_properties.end() ? fallback : it->second.value;
}

std::vector<std::string> DrmPropertyMap::missing(const std::vector<std::string>& required) const
{
    std::vector<std::string> result;
    for (const std::string& name : required) {
        if (!m_properties.contains(name))
            result.push_back(name);
    }
    return result;
}

} // namespace Lingmo::Backend
