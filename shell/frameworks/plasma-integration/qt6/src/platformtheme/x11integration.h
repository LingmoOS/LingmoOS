/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef X11INTEGRATION_H
#define X11INTEGRATION_H

#include "kdeplatformtheme.h"
#include <QHash>
#include <QObject>
#include <xcb/xcb.h>

class QWindow;

class X11Integration : public QObject
{
    Q_OBJECT
public:
    explicit X11Integration(KdePlatformTheme *platformTheme);
    ~X11Integration() override;
    void init();

    void setWindowProperty(QWindow *window, const QByteArray &name, const QByteArray &value);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void installColorScheme(QWindow *w);
    void installDesktopFileName(QWindow *w);
    QHash<QByteArray, xcb_atom_t> m_atoms;
    KdePlatformTheme *m_platformTheme;
};

#endif
