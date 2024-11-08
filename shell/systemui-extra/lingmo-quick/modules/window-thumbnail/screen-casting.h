/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef SCREENCASTING_H
#define SCREENCASTING_H
#include <QObject>
#include <QSharedPointer>
#include <QVector>
#include <optional>

struct zkde_screencast_unstable_v1;

namespace KWayland
{
namespace Client
{
class PlasmaWindow;
class Registry;
class Output;
}
}

class ScreencastingPrivate;
class ScreencastingStreamPrivate;
class ScreencastingStream : public QObject
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

class Screencasting : public QObject
{
    Q_OBJECT
public:
    explicit Screencasting(QObject *parent = nullptr);
    explicit Screencasting(KWayland::Client::Registry *registry, int id, int version, QObject *parent = nullptr);
    ~Screencasting() override;

    enum CursorMode {
        Hidden = 1,
        Embedded = 2,
        Metadata = 4,
    };
    Q_ENUM(CursorMode);


    ScreencastingStream *createWindowStream(const QString &uuid, CursorMode mode);

    void setup(zkde_screencast_unstable_v1 *screencasting);
    void destroy();

private:
    QScopedPointer<ScreencastingPrivate> d;

};

#endif // SCREENCASTING_H
