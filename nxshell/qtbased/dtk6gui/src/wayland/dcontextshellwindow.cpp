// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcontextshellwindow.h"

#include "qwaylandpersonalizationshellintegration_p.h"

#include <QtWaylandClient/private/qwaylandwindow_p.h>

DGUI_BEGIN_NAMESPACE
class DContextShellWindowPrivate
{
public:
    explicit DContextShellWindowPrivate(QWindow *window)
        : parentWindow(window)
    {
    }

    QWindow *parentWindow = nullptr;
    bool noTitlebar = false;
};

bool DContextShellWindow::noTitlebar()
{
    return d->noTitlebar;
}

void DContextShellWindow::setNoTitlebar(bool value)
{
    if (value == d->noTitlebar) {
        return;
    }
    d->noTitlebar = value;
    Q_EMIT noTitlebarChanged();
}

static QMap<QWindow *, DContextShellWindow *> s_map;

DContextShellWindow::~DContextShellWindow()
{
    s_map.remove(d->parentWindow);
}

DContextShellWindow::DContextShellWindow(QWindow *window)
    : QObject(window)
    , d(new DContextShellWindowPrivate(window))
{
    s_map.insert(window, this);
    window->create();
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    if (waylandWindow) {
        static QWaylandPersonalizationShellIntegration *shellIntegration = nullptr;
        if (!shellIntegration) {
            shellIntegration = new QWaylandPersonalizationShellIntegration();
            if (!shellIntegration->initialize(waylandWindow->display())) {
                delete shellIntegration;
                shellIntegration = nullptr;
                qWarning() << "failed to init dlayershell intergration";
                return;
            }
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        shellIntegration->createShellSurface(waylandWindow);
#else
        waylandWindow->setShellIntegration(shellIntegration);
#endif
    } else {
        qWarning() << "not a wayland window, will not create zwlr_layer_surface";
    }
}

DContextShellWindow *DContextShellWindow::get(QWindow *window)
{
    auto dlayerShellWindow = s_map.value(window);
    if (dlayerShellWindow) {
        return dlayerShellWindow;
    }
    return new DContextShellWindow(window);
}

DContextShellWindow *DContextShellWindow::qmlAttachedProperties(QObject *object)
{
    auto window = qobject_cast<QWindow *>(object);
    if (window)
        return get(window);
    qWarning() << "not a qwindow unable to create DContextShellWindow";
    return nullptr;
}
DGUI_END_NAMESPACE
