/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only

    kcookie.cpp: KDE authentication cookies.
*/

#include "kcookie_p.h"

#include <ksu_debug.h>

#include <QProcess>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

extern int kdesuDebugArea();

namespace KDESu
{
namespace KDESuPrivate
{
class KCookiePrivate
{
public:
    QByteArray display;
#if HAVE_X11
    QByteArray displayAuth;
#endif
};

KCookie::KCookie()
    : d(new KCookiePrivate)
{
#if HAVE_X11
    getXCookie();
#endif
}

KCookie::~KCookie() = default;

QByteArray KCookie::display() const
{
    return d->display;
}

#if HAVE_X11
QByteArray KCookie::displayAuth() const
{
    return d->displayAuth;
}
#endif

void KCookie::getXCookie()
{
#if HAVE_X11
    d->display = qgetenv("DISPLAY");
    if (d->display.isEmpty()) {
        // maybe we are on Wayland?
        d->display = qgetenv("WAYLAND_DISPLAY");
        if (!d->display.isEmpty()) {
            // don't go into the xauth code path
            return;
        }
    }
#else
    d->display = qgetenv("QWS_DISPLAY");
#endif
    if (d->display.isEmpty()) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "$DISPLAY is not set.";
        return;
    }
#if HAVE_X11 // No need to mess with X Auth stuff
    QByteArray disp = d->display;
    if (disp.startsWith("localhost:")) { // krazy:exclude=strings
        disp.remove(0, 9);
    }

    const QString xauthExec = QStandardPaths::findExecutable(QStringLiteral("xauth"));
    if (xauthExec.isEmpty()) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "Could not run xauth, not found in path";
        return;
    }

    QProcess proc;
    proc.start(xauthExec, QStringList{QStringLiteral("list"), QString::fromUtf8(disp)});
    if (!proc.waitForStarted()) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "Could not run xauth. Found in path:" << xauthExec;
        return;
    }
    proc.waitForReadyRead(100);

    QByteArray output = proc.readLine().simplified();
    if (output.isEmpty()) {
        qCWarning(KSU_LOG) << "No X authentication info set for display" << d->display;
        return;
    }

    QList<QByteArray> lst = output.split(' ');
    if (lst.count() != 3) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "parse error.";
        return;
    }
    d->displayAuth = (lst[1] + ' ' + lst[2]);
    proc.waitForFinished(100); // give QProcess a chance to clean up gracefully
#endif
}

} // namespace KDESuPrivate
} // namespace KDESu
