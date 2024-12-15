// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PERSONALIZATIONWAYLANDCLIENTEXTENSION_H
#define PERSONALIZATIONWAYLANDCLIENTEXTENSION_H

#include "dtreelandplatforminterface.h"
#include "qwayland-treeland-personalization-manager-v1.h"

#include <dtkgui_global.h>
#include <private/qwaylanddisplay_p.h>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QWaylandClientExtensionTemplate>

DGUI_BEGIN_NAMESPACE

class PersonalizationAppearanceContext;
class PersonalizationCursorContext;
class PersonalizationFontContext;
class DTreelandPlatformInterfacePrivate;

class PersonalizationManager: public QWaylandClientExtensionTemplate<PersonalizationManager>,
                              public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
public:
    static PersonalizationManager *instance();

    [[nodiscard]]inline bool isSupported() const { return m_isSupported; }

private:
    void addListener();
    void removeListener();

    explicit PersonalizationManager();
    static void handleListenerGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version);

private:
    QtWaylandClient::QWaylandDisplay *m_waylandDisplay;
    bool m_isSupported;
};

class PersonalizationWindowContext : public QWaylandClientExtensionTemplate<PersonalizationWindowContext>,
                                     public QtWayland::treeland_personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context);
};

class PersonalizationAppearanceContext : public QWaylandClientExtensionTemplate<PersonalizationAppearanceContext>,
                                         public QtWayland::treeland_personalization_appearance_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationAppearanceContext(struct ::treeland_personalization_appearance_context_v1 *context, DTreelandPlatformInterface *interface);

protected:
    void treeland_personalization_appearance_context_v1_round_corner_radius(int32_t radius) override;
    void treeland_personalization_appearance_context_v1_icon_theme(const QString &theme_name) override;
    void treeland_personalization_appearance_context_v1_active_color(const QString &active_color) override;
    void treeland_personalization_appearance_context_v1_window_theme_type(uint32_t type) override;
    void treeland_personalization_appearance_context_v1_window_opacity(uint32_t opacity) override;
private:
    DTreelandPlatformInterface *m_interface;
};

class PersonalizationFontContext : public QWaylandClientExtensionTemplate<PersonalizationFontContext>,
                                   public QtWayland::treeland_personalization_font_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationFontContext(struct ::treeland_personalization_font_context_v1 *context, DTreelandPlatformInterface *interface);

protected:
    void treeland_personalization_font_context_v1_font(const QString &font_name) override;
    void treeland_personalization_font_context_v1_monospace_font(const QString &font_name) override;
    void treeland_personalization_font_context_v1_font_size(uint32_t font_size) override;
private:
    DTreelandPlatformInterface *m_interface;
};

DGUI_END_NAMESPACE

#endif // PERSONALIZATIONWAYLANDCLIENTEXTENSION_H
