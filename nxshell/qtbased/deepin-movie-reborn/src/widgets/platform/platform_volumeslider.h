// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef Platform_VOLUMESLIDER_H
#define Platform_VOLUMESLIDER_H

#include <DSlider>
#include <DToolButton>
#include <QPoint>
#include <DArrowRectangle>
#include <DUtil>

#include "utils.h"
#include "volumemonitoring.h"
#include "threadpool.h"
#include "platform/platform_mainwindow.h"
#include "compositing_manager.h"
#include "dmr_settings.h"
#include "dbus_adpator.h"
#include "../accessibility/ac-deepin-movie-define.h"

DWIDGET_USE_NAMESPACE

namespace dmr {

class Platform_VolumeSlider: public QWidget
{
    Q_OBJECT

public:
    enum State {
        Open,
        Close
    };

signals:
    void sigVolumeChanged(int nValue);
    void sigMuteStateChanged(bool bMute);

public:
    Platform_VolumeSlider(Platform_MainWindow *mw, QWidget *parent);
    ~Platform_VolumeSlider();

    State state() const {return m_state;}
    void initVolume();   //初始化音量
    void stopTimer();
    void popup();        //弹起音量条
    void updatePoint(QPoint point);
    bool getsliderstate();
    int getVolume();   //获取当前实际音量
    void changeVolume(int nVolume);    //改变控件音量
    void calculationStep(int iAngleDelta);   //计算滚轮滚动的步进并判断步进值是否大于等于120;普通鼠标的滚轮精度为120转动一刻为120*1/8=15度

public slots:
    void volumeUp();                   //滚轮加音量
    void volumeDown();                 //滚轮减音量
    void changeMuteState(bool bMute);  //改变静音状态
    void volumeChanged(int nVolume);   //控件音量变化后的后续处理
    void muteButtnClicked();
    void setThemeType(int type);
    void delayedHide();

protected:
    void enterEvent(QEvent *e);
    void showEvent(QShowEvent *se);
    void leaveEvent(QEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *pEvent);
    bool eventFilter(QObject *obj, QEvent *e);
    void refreshIcon();                //刷新图标

private:
    QString readSinkInputPath();       //获取dbus接口的地址
//    cppckeck修改
//    void setAudioVolume(int volume);   //回设dock栏应用音量
    void setMute(bool muted);          //回设dock栏应用静音状态

private:
    DToolButton *m_pBtnChangeMute {nullptr};
    DLabel *m_pLabShowVolume {nullptr};
    DSlider *m_slider;
    Platform_MainWindow *_mw;
    QTimer m_autoHideTimer;
    bool m_bIsMute {false};
    bool m_bFinished {false};
    QPropertyAnimation *pVolAnimation {nullptr};
    State m_state {Close};
    QPoint m_point {0, 0};
   // QPixmap m_bgImage;
    bool m_mouseIn {false};
    int m_nVolume;                      //记录实际音量(实际音量最大值为200,显示最大到100)
    VolumeMonitoring volumeMonitoring;  //监听dock栏应用音量变化
    bool m_bHideWhenFinished;           ///等待动画结束后隐藏

    int m_iStep;                 //鼠标灵敏度的步进
    bool m_bIsWheel;             //是否是通过滚轮调节音量
};
}


#endif // Platform_VOLUMESLIDER_H
