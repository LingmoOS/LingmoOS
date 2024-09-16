// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VTEXTSPEECHANDTRMANAGER_H
#define VTEXTSPEECHANDTRMANAGER_H

class VTextSpeechAndTrManager
{
public:
    // 检测是否在朗读文本
    static bool isTextToSpeechInWorking();
    // 检测语音朗读开关是否打开
    static bool getTextToSpeechEnable();
    // 检测语音听写开关是否打开
    static bool getSpeechToTextEnable();
    // 检测文本翻译开关是否打开
    static bool getTransEnable();
    // 语音朗读
    static void onTextToSpeech();
    // 停止语音朗读
    static void onStopTextToSpeech();
    // 语音听写
    static void onSpeechToText();
    // 文本翻译
    static void onTextTranslate();
};

#endif // VTEXTSPEECHANDTRMANAGER_H
