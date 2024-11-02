/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#ifndef UKMEDIAVOLUMESLIDER_H
#define UKMEDIAVOLUMESLIDER_H
#include <QSlider>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QEvent>
#include <QHideEvent>
#include <QScrollArea>

//typedef struct LingmoUIThemeIcon
//{
//    QImage image;
//    QColor color;
//}LingmoUIThemeIcon;
//enum DisplayerMode{
//    MINI_MODE,
//    ADVANCED_MODE
//};
//enum SwitchButtonState{
//    SWITCH_BUTTON_NORMAL,
//    SWITCH_BUTTON_HOVER,
//    SWITCH_BUTTON_PRESS
//};
//static QColor symbolic_color = Qt::gray;

//class LingmoUIMediaSliderTipLabel:public QLabel
//{
//  public:
//    LingmoUIMediaSliderTipLabel();
//    ~LingmoUIMediaSliderTipLabel();
//protected:
//    void paintEvent(QPaintEvent*);
//};

//class LingmoUIButtonDrawSvg:public QPushButton
//{
//    Q_OBJECT
//public:
//    LingmoUIButtonDrawSvg(QWidget *parent = nullptr);
//    ~LingmoUIButtonDrawSvg();
//    QPixmap filledSymbolicColoredPixmap(QImage &source, QColor &baseColor);
//    QRect IconGeometry();
//    void draw(QPaintEvent* e);
//    void init(QImage image ,QColor color);
//    friend class DeviceSwitchWidget;
//    friend class UkmediaOsdDisplayWidget;

//protected:
//    void paintEvent(QPaintEvent *event);
//    bool event(QEvent *e);
//private:
//    LingmoUIThemeIcon themeIcon;
//};

//class LingmoUIApplicationWidget:public QWidget
//{
//    Q_OBJECT
//public:
//    LingmoUIApplicationWidget(QWidget *parent = nullptr);
//    ~LingmoUIApplicationWidget();
//protected:
//    void paintEvent(QPaintEvent*);
//};

//class LingmoUIMediaButton:public QPushButton
//{
//    Q_OBJECT
//public:
//    LingmoUIMediaButton(QWidget *parent = nullptr);
//    ~LingmoUIMediaButton();
//    friend class UkmediaMiniMasterVolumeWidget;

//Q_SIGNALS:
//    void advanceToMiniSignal();
//    void miniToAdvanceSignal();
//    void moveMiniSwitchBtnSignale();
//    void moveAdvanceSwitchBtnSignal();
//protected:
//    void mousePressEvent(QMouseEvent *e)override;
//    void mouseMoveEvent(QMouseEvent *e)override;
//    void mouseReleaseEvent(QMouseEvent *e)override;
////    void paintEvent(QPaintEvent *event);
//    void enterEvent(QEvent *event);
//    void leaveEvent(QEvent *event);
//private:
//};

//class UkmediaVolumeSlider : public QSlider
//{
//    Q_OBJECT
//public:
////    UkmediaVolumeSlider(QWidget *parent = nullptr);
//    UkmediaVolumeSlider(QWidget *parent = nullptr,bool needTip = false);
//    void initStyleOption(QStyleOptionSlider *option);
//    ~UkmediaVolumeSlider();
//Q_SIGNALS:
//    void silderPressSignal();
//    void silderReleaseSignal();
//private:
//    LingmoUIMediaSliderTipLabel *m_pTiplabel;
//    bool state = false;
//    bool mousePress = false;


//protected:
//    void mousePressEvent(QMouseEvent *ev);
//    void mouseReleaseEvent(QMouseEvent *e);
//    void mouseMoveEvent(QMouseEvent *e)
//    {
//        int value = 0;
//        int currentX = e->pos().x();
//        double per = currentX * 1.0 / this->width();
//        if ((this->maximum() - this->minimum()) >= 50) { //减小鼠标点击像素的影响
//            value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
//            if (value <= (this->maximum() / 2 - this->maximum() / 10 + this->minimum() / 10)) {
//                value = qRound(per*(this->maximum() - this->minimum() - 1)) + this->minimum();
//            } else if (value > (this->maximum() / 2 + this->maximum() / 10 + this->minimum() / 10)) {
//                value = qRound(per*(this->maximum() - this->minimum() + 1)) + this->minimum();
//            } else {
//                value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
//            }
//        } else {
//            value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
//        }
//        this->setValue(value);
//        QSlider::mousePressEvent(e);
////        setCursor(QCursor(Qt::OpenHandCursor));
////        m_displayLabel->move((this->width()-m_displayLabel->width())*this->value()/(this->maximum()-this->minimum()),3);
////        QSlider::mouseMoveEvent(e);
//    }
//    void leaveEvent(QEvent *e);

//    void enterEvent(QEvent *e);
//    void paintEvent(QPaintEvent *e);
//};

//class LingmoUIScrollArea : public QScrollArea
//{
//    Q_OBJECT
//public:
//    LingmoUIScrollArea(QWidget *parent = nullptr);
//    ~LingmoUIScrollArea();

//protected:
//    void paintEvent(QPaintEvent *e);

//};

//class LingmoUIQMenu:public QMenu
//{
//    Q_OBJECT
//public:
//    LingmoUIQMenu();
//    ~LingmoUIQMenu();
//protected:
//    void hideEvent(QHideEvent *e);
//    virtual bool event(QEvent *e) override;


//};

#endif // UKMEDIAVOLUMESLIDER_H
