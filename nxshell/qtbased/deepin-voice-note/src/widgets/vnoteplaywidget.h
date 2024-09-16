// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEPLAYWIDGET_H
#define VNOTEPLAYWIDGET_H

#include "common/vlcpalyer.h"

#include <DFloatingWidget>
#include <DLabel>
#include <DSlider>
#include <DIconButton>
#include <DWidget>
#include <DFrame>

DWIDGET_USE_NAMESPACE

struct VNVoiceBlock;
class VNoteIconButton;
class VNote2SIconButton;

class QPainter;
class QWidget;
class VNotePlayWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit VNotePlayWidget(QWidget *parent = nullptr);
    //播放
    void playVoice(VNVoiceBlock *voiceData, bool bIsSame);
    //获取状态
    VlcPalyer::VlcState getPlayerStatus();
signals:
    void sigPlayVoice(VNVoiceBlock *voiceData);
    void sigPauseVoice(VNVoiceBlock *voiceData);
    void sigWidgetClose(VNVoiceBlock *voiceData);
public slots:
    //当前播放位置改变
    void onVoicePlayPosChange(qint64 pos);
    //进度条单击
    void onSliderPressed();
    //进度条单击释放
    void onSliderReleased();
    //进度条移动
    void onSliderMove(int pos);
    //播放/暂停
    void onPlayerBtnClicked();
    //播放结束，关闭播放窗口
    void onCloseBtnClicked();
    //播放文件总时长改变
    void onDurationChanged(qint64 duration);

protected:
    //事件过滤器
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void initUI();
    void initConnection();
    //初始化播放库
    void initPlayer();
    bool m_sliderReleased {true};
    DLabel *m_timeLab {nullptr};
    DLabel *m_nameLab {nullptr};
    DSlider *m_slider {nullptr};
    DFrame *m_sliderHover {nullptr};
    DIconButton *m_closeBtn {nullptr};
    VNVoiceBlock *m_voiceBlock {nullptr};
    VlcPalyer *m_player {nullptr};
    DIconButton *m_playerBtn {nullptr};
};

#endif // VNOTEPLAYWIDGET_H
