/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WINDOWRADIUS_H
#define WINDOWRADIUS_H

#include <QObject>
#include <QPointF>

namespace KWin
{
class Window;

class WindowRadius : public QObject
{
    Q_OBJECT

public:
    explicit WindowRadius(Window *window);
    ~WindowRadius();

    int updateWindowRadius();

    QPointF getWindowRadius();
    QPointF windowRadius() { return m_radius;};
    QString theme() const;

public Q_SLOTS:
    void onUpdateWindowRadiusChanged();

public:
    Window *m_window;
    quint32 m_atom_deepin_scissor_window;
    QPointF m_radius = QPointF(-1, 0);
    bool    m_isMaximized = false;
    float   m_scale = 1.0;
};
}

#endif
