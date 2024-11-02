/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QGSettings/QGSettings>

#define OFF_BG_DARK_COLOR "#404040"
#define OFF_HOVER_BG_DARK_COLOR "#666666"
#define ON_BG_DARK_COLOR "#3790FA"
#define ON_HOVER_BG_DARK_COLOR "#40A9FB"
#define DISABLE_DARK_COLOR "#474747"
#define DISABLE_RECT_DARK_COLOR "#6E6E6E"
#define ENABLE_RECT_DARK_COLOR "#FFFFFF"

#define OFF_BG_LIGHT_COLOR "#E0E0E0"
#define OFF_HOVER_BG_LIGHT_COLOR "#B3B3B3"
#define ON_BG_LIGHT_COLOR "#3790FA"
#define ON_HOVER_BG_LIGHT_COLOR "#40A9FB"
#define DISABLE_LIGHT_COLOR "#E9E9E9"
#define DISABLE_RECT_LIGHT_COLOR "#B3B3B3"
#define ENABLE_RECT_LIGHT_COLOR "#FFFFFF"


class SwitchButton : public QWidget
{
    Q_OBJECT

public:
    SwitchButton(QWidget *parent = 0);
    ~SwitchButton();

    void setChecked(bool checked);

    bool isChecked();
    void setFocusStyle(bool val);
    void setDisabledFlag(bool);
    bool getDisabledFlag();
protected:
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

    void drawBg(QPainter * painter);
    void drawSlider(QPainter * painter);
    void changeColor(const QString &themes);

private:
    bool checked;
    bool disabled;
    bool focused = false;

    QColor bgColorOff;
    QColor bgColorOn;
    QColor bgHoverOnColor;
    QColor bgHoverOffColor;
    QColor bgColorDisabled;

    QColor sliderColorEnabled;
    QColor sliderColorDisabled;


    QColor rectColorEnabled;
    QColor rectColorDisabled;

    QGSettings *m_qtThemeSetting;
    QGSettings *m_gtkThemeSetting;

    int space; //滑块离背景间隔
    int rectRadius; //圆角角度

    int step; //移动步长
    int startX;
    int endX;

    bool hover;
    QTimer * timer;


private slots:
    void updatevalue();


Q_SIGNALS:
    void checkedChanged(bool checked);
    void clickedButton(bool value);
};

#endif // SWITCHBUTTON_H
