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
#ifndef X11INTEGRATION_H
#define X11INTEGRATION_H

#include <QObject>
#include <QHash>
#include <xcb/xcb.h>

class QWindow;

class X11Integration : public QObject
{
    Q_OBJECT
public:
    explicit X11Integration();
    ~X11Integration() override;
    void init();

    void setWindowProperty(QWindow *window, const QByteArray &name, const QByteArray &value);

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void installDesktopFileName(QWindow *w);
    QHash<QByteArray, xcb_atom_t> m_atoms;
};

#endif
