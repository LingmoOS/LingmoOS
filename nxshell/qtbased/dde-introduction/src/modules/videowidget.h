// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "moduleinterface.h"
//#include "../dvideowidget.h"
#include "../widgets/coverphoto.h"

#include <QBitmap>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLocale>
#include <QMediaPlaylist>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>

#include <DClipEffectWidget>
#include <DLabel>
#include <DWidget>
#include <QPropertyAnimation>
#include <player_widget.h>
#include <player_engine.h>
#include <DSysInfo>
#include <QPainterPath>

#include <player_engine.h>
#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class QGraphicsOpacityEffect;
//播放视频类
class VideoWidget : public ModuleInterface {
    Q_OBJECT
public:
    explicit VideoWidget(bool autoPlay, QWidget *parent = nullptr);

    //更新第一次启动背景图
    void updateBigIcon() Q_DECL_OVERRIDE;
    //更新日常的背景图
    void updateSmallIcon() Q_DECL_OVERRIDE;
    void updateSelectBtnPos() Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    //根据视频的播放状态更新播放按钮的效果图片
    void updateControlButton();
    //播放按钮点击响应函数
    void onControlButtonClicked();
    //将当前动画暂停,下次播放从当前进度播放
    void stop();
    void updateInterface(QSize);
    //显示播放按钮
    void showVideoControlButton();

signals:
    void cancelCloseFrame();

public slots:
    //Add by ut001000 renfeixiang 2020-12-28 删除背景响应函数
    void deleteBackground();
    //add by ut002764 2021.6.2 恢复暂停，开始响应函数
    void PauseResetOrPlay();

protected:
    //鼠标进入事件,播放按钮显示,定时器停止
    void enterEvent(QEvent *) Q_DECL_OVERRIDE;
    //鼠标离开事件,定时器开始,播放按钮准备隐藏
    void leaveEvent(QEvent *) Q_DECL_OVERRIDE;
    //大小变化事件,大小变化时修改播放按钮的对应位置
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

private:
    void updateClip();

private:
    //视频
    dmr::PlayerWidget *m_video;
    //播放按钮
    DImageButton* m_control;
    DClipEffectWidget *m_clip;//这是什么功能??
    //播放按钮移动动画
    QPropertyAnimation *m_btnAni;
    //播放按钮隐藏动画
    QPropertyAnimation *m_hideAni;
    //播放按钮半透明属性
    QGraphicsOpacityEffect *m_hideEffect;
    //播放按钮隐藏的定时器
    QTimer *m_leaveTimer;
    //没使用
    QTimer *m_pauseTimer;
    //封面图片
    CoverPhoto *m_background;
    //没使用
    DLabel *m_label;
    bool m_load;
    bool m_first;
    bool m_isCommunity;
    QString m_strVideoCoverIcon;
};

#endif
