 /* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#ifndef LINGMOCUSTOMSTYLE_H
#define LINGMOCUSTOMSTYLE_H

#include <QObject>
#include <QPushButton>
#include <QImage>
#include <QSlider>
#include <QLabel>
#include <QMouseEvent>
#include "kslider.h"
#include <QDomDocument>
#include <QDir>
#include <QFile>
#include <QDebug>

using namespace kdk;

static QColor symbolic_color = Qt::gray;

//文本长自动省略并添加悬浮
class FixLabel : public QLabel
{
        Q_OBJECT
    public:
        explicit FixLabel(QWidget *parent = nullptr);
        explicit FixLabel(QString text , QWidget *parent = nullptr);
        ~FixLabel();
        void setText(const QString &text, bool saveTextFlag = true);
        QString fullText;
    private:
        void paintEvent(QPaintEvent *event);

    private:
};

class AudioSlider : public KSlider
{
    Q_OBJECT
public:
    AudioSlider(QWidget *parent = nullptr);
    ~AudioSlider();
    bool isMouseWheel = false;

private:
    int blueValue = 0;

Q_SIGNALS:
    void blueValueChanged(int value);   //针对蓝牙a2dp模式下滑动条跳动，以10为间隔设置音量

protected:
    void wheelEvent(QWheelEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

class UkmediaVolumeSlider : public KSlider
{
    Q_OBJECT
public:
    UkmediaVolumeSlider(QWidget *parent = nullptr);
    ~UkmediaVolumeSlider();
    void initStyleOption(QStyleOptionSlider *option);
    void showTooltip();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
};


class LingmoUIButtonDrawSvg:public QPushButton
{
    Q_OBJECT
public:
    LingmoUIButtonDrawSvg(QWidget *parent = nullptr);
    ~LingmoUIButtonDrawSvg();
    QPixmap filledSymbolicColoredPixmap(QImage &source, QColor &baseColor);
    QRect IconGeometry();
    void drawIcon(QPaintEvent* e); //绘制图标
    void refreshIcon(QImage image ,QColor color); //刷新图标
    void outputVolumeDarkThemeImage(int value, bool status);
    friend class UkmediaMainWidget;
protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *e);
private:
    QImage mImage;
    QColor mColor;

    bool mousePress = false;
};

class TitleLabel : public QLabel
{
    Q_OBJECT
public:
    TitleLabel(QWidget *parent = nullptr);
    ~TitleLabel();
};

#endif // LINGMOCUSTOMSTYLE_H
