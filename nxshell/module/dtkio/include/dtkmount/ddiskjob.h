// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDISKJOB_H
#define DDISKJOB_H

#include <DtkMountGlobal>

#include <QObject>

#include <DExpected>

DMOUNT_BEGIN_NAMESPACE

class DDiskJob;
namespace DDeviceManager {
DCORE_NAMESPACE::DExpected<DDiskJob *> createDiskJob(const QString &, QObject *);
}   // namespace DDeviceManager

class DDiskJobPrivate;
class DDiskJob : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DDiskJob)
    friend DCORE_NAMESPACE::DExpected<DDiskJob *> DDeviceManager::createDiskJob(const QString &, QObject *);

    Q_PROPERTY(QString path READ path CONSTANT FINAL)
    Q_PROPERTY(QStringList objects READ objects CONSTANT FINAL)
    Q_PROPERTY(bool cancelable READ cancelable CONSTANT FINAL)
    Q_PROPERTY(bool progressValid READ progressValid CONSTANT FINAL)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged FINAL)
    Q_PROPERTY(QString operation READ operation CONSTANT FINAL)
    Q_PROPERTY(quint32 startedByUid READ startedByUid CONSTANT FINAL)
    Q_PROPERTY(quint64 bytes READ bytes CONSTANT FINAL)
    Q_PROPERTY(quint64 expectedEndTime READ expectedEndTime NOTIFY expectedEndTimeChanged FINAL)
    Q_PROPERTY(quint64 rate READ rate NOTIFY rateChanged FINAL)
    Q_PROPERTY(quint64 startTime READ startTime CONSTANT FINAL)

public:
    ~DDiskJob() override;

    QString path() const;
    QStringList objects() const;
    bool cancelable() const;
    bool progressValid() const;
    double progress() const;
    QString operation() const;
    quint32 startedByUid() const;
    quint64 bytes() const;
    quint64 expectedEndTime() const;
    quint64 rate() const;
    quint64 startTime() const;

public Q_SLOTS:
    void cancel(const QVariantMap &options);

Q_SIGNALS:
    void completed(bool success, const QString &message);
    void progressChanged(double progress);
    void rateChanged(quint64 rate);
    void expectedEndTimeChanged(quint64 expectedEndTime);

protected:
    explicit DDiskJob(const QString &path, QObject *parent = nullptr);

private:
    QScopedPointer<DDiskJobPrivate> d_ptr;
};

DMOUNT_END_NAMESPACE

#endif   // DDISKJOB_H
