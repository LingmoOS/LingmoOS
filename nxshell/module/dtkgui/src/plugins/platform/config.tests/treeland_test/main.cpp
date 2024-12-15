// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <wayland-client-core.h>
#include "qwayland-treeland-personalization-manager-v1.h"
#include <qwaylandclientextension.h>

#include <private/qguiapplication_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>
#include <private/qwaylanddisplay_p.h>

class PersonalizationWindowContext : public QWaylandClientExtensionTemplate<PersonalizationWindowContext>,
                                     public QtWayland::treeland_personalization_window_context_v1
{
public:
    PersonalizationWindowContext(struct ::treeland_personalization_window_context_v1 *context)
    : QWaylandClientExtensionTemplate<PersonalizationWindowContext>(1)
    , QtWayland::treeland_personalization_window_context_v1(context)
    {
        set_titlebar(enable_mode::enable_mode_disable);
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    return 0;
}
