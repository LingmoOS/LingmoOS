/*  This file is part of the KDE libraries
 *  Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>
 *  Copyright 2016 Marco Martin <mart@kde.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#include "x11integration.h"

#include <QCoreApplication>
#include <QX11Info>
#include <QPlatformSurfaceEvent>
#include <QGuiApplication>
#include <QWindow>
#include <QWidget>
#include <QVariant>
#include <QRegion>
#include <QDebug>

#include <NETWM>
#include <KWindowEffects>

#include <xcb/xcb.h>

static const char s_schemePropertyName[] = "KDE_COLOR_SCHEME_PATH";
static const QByteArray s_blurBehindPropertyName = QByteArrayLiteral("ENABLE_BLUR_BEHIND_HINT");
static const QByteArray s_blurRegionPropertyName = QByteArrayLiteral("BLUR_REGION");

X11Integration::X11Integration()
    : QObject()
{
}

X11Integration::~X11Integration() = default;

void X11Integration::init()
{
    QCoreApplication::instance()->installEventFilter(this);
}

bool X11Integration::eventFilter(QObject *watched, QEvent *event)
{
    //the drag and drop window should NOT be a tooltip
    //https://bugreports.qt.io/browse/QTBUG-52560
    if (event->type() == QEvent::Show && watched->inherits("QShapedPixmapWindow")) {
        //static cast should be safe there
        QWindow *w = static_cast<QWindow *>(watched);
        NETWinInfo info(QX11Info::connection(), w->winId(), QX11Info::appRootWindow(), NET::WMWindowType, NET::Properties2());
        info.setWindowType(NET::DNDIcon);
        // TODO: does this flash the xcb connection?
    }

    // if (event->type() == QEvent::PlatformSurface) {
    //     if (QWindow *w = qobject_cast<QWindow*>(watched)) {
    //         QPlatformSurfaceEvent *pe = static_cast<QPlatformSurfaceEvent*>(event);
    //         if (!w->flags().testFlag(Qt::ForeignWindow)) {
    //             if (pe->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
    //                 const auto blurBehindProperty = w->property(s_blurBehindPropertyName.constData());
    //                 if (blurBehindProperty.isValid()) {
    //                     KWindowEffects::enableBlurBehind(w->winId(), blurBehindProperty.toBool());
    //                 }
    //                 installDesktopFileName(w);
    //             }
    //         }
    //     }
    // }

    // if (event->type() == QEvent::ApplicationPaletteChange) {
    //     const auto topLevelWindows = QGuiApplication::topLevelWindows();
    //     for (QWindow *w : topLevelWindows) {
    //         installColorScheme(w);
    //     }
    // }

    return false;
}

void X11Integration::installDesktopFileName(QWindow *w)
{
    if (!w->isTopLevel()) {
        return;
    }

    QString desktopFileName = QGuiApplication::desktopFileName();
    if (desktopFileName.isEmpty()) {
        return;
    }
    // handle apps which set the desktopFileName property with filename suffix,
    // due to unclear API dox (https://bugreports.qt.io/browse/QTBUG-75521)
    if (desktopFileName.endsWith(QLatin1String(".desktop"))) {
        desktopFileName.chop(8);
    }
    NETWinInfo info(QX11Info::connection(), w->winId(), QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    info.setDesktopFileName(desktopFileName.toUtf8().constData());
}

void X11Integration::setWindowProperty(QWindow *window, const QByteArray &name, const QByteArray &value)
{
    auto *c = QX11Info::connection();

    xcb_atom_t atom;
    auto it = m_atoms.find(name);
    if (it == m_atoms.end()) {
        const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, false, name.length(), name.constData());
        QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> reply(xcb_intern_atom_reply(c, cookie, nullptr));
        if (!reply.isNull()) {
            atom = reply->atom;
            m_atoms[name] = atom;
        } else {
            return;
        }
    } else {
        atom = *it;
    }

    if (value.isEmpty()) {
        xcb_delete_property(c, window->winId(), atom);
    } else {
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom, XCB_ATOM_STRING,
                            8, value.length(), value.constData());
    }
}
