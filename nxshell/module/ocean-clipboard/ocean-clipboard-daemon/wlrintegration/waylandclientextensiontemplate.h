// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <private/qwaylandclientextension_p.h>

/**
 * @brief Like QWaylandShellIntegrationTemplate but without shell specific pure virtual functions
 */
template <typename T>
class WaylandClientExtensionTemplate : public QWaylandClientExtension
{
public:
    explicit WaylandClientExtensionTemplate(const int ver)
        : QWaylandClientExtension(ver) {}

    bool initialize() {
        QWaylandClientExtension::initialize();
        return isActive();
    }

    const wl_interface *extensionInterface() const override {
        return static_cast<const T *>(this)->interface();
    }

    void bind(wl_registry *registry, int id, int ver) override {
        std::array<int, 3> versions = {
            QWaylandClientExtension::version(),
            T::interface()->version,
            ver
        };
        int minVer = *std::min_element(versions.cbegin(), versions.cend());
        setVersion(minVer);
        static_cast<T *>(this)->init(registry, id, minVer);
    }
};
