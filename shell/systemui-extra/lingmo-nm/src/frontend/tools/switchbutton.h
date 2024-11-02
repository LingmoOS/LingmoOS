/*
 * Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QObject>
#include <QTimer>
#include <QWidget>
#include <QPainter>

class SwitchButton : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchButton(QWidget *parent = nullptr);
    void     setSwitchStatus(bool check);
    bool     getSwitchStatus();
    void     setEnabled(bool enabled);
    bool     getEnabled();

private:
    QColor          m_colorActive;
    QColor          m_colorInactive;
    int             m_bIsOn = 1;
    QTimer          *m_cTimer;
    float           m_fWidth;
    float           m_fHeight;
    float           m_fCurrentValue;
    void            paintEvent(QPaintEvent *event);
    void            mousePressEvent(QMouseEvent *event);
    void            enterEvent(QEvent *event);
    void            leaveEvent(QEvent *event);
    bool            m_enabled = true;

Q_SIGNALS:
    void clicked();
    void switchStatusChanged();

private Q_SLOTS:
    void startAnimation();

};

#endif // SWITCHBUTTON_H
