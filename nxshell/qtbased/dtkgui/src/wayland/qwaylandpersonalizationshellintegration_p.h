// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkgui_global.h>
#include <private/qwaylandshellintegration_p.h>
#include <qwayland-treeland-personalization-manager-v1.h>

DGUI_BEGIN_NAMESPACE
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QWaylandWindowContextSurface;
class QWaylandPersonalizationShellIntegration : public QtWaylandClient::QWaylandShellIntegration
#else
class QWaylandPersonalizationShellIntegration
    : public QtWaylandClient::QWaylandShellIntegrationTemplate<
          QWaylandPersonalizationShellIntegration>,
      public QtWayland::treeland_personalization_manager_v1
#endif
{
public:
    QWaylandPersonalizationShellIntegration();
    ~QWaylandPersonalizationShellIntegration() override;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool initialize(QtWaylandClient::QWaylandDisplay *display) override;
    struct personalization_window_context_v1 *get_window_context(struct ::wl_surface *surface);

    struct treeland_personalization_manager_v1 *object() { return m_manager->object(); }
#endif

    QtWaylandClient::QWaylandShellSurface *createShellSurface(
        QtWaylandClient::QWaylandWindow *window) override;

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    static void registryPluginManager(void *data,
                                      struct wl_registry *registry,
                                      uint32_t id,
                                      const QString &interface,
                                      uint32_t version);
    QScopedPointer<QtWayland::treeland_personalization_manager_v1> m_manager;
#endif
};
DGUI_END_NAMESPACE
