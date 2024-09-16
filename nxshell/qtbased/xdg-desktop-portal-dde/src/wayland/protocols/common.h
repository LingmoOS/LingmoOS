// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <private/qguiapplication_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandintegration_p.h>
#include <QPointer>

inline QtWaylandClient::QWaylandIntegration *waylandIntegration()
{
    return dynamic_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
}

inline QPointer<QtWaylandClient::QWaylandDisplay> waylandDisplay()
{
    return waylandIntegration()->display();
}
