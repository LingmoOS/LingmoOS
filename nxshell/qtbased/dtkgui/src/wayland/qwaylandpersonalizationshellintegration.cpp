// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qwaylandpersonalizationshellintegration_p.h"
#include "qwaylandwindowcontextshellsurface_p.h"
#include "wayland-treeland-personalization-manager-v1-client-protocol.h"

DGUI_BEGIN_NAMESPACE
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QWaylandPersonalizationShellIntegration::QWaylandPersonalizationShellIntegration()
    : QWaylandShellIntegration()

#else
QWaylandPersonalizationShellIntegration::QWaylandPersonalizationShellIntegration()
    : QWaylandShellIntegrationTemplate<QWaylandPersonalizationShellIntegration>(4)
#endif
{
}

QWaylandPersonalizationShellIntegration::~QWaylandPersonalizationShellIntegration()
{
    if (object()
        && treeland_personalization_manager_v1_get_version(object())
            >= TREELAND_PERSONALIZATION_MANAGER_V1_GET_WINDOW_CONTEXT_SINCE_VERSION) {
        treeland_personalization_manager_v1_destroy(object());
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool QWaylandPersonalizationShellIntegration::initialize(QtWaylandClient::QWaylandDisplay *display)
{
    QWaylandShellIntegration::initialize(display);
    display->addRegistryListener(registryPluginManager, this);
    return m_manager != nullptr;
}

struct personalization_window_context_v1 *
QWaylandPersonalizationShellIntegration::get_window_context(struct ::wl_surface *surface)
{
    return m_manager->get_window_context(surface);
}
#endif

QtWaylandClient::QWaylandShellSurface *QWaylandPersonalizationShellIntegration::createShellSurface(
    QtWaylandClient::QWaylandWindow *window)
{
    return new QWaylandWindowContextSurface(this, window);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void QWaylandPersonalizationShellIntegration::registryPluginManager(void *data,
                                                                    struct wl_registry *registry,
                                                                    uint32_t id,
                                                                    const QString &interface,
                                                                    uint32_t version)
{
    QWaylandPersonalizationShellIntegration *shell =
        static_cast<QWaylandPersonalizationShellIntegration *>(data);
    if (interface == treeland_personalization_manager_v1_interface.name) {
        shell->m_manager.reset(
            new QtWayland::treeland_personalization_manager_v1(registry, id, version));
    }
}
#endif
DGUI_END_NAMESPACE
