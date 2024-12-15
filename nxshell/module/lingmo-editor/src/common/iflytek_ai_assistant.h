// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IFLYTEKAIASSISTANT_H
#define IFLYTEKAIASSISTANT_H

#include <QObject>

class QDBusInterface;

class IflytekAiAssistant : public QObject
{
    Q_OBJECT

public:
    static IflytekAiAssistant *instance();

    bool valid() const;
    void checkAiExists();

    enum CallRet {
        Invalid = -1,
        Disable = 0,
        Enable = 1,
    };

    // text to speech
    CallRet isTtsInWorking() const;
    CallRet isTtsEnable() const;
    void textToSpeech();
    bool stopTtsDirectly();

    // speech to text
    CallRet getIatEnable() const;
    void speechToText();

    // text translation
    CallRet getTransEnable() const;
    void textToTranslate();

    Q_SIGNAL void initFinished();

private:
    explicit IflytekAiAssistant(QObject *object = nullptr);
    ~IflytekAiAssistant() override = default;

    bool m_inited{false};
    bool m_valid{false};
};

#endif  // IFLYTEKAIASSISTANT_H
