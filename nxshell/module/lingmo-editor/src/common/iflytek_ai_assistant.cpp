// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iflytek_ai_assistant.h"

#include <mutex>

#include <QtConcurrent/QtConcurrentRun>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDebug>

IflytekAiAssistant::IflytekAiAssistant(QObject *parent)
    : QObject(parent)
{
}

IflytekAiAssistant *IflytekAiAssistant::instance()
{
    static IflytekAiAssistant ins;
    return &ins;
}

bool IflytekAiAssistant::valid() const
{
    return m_inited && m_valid;
}

IflytekAiAssistant::CallRet IflytekAiAssistant::isTtsInWorking() const
{
    if (!valid()) {
        return Invalid;
    }

    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/tts", "com.iflytek.aiassistant.tts");
    interface.setTimeout(100);

    QDBusReply<bool> ret = interface.call("isTTSInWorking");
    if (ret.isValid()) {
        return ret.value() ? Enable : Disable;
    }
    return Invalid;
}

IflytekAiAssistant::CallRet IflytekAiAssistant::isTtsEnable() const
{
    if (!valid()) {
        return Invalid;
    }

    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/tts", "com.iflytek.aiassistant.tts");
    interface.setTimeout(100);

    QDBusReply<bool> ret = interface.call("getTTSEnable");
    if (ret.isValid()) {
        return ret.value() ? Enable : Disable;
        ;
    }
    return Invalid;
}

void IflytekAiAssistant::textToSpeech()
{
    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/lingmomain", "com.iflytek.aiassistant.mainWindow");
    interface.asyncCall("TextToSpeech");
}

bool IflytekAiAssistant::stopTtsDirectly()
{
    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/tts", "com.iflytek.aiassistant.tts");
    auto ret = interface.asyncCall("stopTTSDirectly");
    return ret.isValid();
}

IflytekAiAssistant::CallRet IflytekAiAssistant::getIatEnable() const
{
    if (!valid()) {
        return Invalid;
    }

    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/iat", "com.iflytek.aiassistant.iat");
    interface.setTimeout(100);

    QDBusReply<bool> ret = interface.call("getIatEnable");
    if (ret.isValid()) {
        return ret.value() ? Enable : Disable;
    }
    return Invalid;
}

void IflytekAiAssistant::speechToText()
{
    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/lingmomain", "com.iflytek.aiassistant.mainWindow");
    interface.asyncCall("SpeechToText");
}

IflytekAiAssistant::CallRet IflytekAiAssistant::getTransEnable() const
{
    if (!valid()) {
        return Invalid;
    }

    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/trans", "com.iflytek.aiassistant.trans");
    interface.setTimeout(100);

    QDBusReply<bool> ret = interface.call("getTransEnable");
    if (ret.isValid()) {
        return ret.value() ? Enable : Disable;
    }
    return Invalid;
}

void IflytekAiAssistant::textToTranslate()
{
    QDBusInterface interface("com.iflytek.aiassistant", "/aiassistant/lingmomain", "com.iflytek.aiassistant.mainWindow");
    interface.asyncCall("TextToTranslate");
}

void IflytekAiAssistant::checkAiExists()
{
    static std::once_flag kAiFlag;
    std::call_once(kAiFlag, [this]() {
        QtConcurrent::run([this]() {
            QDBusConnection connection = QDBusConnection::sessionBus();
            QDBusConnectionInterface *bus = connection.interface();
            bool isVailid = bus->isServiceRegistered("com.iflytek.aiassistant");

            // call on non-gui thread, so queued connection.
            QMetaObject::invokeMethod(
                this,
                [isVailid, this]() {
                    this->m_inited = true;
                    this->m_valid = isVailid;

                    Q_EMIT initFinished();
                },
                Qt::QueuedConnection);
        });
    });
}
