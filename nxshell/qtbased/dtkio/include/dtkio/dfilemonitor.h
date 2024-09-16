// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEMONITOR_H
#define DFILEMONITOR_H

#include <QObject>
#include <QUrl>
#include <QScopedPointer>

#include <DError>
#include <DExpected>

#include "dtkio_global.h"
#include "dtkiotypes.h"

DIO_BEGIN_NAMESPACE
class DFileMonitorPrivate;
class DFileMonitor : public QObject
{
    Q_OBJECT
public:
    explicit DFileMonitor(const QUrl &url, QObject *parent = nullptr);
    virtual ~DFileMonitor() override;

    DTK_CORE_NAMESPACE::DExpected<QUrl> url() const;
    DTK_CORE_NAMESPACE::DExpected<void> setTimeRate(quint32 msec);
    DTK_CORE_NAMESPACE::DExpected<quint32> timeRate() const;
    DTK_CORE_NAMESPACE::DExpected<void> setWatchType(WatchType type);
    DTK_CORE_NAMESPACE::DExpected<WatchType> watchType() const;

    DTK_CORE_NAMESPACE::DExpected<bool> running() const;
    DTK_CORE_NAMESPACE::DExpected<bool> start();
    DTK_CORE_NAMESPACE::DExpected<bool> start(quint32 timeRate = 200);

    bool stop();

    DTK_CORE_NAMESPACE::DError lastError() const;

Q_SIGNALS:
    void fileChanged(const QUrl &url);
    void fileDeleted(const QUrl &url);
    void fileAdded(const QUrl &url);
    void fileRenamed(const QUrl &fromUrl, const QUrl &toUrl);

private:
    QScopedPointer<DFileMonitorPrivate> d;
};
DIO_END_NAMESPACE

#endif   // DFILEMONITOR_H
