// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTERECORDBAR_H
#define VNOTERECORDBAR_H

#include <DFloatingMessage>

#include <QWidget>
#include <QStackedLayout>

DWIDGET_USE_NAMESPACE
class AudioWatcher;
class VNotePlayWidget;
class VNoteRecordWidget;
class VNote2SIconButton;
struct VNVoiceBlock;

class VNoteRecordBar : public QWidget
{
    Q_OBJECT
public:
    explicit VNoteRecordBar(QWidget *parent = nullptr);
    //停止录音
    void stopRecord();
    /**
     * @brief 播放语音
     * @param voiceData :语音信息
     * @param bIsSame :此次播放的语音是否与上一次操作的语音相同
     */
    void playVoice(VNVoiceBlock *voiceData, bool bIsSame);
    //停止播放
    void stopPlay();

private:
    //初始化设备检测
    void initAudioWatcher();
    //初始化参数配置
    void initSetting();
    //初始化UI布局
    void initUI();
    //连接槽函数
    void initConnections();
    //判断录音音量是否过低
    bool volumeToolow(const double &volume);

signals:
    //录音信号
    void sigStartRecord(const QString &recordPath);
    void sigFinshRecord(const QString &voicePath, qint64 voiceSize);
    //播放信号
    void sigPlayVoice(VNVoiceBlock *voiceData);
    void sigPauseVoice(VNVoiceBlock *voiceData);
    void sigWidgetClose(VNVoiceBlock *voiceData);
    //设备异常提示
    void sigDeviceExceptionMsgShow();
    void sigDeviceExceptionMsgClose();

public slots:
    //开始录音
    void onStartRecord();
    //结束录音
    void onFinshRecord(const QString &voicePath, qint64 voiceSize);
    //播放窗口关闭
    void onClosePlayWidget(VNVoiceBlock *voiceData);
    //设备音量改变
    void onAudioVolumeChange(int mode);
    //设备改变
    void onAudioDeviceChange(int mode);
    //输入/输出设备切换
    void onAudioSelectChange(QVariant value);
    void onChangeTheme();
    //控制中心中是否更改设备的使能状态
    void onDeviceEnableChanged(int mode,bool enabled);
protected:
    //事件过滤器
    bool eventFilter(QObject *o, QEvent *e) override;
    //开始录音
    void startRecord();

protected:
    QStackedLayout *m_mainLayout {nullptr};
    VNotePlayWidget *m_playPanel {nullptr};
    VNoteRecordWidget *m_recordPanel {nullptr};
    VNote2SIconButton *m_recordBtn {nullptr};
    QWidget *m_recordBtnHover {nullptr};

    QString m_recordPath {""};
    int m_currentMode {0};
    AudioWatcher *m_audioWatcher {nullptr};
    bool m_showVolumeWanning {false};
};

#endif // VNOTERECORDBAR_H
