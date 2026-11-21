// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_DRM_DRM_PROPERTIES_H_
#define LINGMO_BACKEND_DRM_DRM_PROPERTIES_H_

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Lingmo::Backend {

// Property ids are per-object and driver-assigned, so every atomic property has
// to be looked up by name before it can be set.
class DrmPropertyMap
{
public:
    struct Property {
        uint32_t id = 0;
        uint64_t value = 0;
    };

    DrmPropertyMap() = default;

    // Reads all properties of a DRM object. `objectType` is one of
    // DRM_MODE_OBJECT_*. Returns an empty map on failure.
    static DrmPropertyMap fromObject(int fd, uint32_t objectId, uint32_t objectType);

    void insert(const std::string& name, uint32_t id, uint64_t value);

    bool contains(const std::string& name) const { return m_properties.contains(name); }
    std::optional<Property> find(const std::string& name) const;

    // Zero when the property is absent; zero is never a valid property id.
    uint32_t id(const std::string& name) const;
    uint64_t value(const std::string& name, uint64_t fallback = 0) const;

    // Names from `required` that this object does not expose.
    std::vector<std::string> missing(const std::vector<std::string>& required) const;

    int size() const { return int(m_properties.size()); }
    bool isEmpty() const { return m_properties.empty(); }

private:
    std::unordered_map<std::string, Property> m_properties;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_PROPERTIES_H_
