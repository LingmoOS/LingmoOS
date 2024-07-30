/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <optional>

#include <kpipewire_export.h>

class QScreen;
struct zkde_screencast_unstable_v1;

namespace KWayland
{
namespace Client
{
class PlasmaWindow;
}
}

class ScreencastingPrivate;
class ScreencastingSourcePrivate;
class ScreencastingStreamPrivate;
class KPIPEWIRE_EXPORT ScreencastingStream : public QObject
{
    Q_OBJECT
public:
    ScreencastingStream(QObject *parent);
    ~ScreencastingStream() override;

    quint32 nodeId() const;

Q_SIGNALS:
    void created(quint32 nodeid);
    void failed(const QString &error);
    void closed();

private:
    friend class Screencasting;
    QScopedPointer<ScreencastingStreamPrivate> d;
};

class KPIPEWIRE_EXPORT Screencasting : public QObject
{
    Q_OBJECT
public:
    explicit Screencasting(QObject *parent = nullptr);
    ~Screencasting() override;

    enum CursorMode {
        Hidden = 1,
        Embedded = 2,
        Metadata = 4,
    };
    Q_ENUM(CursorMode);

    ScreencastingStream *createOutputStream(const QString &outputName, CursorMode mode);
    ScreencastingStream *createOutputStream(QScreen *screen, CursorMode mode);
    ScreencastingStream *createRegionStream(const QRect &region, qreal scaling, CursorMode mode);
    ScreencastingStream *createWindowStream(const QString &uuid, CursorMode mode);
    ScreencastingStream *createVirtualMonitorStream(const QString &name, const QSize &size, qreal scale, CursorMode mode);

    void destroy();

Q_SIGNALS:
    void initialized();
    void removed();
    void sourcesChanged();

private:
    QScopedPointer<ScreencastingPrivate> d;
};
