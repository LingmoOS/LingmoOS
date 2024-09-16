// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANIMATION_H
#define ANIMATION_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>

class Animation : public QWidget
{
    Q_OBJECT
    
public:
    Animation(int x, int y, QPixmap pixmap, QColor color, QWidget *parent=0);
	~Animation(); 
    
public slots:
    void renderAnimation();
    
signals:
    void finish();
    
protected:
    void paintEvent(QPaintEvent *);
    
private:
    QColor cursorColor;
    QPixmap screenshotPixmap;
    QTimer *renderTimer;
    int animationDuration;
    int animationFrames;
    int blockHeight;
    int blockWidth;
    int cursorX;
    int cursorY;
    int height;
    int renderTicker;
    int width;
};	

#endif
