/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "controlcenterdialog.h"
#include <KWindowSystem>
#include <NETWM>
#include <xcb/xcb.h>
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>

ControlCenterDialog::ControlCenterDialog(QQuickWindow *parent)
    : QQuickWindow(parent)
{
    setFlags(Qt::Popup);
    setColor(Qt::transparent);
    installEventFilter(this);
}

void ControlCenterDialog::open()
{
    setVisible(true);
    setMouseGrabEnabled(true);
    setKeyboardGrabEnabled(true);
}

bool ControlCenterDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (QWindow *w = qobject_cast<QWindow*>(object)) {
            if (!w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPosition().toPoint())) {
                ControlCenterDialog::setVisible(false);
            }
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            ControlCenterDialog::setVisible(false);
        }
    } else if (event->type() == QEvent::Show) {
        auto *nativeInterface = qApp->platformNativeInterface();
        xcb_connection_t *connection = static_cast<xcb_connection_t*>(
            nativeInterface->nativeResourceForScreen("xcbconnection"));
        xcb_window_t rootWindow = DefaultRootWindow(connection);
        NETWinInfo info(connection, winId(), rootWindow, NET::WMState);
        info.setState(NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher,
                      NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    } else if (event->type() == QEvent::Hide) {
        setMouseGrabEnabled(false);
        setKeyboardGrabEnabled(false);
    }

    return QObject::eventFilter(object, event);
}
