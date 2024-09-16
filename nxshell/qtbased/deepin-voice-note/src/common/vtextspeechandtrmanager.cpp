// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vtextspeechandtrmanager.h"

#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>
#include <QProcess>

static bool isSpeeching = false;

/**
 * @brief VTextSpeechAndTrManager::isTextToSpeechInWorking
 * @return true 正在朗读
 */
bool VTextSpeechAndTrManager::isTextToSpeechInWorking()
{
    if (isSpeeching) {
        QDBusMessage stopReadingMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                     "/aiassistant/tts",
                                                                     "com.iflytek.aiassistant.tts",
                                                                     "isTTSInWorking");

        QDBusReply<bool> stopReadingStateRet = QDBusConnection::sessionBus().call(stopReadingMsg, QDBus::BlockWithGui);
        if (stopReadingStateRet.isValid()) {
            isSpeeching = stopReadingStateRet.value();
        } else {
            isSpeeching = false;
        }
    }

    return isSpeeching;
}

/**
 * @brief VTextSpeechAndTrManager::getTextToSpeechEnable
 * @return true 朗读功能可用
 */
bool VTextSpeechAndTrManager::getTextToSpeechEnable()
{
    QDBusMessage voiceReadingMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                  "/aiassistant/tts",
                                                                  "com.iflytek.aiassistant.tts",
                                                                  "getTTSEnable");

    QDBusReply<bool> voiceReadingStateRet = QDBusConnection::sessionBus().call(voiceReadingMsg, QDBus::BlockWithGui);
    if (voiceReadingStateRet.isValid()) {
        return voiceReadingStateRet.value();
    } else {
        return false;
    }
}

/**
 * @brief VTextSpeechAndTrManager::getSpeechToTextEnable
 * @return true 听写功能可用
 */
bool VTextSpeechAndTrManager::getSpeechToTextEnable()
{
    QDBusMessage dictationMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                               "/aiassistant/iat",
                                                               "com.iflytek.aiassistant.iat",
                                                               "getIatEnable");

    QDBusReply<bool> dictationStateRet = QDBusConnection::sessionBus().call(dictationMsg, QDBus::BlockWithGui);
    if (dictationStateRet.isValid()) {
        return dictationStateRet.value();
    } else {
        return false;
    }
}

/**
 * @brief VTextSpeechAndTrManager::getTransEnable
 * @return true 翻译功能可用
 */
bool VTextSpeechAndTrManager::getTransEnable()
{
    QDBusMessage translateReadingMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                      "/aiassistant/trans",
                                                                      "com.iflytek.aiassistant.trans",
                                                                      "getTransEnable");

    QDBusReply<bool> translateStateRet = QDBusConnection::sessionBus().call(translateReadingMsg, QDBus::BlockWithGui);
    if (translateStateRet.isValid()) {
        return translateStateRet.value();
    } else {
        return false;
    }
}

/**
 * @brief VTextSpeechAndTrManager::onTextToSpeech
 */
void VTextSpeechAndTrManager::onTextToSpeech()
{
    QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/deepinmain com.iflytek.aiassistant.mainWindow.TextToSpeech");
    isSpeeching = true;
}

/**
 * @brief VTextSpeechAndTrManager::onStopTextToSpeech
 */
void VTextSpeechAndTrManager::onStopTextToSpeech()
{
    if (isSpeeching) {
        QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/tts com.iflytek.aiassistant.tts.stopTTSDirectly");
        isSpeeching = false;
    }
}

/**
 * @brief VTextSpeechAndTrManager::onSpeechToText
 */
void VTextSpeechAndTrManager::onSpeechToText()
{
    QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/deepinmain com.iflytek.aiassistant.mainWindow.SpeechToText");
}

/**
 * @brief VTextSpeechAndTrManager::onTextTranslate
 */
void VTextSpeechAndTrManager::onTextTranslate()
{
    QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/deepinmain com.iflytek.aiassistant.mainWindow.TextToTranslate");
}
