/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCUPSJOB_H
#define KCUPSJOB_H

#include <KCupsConnection.h>
#include <kcupslib_export.h>

class KCUPSLIB_EXPORT KCupsJob
{
public:
    KCupsJob();
    KCupsJob(int jobId, const QString &printer);

    int id() const;
    QString idStr() const;
    QString name() const;
    QString originatingUserName() const;
    QString originatingHostName() const;
    QString printer() const;
    QDateTime createdAt() const;
    QDateTime completedAt() const;
    QDateTime processedAt() const;
    int pages() const;
    int processedPages() const;
    int size() const;
    bool preserved() const;

    static QString iconName(ipp_jstate_t state);
    ipp_jstate_t state() const;
    QString stateMsg() const;
    QString stateReason() const;
    QString holdUntil() const;

    bool authenticationRequired() const;
    bool reprintEnabled() const;

    static bool cancelEnabled(ipp_jstate_t state);
    static bool holdEnabled(ipp_jstate_t state);
    static bool releaseEnabled(ipp_jstate_t state);

protected:
    KCupsJob(const QVariantMap &arguments);

private:
    friend class KCupsRequest;

    int m_jobId;
    QString m_printer;
    QVariantMap m_arguments;
};

typedef QList<KCupsJob> KCupsJobs;
Q_DECLARE_METATYPE(KCupsJobs)
Q_DECLARE_METATYPE(KCupsJob)

#endif // KCUPSJOB_H
