// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPSSTATEINTERFACE_H
#define OPSSTATEINTERFACE_H

#include <QtGlobal>

class OpsStateInterface
{
public:
    enum {
        StateNone = 0,
        StateSearching,
        StateRecording,
        StatePlaying,
        StateVoice2Text,
        StateAppQuit,
        StateAISrvAvailable,
        //Add other state at here
        StateMax,
    };

    static OpsStateInterface *instance();
    //根据操作类型设置对象状态
    void operState(int type, bool isSet);
    //是否正在搜索
    bool isSearching() const;
    //是否正在录音
    bool isRecording() const;
    //是否正在播放
    bool isPlaying() const;
    //是否正在语音转文字
    bool isVoice2Text() const;
    //应用是否需要退出
    bool isAppQuit() const;
    //后台语音服务是否存在，不存在则所有语音功能不可用
    bool isAiSrvExist() const;

protected:
    OpsStateInterface();
    quint64 m_states {StateNone};
};

#endif // OPSSTATEINTERFACE_H
