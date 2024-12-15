// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VOICEVOLUMEWATCHER_H
#define VOICEVOLUMEWATCHER_H
#include "audioutils.h"

#include <QTimer>

/**
 * @brief The voiceVolumeWatcher class 音频监听类
 */
class voiceVolumeWatcher : public QObject
{
    Q_OBJECT
public:
    explicit voiceVolumeWatcher(QObject *parent = nullptr);
    ~voiceVolumeWatcher();
    /**
     * @brief setWatch  设置是否打开音频监听
     * @param isWatcher
     */
    void setWatch(const bool isWatcher);
    void setIsRecoding(bool value);
    // 将原来的run()方法改为定时器的槽函数，便于截图快速退出
    // 取消之前的线程方式，采用定时器监测
    void slotvoiceVolumeWatcher();
    /**
     * @brief 返回当前是否有系统声卡
     * @return
     */
    bool getystemAudioState();
protected:
    void onCardsChanged(const QString &value);
    void initAvailInputPorts(const QString &cards);
    bool isMicrophoneAvail(const QString &activePort) const;

signals:
    void sigRecodeState(bool couldUse);
    /* void inputSourceChanged(const QString &name); */

private:

    AudioUtils *m_audioUtils;

    struct Port {
        QString  portId;
        QString portName;
        QString cardName;
        int     available {1};
        int     cardId;
        bool    isActive {false};

        bool isInputPort() const;
        bool isLoopback() const;
    };
    friend QDebug &operator << (QDebug &out, const Port &port);

    //All available input ports.except loopback port.
    QMap<QString, Port> m_availableInputPorts;
    //bool m_isRecoding;
    bool m_coulduse;

    /**
     * @brief m_watchTimer 麦克风定时检测器
     */
    QTimer *m_watchTimer = nullptr;
    /**
     * @brief 是否存在系统声卡 默认存在
     */
    bool m_isExistSystemAudio ;
};


#endif // VOICEVOLUMEWATCHER_H
