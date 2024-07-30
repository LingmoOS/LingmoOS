/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NOTIFYBYAUDIO_H
#define NOTIFYBYAUDIO_H

#include "knotificationplugin.h"

#include <QHash>
#include <QUrl>

#include <KConfigWatcher>

class KNotification;

struct ca_context;

class NotifyByAudio : public KNotificationPlugin
{
    Q_OBJECT

public:
    explicit NotifyByAudio(QObject *parent = nullptr);
    ~NotifyByAudio() override;

    QString optionName() override
    {
        return QStringLiteral("Sound");
    }
    void notify(KNotification *notification, const KNotifyConfig &notifyConfig) override;
    void close(KNotification *notification) override;

private Q_SLOTS:
    void finishCallback(uint32_t id, int error_code);

private:
    static void ca_finish_callback(ca_context *c, uint32_t id, int error_code, void *userdata);

    ca_context *context();
    void finishNotification(KNotification *notification, quint32 id);

    bool playSound(quint32 id, const QString &eventName, const QUrl &fallbackUrl);

    ca_context *m_context = nullptr;
    quint32 m_currentId = 0;
    QHash<quint32, KNotification *> m_notifications;
    // in case we loop we store the URL for the notification to be able to replay it
    QHash<quint32, std::pair<QString, QUrl>> m_loopSoundUrls;

    KConfigWatcher::Ptr m_settingsWatcher;
    QString m_soundTheme;
    bool m_enabled = true;
};

#endif
