// Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AudioProcessingThread_H
#define AudioProcessingThread_H

#include <QThread>
#include <QMutex>
#include <QDebug>

#ifdef __cplusplus
extern "C" {
#endif
#include "camview.h"
#include "audio.h"
#include "gviewrender.h"
#ifdef __cplusplus
}
#endif

/**
 * @brief 音频处理线程
 */
class AudioProcessingThread : public QThread
{
    Q_OBJECT
public:
    AudioProcessingThread();

    ~AudioProcessingThread();

    /**
     * @brief stop 停止线程
     */
    void stop();

    /**
     * @brief init 线程初始化
     */
    void init();

    /**
     * @brief getStatus 获取状态
     */
    QAtomicInt getStatus()
    {
        return m_stopped;
    }

protected:
    /**
     * @brief run 运行线程
     */
    void run();

signals:
    void sendAudioProcessing(uchar* data, uint size);

public:
    QMutex            m_rwMtxData;
private:
    QAtomicInt        m_stopped;
    audio_buff_t      *m_auidoBuffer;
    uchar             *m_data;

    uint              m_datasize;
};

#endif // AudioProcessingThread_H
