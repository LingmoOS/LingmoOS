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

#ifndef UKMEDIACUSTOMCLASS_H
#define UKMEDIACUSTOMCLASS_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QProxyStyle>
#include <QPainter>
#include <QDebug>
#include <QTabWidget>
#include <QFrame>
#include "kslider.h"

#define MIDDLE_COLOR 178
#define BACKGROUND_COLOR QColor(0,0,0,0)

using namespace kdk;

class UkmediaTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    UkmediaTrayIcon(QWidget *parent = nullptr);
    ~UkmediaTrayIcon();

Q_SIGNALS:
    void wheelRollEventSignal(bool);

protected:
    bool event(QEvent *e) ;
};

class MyTimer : public QObject
{
  Q_OBJECT

public:
  MyTimer(QObject* parent = NULL);
  ~MyTimer();
  void  handleTimeout();  //超时处理函数
  virtual void timerEvent( QTimerEvent *event);
private:
  int m_nTimerID;
Q_SIGNALS:
  void timeOut();
};

typedef struct LingmoUIThemeIcon
{
    QImage image;
    QColor color;
}LingmoUIThemeIcon;
enum DisplayerMode{
    MINI_MODE,
    ADVANCED_MODE
};
enum PushButtonState{
    PUSH_BUTTON_NORMAL,
    PUSH_BUTTON_CLICK,
    PUSH_BUTTON_PRESS
};
static QColor symbolic_color = Qt::gray;

class LingmoUIMediaSliderTipLabel:public QLabel
{
  public:
    LingmoUIMediaSliderTipLabel();
    ~LingmoUIMediaSliderTipLabel();
protected:
    void paintEvent(QPaintEvent*);
};

class LingmoUIButtonDrawSvg:public QPushButton
{
    Q_OBJECT
public:
    LingmoUIButtonDrawSvg(QWidget *parent = nullptr);
    ~LingmoUIButtonDrawSvg();
    QPixmap filledSymbolicColoredPixmap(QImage &source, QColor &baseColor);
    QRect IconGeometry();
    void draw(QPaintEvent* e);
    void init(QImage image ,QColor color);
    friend class DeviceSwitchWidget;
    friend class UkmediaMainWidget;
    friend class UkmediaOsdDisplayWidget;
    friend class UkmediaSystemVolumeWidget;

protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *e);
private:
    int buttonState = PUSH_BUTTON_NORMAL;
    LingmoUIThemeIcon themeIcon;
};

class LingmoUIApplicationWidget:public QWidget
{
    Q_OBJECT
public:
    LingmoUIApplicationWidget(QWidget *parent = nullptr);
    ~LingmoUIApplicationWidget();
protected:
    void paintEvent(QPaintEvent*);
};

class LingmoUIMediaMuteButton:public QPushButton
{
    Q_OBJECT
public:
    LingmoUIMediaMuteButton(QWidget *parent = nullptr);
    ~LingmoUIMediaMuteButton();
    friend class UkmediaMiniMasterVolumeWidget;
    friend class UkmediaMainWidget;
    friend class UkmediaSystemVolumeWidget;
    friend class ApplicationVolumeWidget;
    void setButtonIcon(const QIcon &icon);
    void setActive(const bool &isActive);
    void setDefaultPixmap();
    enum PixmapColor {
        WHITE = 0,
        BLACK,
        GRAY,
        BLUE,
    };
    const QPixmap loadSvg(const QPixmap &source, const PixmapColor &color);

protected:
    void mousePressEvent(QMouseEvent *e)override;
    void mouseReleaseEvent(QMouseEvent *e)override;
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

private:
    bool m_isActivated = false;

    QColor m_backgroundColor;
    QLabel * m_iconLabel = nullptr;
    QPixmap m_pixmap;
    void refreshButtonIcon();
    int buttonState = PUSH_BUTTON_NORMAL;

private Q_SLOTS:
    void onPaletteChanged();
};

class LingmoUIMediaButton:public QPushButton
{
    Q_OBJECT
public:
    LingmoUIMediaButton(QWidget *parent = nullptr);
    ~LingmoUIMediaButton();
    friend class UkmediaMiniMasterVolumeWidget;
    friend class UkmediaMainWidget;
    friend class UkmediaSystemVolumeWidget;
    friend class ApplicationVolumeWidget;
    void setButtonIcon(const QIcon &icon);
    void setDefaultPixmap();
    void setActive(const bool &isActive);
    enum PixmapColor {
        WHITE = 0,
        BLACK,
        GRAY,
        BLUE,
    };
    const QPixmap loadSvg(const QPixmap &source, const PixmapColor &color);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event)override;
    void mouseReleaseEvent(QMouseEvent *event)override;
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
private:

    int buttonState = PUSH_BUTTON_NORMAL;

    bool m_isActivated = false;
    QLabel * m_iconLabel = nullptr;
    QColor m_backgroundColor;
    QPixmap m_pixmap;

    void refreshButtonIcon();

private Q_SLOTS:
    void onPaletteChanged();
};


class LingmoUISettingButton:public QLabel
{
    Q_OBJECT
public:
    LingmoUISettingButton(QWidget *parent = nullptr);
    ~LingmoUISettingButton();
    friend class UkmediaMiniMasterVolumeWidget;
    friend class UkmediaMainWidget;
    friend class UkmediaSystemVolumeWidget;
    friend class ApplicationVolumeWidget;

protected:

    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    QColor m_foregroundColor;

    void setPressColor();
    void setHoverColor();
    void setNormalColor();

Q_SIGNALS:
    void clicked(void);

private Q_SLOTS:
    void onPaletteChanged();
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

class UkmediaVolumeSlider : public QSlider
{
    Q_OBJECT
public:
//    UkmediaVolumeSlider(QWidget *parent = nullptr);
    UkmediaVolumeSlider(QWidget *parent = nullptr,bool needTip = false);
    void initStyleOption(QStyleOptionSlider *option);
    ~UkmediaVolumeSlider();
    bool isMouseWheel = false;

Q_SIGNALS:
    void silderPressSignal();
    void silderReleaseSignal();
    void blueValueChanged(int value);  //针对蓝牙a2dp模式下滑动条跳动，以10为间隔设置音量

private:
    LingmoUIMediaSliderTipLabel *m_pTiplabel;
    bool state = false;
    bool mousePress = false;
    int  blueValue = 0;  //针对蓝牙a2dp模式下滑动条跳动，以10为间隔设置音量
    void updateValue(QMouseEvent *e);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void paintEvent(QPaintEvent *e);
};

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

class Divider : public QFrame
{
public:
    Divider(QWidget * parent = nullptr);
    ~Divider() = default;

protected:
    void paintEvent(QPaintEvent *event);
};

#endif
