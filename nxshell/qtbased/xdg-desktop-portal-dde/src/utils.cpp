// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "utils.h"

void Utils::setParentWindow(QWidget *w, const QString &parent_window)
{
    if (parent_window.startsWith(QLatin1String("x11:"))) {
        w->setAttribute(Qt::WA_NativeWindow, true);

        WId mainWindowId;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        mainWindowId = parent_window.midRef(4).toULongLong(nullptr, 16);
#else
        mainWindowId = QStringView(parent_window).mid(4).toULongLong(nullptr, 16);
#endif

        // Set the WA_NativeWindow attribute to force the creation of the QWindow.
        // Without this QWidget::windowHandle() returns 0.
        w->setAttribute(Qt::WA_NativeWindow, true);
        QWindow *subWindow = w->windowHandle();
        Q_ASSERT(subWindow);

        QWindow *mainWindow = QWindow::fromWinId(mainWindowId);
        if (!mainWindow) {
            // foreign windows not supported on all platforms
            return;
        }
        // mainWindow is not the child of any object, so make sure it gets deleted at some point
        connect(w, &QObject::destroyed, mainWindow, &QObject::deleteLater);
        subWindow->setTransientParent(mainWindow);


    }
    if (parent_window.startsWith((QLatin1String("wayland:")))) {
        if (!w->window()->windowHandle()) {
            w->window()->winId(); // create QWindow
        }
        setParentWindow(w->window()->windowHandle(), parent_window);
    }
}

void Utils::setParentWindow(QWindow *w, const QString &parent_window)
{
    // TODO
}
