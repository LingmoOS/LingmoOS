/**
 * Copyright (C) 2018 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QWidget>
#include <QMap>
#include <QTime>
#include "x11keyboard.h"

namespace Ui {
class KeyboardWidget;
}

class QPushButton;

class KeyboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardWidget(QWidget *parent = 0);
    ~KeyboardWidget();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void bindSingal();
    void toggleCase();
    void switchPage();
    void setDefaultIcon();
    void setIconSize();
    void changeKeyStyle(QPushButton *btn, bool isPressed);
    void changeFuncKeyStyle(QPushButton *btn, bool isPressed);
    void changeShitKeyStyle(QPushButton *btn, bool isPressed);
    void changeDirectKeyStyle(QPushButton *btn, bool isPressed);
    void clearModifier();
    QString getKeyName(QPushButton *btn);


private Q_SLOTS:
    void onButtonClicked();
    void onButtonPressed();
    void onButtonReleased();

Q_SIGNALS:
    void aboutToClose();
    void keyPressed(QChar c);
    void keyPressed(FuncKey::FUNCKEY key);

private:
    Ui::KeyboardWidget      *ui;
    bool                    capsLock;           //是否大写锁定
    bool                    isShift;
    QTime                   shiftLastClicked;   //shift键上次被点击的时间
    int                     page;               //当前是第几页的键盘
    X11Keyboard             *vKeyboard;
};

#endif // KEYBOARDWIDGET_H
