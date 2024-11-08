/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef SCREENCASTINGREQUEST_H
#define SCREENCASTINGREQUEST_H

#include "screen-casting.h"
#include <QObject>

class ScreencastingStream;

class ScreenCastingRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
    Q_PROPERTY(quint32 nodeId READ nodeId NOTIFY nodeIdChanged)

public:
    explicit ScreenCastingRequest(QObject *parent = nullptr);
    ~ScreenCastingRequest();

    void setUuid(const QString &uuid);
    QString uuid() const;
    quint32 nodeId() const;

    void create(Screencasting *screencasting);
Q_SIGNALS:
    void nodeIdChanged(quint32 nodeId);
    void uuidChanged(const QString &uuid);
    void closeRunningStreams();
    void cursorModeChanged(Screencasting::CursorMode cursorMode);

private:
    void setNodeid(uint nodeId);

    ScreencastingStream *m_stream = nullptr;
    QString m_uuid;
    KWayland::Client::Output *m_output = nullptr;
    quint32 m_nodeId = 0;
};

#endif // SCREENCASTINGREQUEST_H
