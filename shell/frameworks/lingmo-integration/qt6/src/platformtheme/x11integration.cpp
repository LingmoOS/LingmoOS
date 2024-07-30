/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "x11integration.h"

#include <NETWM>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QPlatformSurfaceEvent>
#include <QWindow>
#include <private/qtx11extras_p.h>

#include <KWindowEffects>

static const char s_schemePropertyName[] = "KDE_COLOR_SCHEME_PATH";
static const QByteArray s_blurBehindPropertyName = QByteArrayLiteral("ENABLE_BLUR_BEHIND_HINT");

X11Integration::X11Integration(KdePlatformTheme *platformTheme)
    : QObject()
    , m_platformTheme(platformTheme)
{
}

X11Integration::~X11Integration() = default;

void X11Integration::init()
{
    QCoreApplication::instance()->installEventFilter(this);
}

bool X11Integration::eventFilter(QObject *watched, QEvent *event)
{
    // the drag and drop window should NOT be a tooltip
    // https://bugreports.qt.io/browse/QTBUG-52560
    if (event->type() == QEvent::Show && watched->inherits("QShapedPixmapWindow")) {
        // static cast should be safe there
        QWindow *w = static_cast<QWindow *>(watched);
        NETWinInfo info(QX11Info::connection(), w->winId(), QX11Info::appRootWindow(), NET::WMWindowType, NET::Properties2());
        info.setWindowType(NET::DNDIcon);
        // TODO: does this flash the xcb connection?
    }
    if (event->type() == QEvent::PlatformSurface) {
        if (QWindow *w = qobject_cast<QWindow *>(watched)) {
            QPlatformSurfaceEvent *pe = static_cast<QPlatformSurfaceEvent *>(event);
            if (!w->flags().testFlag(Qt::ForeignWindow)) {
                if (pe->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                    auto flags = w->flags();
                    // A recent KWin change means it now follows WindowButtonHints on X11
                    // Some KDE applications use QDialogs for their main window,
                    // which means the KWin now surfaces those windows having the wrong hints.
                    // To avoid clients changing, we adjust flags here.
                    // This is documented as being only available on some platforms,
                    // so altering is relatively safe.
                    if (flags.testFlag(Qt::Dialog) && !flags.testFlag(Qt::CustomizeWindowHint)) {
                        if (!w->transientParent()) {
                            flags.setFlag(Qt::WindowCloseButtonHint);
                            flags.setFlag(Qt::WindowMinMaxButtonsHint);
                        }
                        w->setFlags(flags);
                    }

                    if (qApp->property(s_schemePropertyName).isValid()) {
                        installColorScheme(w);
                    }
                    const auto blurBehindProperty = w->property(s_blurBehindPropertyName.constData());
                    if (blurBehindProperty.isValid()) {
                        KWindowEffects::enableBlurBehind(w, blurBehindProperty.toBool());
                    }
                    installDesktopFileName(w);
                }
            }
        }
    }
    if (event->type() == QEvent::ApplicationPaletteChange) {
        const auto topLevelWindows = QGuiApplication::topLevelWindows();
        for (QWindow *w : topLevelWindows) {
            installColorScheme(w);
        }
    }
    return false;
}

void X11Integration::installColorScheme(QWindow *w)
{
    if (!w->isTopLevel() || !w->handle() /* e.g. WebEngine's QQuickWindow */) {
        return;
    }
    static xcb_atom_t atom = XCB_ATOM_NONE;
    xcb_connection_t *c = QX11Info::connection();
    if (atom == XCB_ATOM_NONE) {
        const QByteArray name = QByteArrayLiteral("_KDE_NET_WM_COLOR_SCHEME");
        const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, false, name.length(), name.constData());
        QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> reply(xcb_intern_atom_reply(c, cookie, nullptr));
        if (!reply.isNull()) {
            atom = reply->atom;
        } else {
            // no point in continuing, we don't have the atom
            return;
        }
    }
    const QString path = qApp->property(s_schemePropertyName).toString();
    if (path.isEmpty()) {
        xcb_delete_property(c, w->winId(), atom);
    } else {
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, w->winId(), atom, XCB_ATOM_STRING, 8, path.size(), qPrintable(path));
    }
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
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom, XCB_ATOM_STRING, 8, value.length(), value.constData());
    }
}

#include "moc_x11integration.cpp"
