// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORMENU_H
#define COLORMENU_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QColor>
#include <QPaintEvent>

class ColorMenu : public QWidget
{
    Q_OBJECT

public:
    ColorMenu(int x, int y, int size, QColor color, QWidget *parent = 0);
    ~ColorMenu();

signals:
    void copyColor(QColor color, QString colorType);
    void exit();

public slots:
    void copyFloatRGBAColor();
    void copyFloatRGBColor();
    void copyHexColor();
    void copyCmykColor();
    void copyHsvColor();
    void copyRGBAColor();
    void copyRGBColor();
//    void showMenu();

protected:
    void paintEvent(QPaintEvent *);

private:
    QAction *hexAction;
    QAction *rgbAction;
    QAction *rgbFloatAction;
    QAction *rgbaAction;
    QAction *rgbaFloatAction;
    QAction *cmykAction;
    QAction *hsvAction;
    QColor windowColor;
    QMenu *colorMenu;
    bool clickMenuItem;
    int menuOffsetX;
    int menuOffsetY;
    int shadowBottomMargin;
    int shadowXMargin;
    int windowSize;
    int windowX;
    int windowY;
};

#endif
