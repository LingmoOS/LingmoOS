// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DUDISKSJOB_H
#define DUDISKSJOB_H

#include <QObject>

class DUDisksJobPrivate;
class DUDisksJob : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DUDisksJob)

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
    ~DUDisksJob();
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
    void completed(bool success, QString message);
    void progressChanged(double progress);
    void rateChanged(quint64 rate);
    void expectedEndTimeChanged(quint64 expectedEndTime);

private:
    QScopedPointer<DUDisksJobPrivate> d_ptr;

    explicit DUDisksJob(QString path, QObject *parent = nullptr);

private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &changed_properties);

    friend class DDiskManager;
};
#endif
