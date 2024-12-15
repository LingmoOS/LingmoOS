// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "personalizationwaylandclientextension.h"

#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>
#include <qwaylandclientextension.h>
#include "dplatformtheme.h"

#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QGuiApplication>

DGUI_BEGIN_NAMESPACE

PersonalizationManager::PersonalizationManager()
    : QWaylandClientExtensionTemplate<PersonalizationManager>(treeland_personalization_manager_v1_interface.version)
    , m_isSupported(false)
{
    QtWaylandClient::QWaylandIntegration *waylandIntegration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
    if (!waylandIntegration) {
        qWarning() << "waylandIntegration is nullptr!!!";
        return;
    }
    m_waylandDisplay = waylandIntegration->display();
    if (m_waylandDisplay == nullptr) {
        qWarning() << "waylandDisplay is nullptr!!!";
        return;
    }
    addListener();
    m_isSupported = m_waylandDisplay->hasRegistryGlobal(QString::fromUtf8(treeland_personalization_manager_v1_interface.name));
    if (!m_isSupported) {
        qWarning() << "PersonalizationManager is not support";
    }
}

void PersonalizationManager::addListener()
{
    m_waylandDisplay->addRegistryListener(&handleListenerGlobal, this);
}

void PersonalizationManager::removeListener()
{
    m_waylandDisplay->removeListener(&handleListenerGlobal, this);
}

void PersonalizationManager::handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    if (interface == treeland_personalization_manager_v1_interface.name) {
        PersonalizationManager *integration = static_cast<PersonalizationManager *>(data);
        if (!integration) {
            qWarning() << "integration is nullptr!!!";
            return;
        }

        integration->init(registry, id, version);
    }
}

PersonalizationManager *PersonalizationManager::instance()
{
    static PersonalizationManager instance;
    return &instance;
}

PersonalizationWindowContext::PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWindowContext>(treeland_personalization_window_context_v1_interface.version)
    , QtWayland::treeland_personalization_window_context_v1(context)
{

}

PersonalizationAppearanceContext::PersonalizationAppearanceContext(struct ::treeland_personalization_appearance_context_v1 *context, DTreelandPlatformInterface *interface)
    : QWaylandClientExtensionTemplate<PersonalizationAppearanceContext>(treeland_personalization_appearance_context_v1_interface.version)
    , QtWayland::treeland_personalization_appearance_context_v1(context)
    , m_interface(interface)
{
    get_round_corner_radius();
    get_icon_theme();
    get_active_color();
    get_window_theme_type();
    get_window_opacity();
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_round_corner_radius(int32_t radius)
{
    m_interface->m_windowRadius = radius;
    emit m_interface->m_platformTheme->windowRadiusChanged(radius);
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_icon_theme(const QString &theme_name)
{
    m_interface->m_iconThemeName = theme_name.toUtf8();
    emit m_interface->m_platformTheme->iconThemeNameChanged(theme_name.toUtf8());
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_active_color(const QString &active_color)
{
    m_interface->m_activeColor = active_color;
    emit m_interface->m_platformTheme->activeColorChanged(active_color);
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_window_theme_type(uint32_t type)
{
    QString theme = QStringLiteral("lingmo");
    if (type == PersonalizationAppearanceContext::theme_type::theme_type_dark) {
        theme = theme + QStringLiteral("-dark");
    }
    m_interface->m_themeName = theme.toUtf8();
    emit m_interface->m_platformTheme->themeNameChanged(theme.toUtf8());
}

void PersonalizationAppearanceContext::treeland_personalization_appearance_context_v1_window_opacity(uint32_t opacity)
{
    m_interface->m_blurOpacity = opacity;
}

PersonalizationFontContext::PersonalizationFontContext(struct ::treeland_personalization_font_context_v1 *context, DTreelandPlatformInterface *interface)
    : QWaylandClientExtensionTemplate<PersonalizationFontContext>(treeland_personalization_font_context_v1_interface.version)
    , QtWayland::treeland_personalization_font_context_v1(context)
    , m_interface(interface)
{
    get_font();
    get_monospace_font();
    get_font_size();
}

void PersonalizationFontContext::treeland_personalization_font_context_v1_font(const QString &font_name)
{
    m_interface->m_fontName = font_name.toUtf8();
    emit m_interface->m_platformTheme->fontNameChanged(font_name.toUtf8());
}

void PersonalizationFontContext::treeland_personalization_font_context_v1_monospace_font(const QString &font_name)
{
    m_interface->m_monoFontName = font_name.toUtf8();
    emit m_interface->m_platformTheme->monoFontNameChanged(font_name.toUtf8());
}

void PersonalizationFontContext::treeland_personalization_font_context_v1_font_size(uint32_t font_size)
{
    // treeland侧无法存储浮点数，约定 font_size 为 10 倍的 pointSize
    qreal pointSize = font_size / 10.0;
    pointSize = qBound(8.25, pointSize, 15.00);
    m_interface->m_fontPointSize = pointSize;
    emit m_interface->m_platformTheme->fontPointSizeChanged(pointSize);
}

DGUI_END_NAMESPACE
