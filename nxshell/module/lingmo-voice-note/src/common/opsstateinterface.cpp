// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opsstateinterface.h"
#include <DSysInfo>
#include <DLog>

DCORE_USE_NAMESPACE

/**
 * @brief OpsStateInterface::OpsStateInterface
 */
OpsStateInterface::OpsStateInterface()
{
    ;
}

/**
 * @brief OpsStateInterface::operState
 * @param type 操作目标
 * @param isSet 设置状态
 */
void OpsStateInterface::operState(int type, bool isSet)
{
    quint8 shift = static_cast<quint8>(type);

    if (shift > StateNone && shift < StateMax) {
        if (isSet) {
            m_states |= (1 << shift);
        } else {
            m_states &= (~(1 << shift));
        }
    } else {
        qCritical() << "Operation error:Invalid opsType =" << type;
    }
}

/**
 * @brief OpsStateInterface::isSearching
 * @return true 正在搜索
 */
bool OpsStateInterface::isSearching() const
{
    return (m_states & (1 << StateSearching));
}

/**
 * @brief OpsStateInterface::isRecording
 * @return true 正在录音
 */
bool OpsStateInterface::isRecording() const
{
    return (m_states & (1 << StateRecording));
}

/**
 * @brief OpsStateInterface::isPlaying
 * @return true 正在播放
 */
bool OpsStateInterface::isPlaying() const
{
    return (m_states & (1 << StatePlaying));
}

/**
 * @brief OpsStateInterface::isVoice2Text
 * @return true 正在语音转文字
 */
bool OpsStateInterface::isVoice2Text() const
{
    return (m_states & (1 << StateVoice2Text));
}

/**
 * @brief OpsStateInterface::isAppQuit
 * @return true 需要退出
 */
bool OpsStateInterface::isAppQuit() const
{
    return (m_states & (1 << StateAppQuit));
}

/**
 * @brief OpsStateInterface::isAiSrvExist
 * @return true 语音功能可用
 */
bool OpsStateInterface::isAiSrvExist() const
{
    return (m_states & (1 << StateAISrvAvailable));
}

/**
 * @brief OpsStateInterface::instance
 * @return 单例对象
 */
OpsStateInterface *OpsStateInterface::instance()
{
    static OpsStateInterface _instance;
    return &_instance;
}
