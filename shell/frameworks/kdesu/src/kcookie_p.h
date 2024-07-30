/*
    This file is part of the KDE project, module kdesu
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef KDESUKCOOKIE_H
#define KDESUKCOOKIE_H

#include <QByteArray>

#include <config-kdesu.h>

#include <memory>

namespace KDESu
{
namespace KDESuPrivate
{
/**
 * Utility class to access the authentication tokens needed to run a KDE
 * program (X11 cookies on X11, for instance).
 * @internal
 */

class KCookie
{
public:
    KCookie();
    ~KCookie();

    KCookie(const KCookie &) = delete;
    KCookie &operator=(const KCookie &) = delete;

    /**
     * Returns the X11 display.
     */
    QByteArray display() const;

#if HAVE_X11
    /**
     * Returns the X11 magic cookie, if available.
     */
    QByteArray displayAuth() const;
#endif

private:
    void getXCookie();

private:
    std::unique_ptr<class KCookiePrivate> const d;
};

}
}

#endif // KDESUKCOOKIE_H
