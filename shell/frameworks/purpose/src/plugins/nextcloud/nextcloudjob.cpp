/*
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "nextcloudjob.h"
#include <KAccounts/Core>
#include <KAccounts/GetCredentialsJob>
#include <KIO/CopyJob>
#include <QDebug>

QList<QUrl> arrayToList(const QJsonArray &array)
{
    QList<QUrl> ret;
    for (const QJsonValue &val : array) {
        ret += val.toVariant().toUrl();
    }
    return ret;
}

void NextcloudJob::start()
{
    const Accounts::AccountId id = data().value(QLatin1String("accountId")).toInt();
    auto credentialsJob = new KAccounts::GetCredentialsJob(id, this);

    connect(credentialsJob, &KAccounts::GetCredentialsJob::finished, this, &NextcloudJob::gotCredentials);

    credentialsJob->start();
}

void NextcloudJob::gotCredentials(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    const Accounts::AccountId id = data().value(QLatin1String("accountId")).toInt();
    Accounts::Account *acc = Accounts::Account::fromId(KAccounts::accountsManager(), id);

    const auto services = acc->services();
    for (const Accounts::Service &service : services) {
        if (service.name() == QStringLiteral("dav-storage")) {
            acc->selectService(service);
        }
    }

    KAccounts::GetCredentialsJob *credentialsJob = qobject_cast<KAccounts::GetCredentialsJob *>(job);
    Q_ASSERT(credentialsJob);
    const QString folder = data().value(QLatin1String("folder")).toString();

    QUrl destUrl;
    destUrl.setHost(acc->valueAsString(QStringLiteral("dav/host")));
    destUrl.setScheme(QStringLiteral("webdav"));
    destUrl.setPath(acc->valueAsString(QStringLiteral("dav/storagePath")) + folder);
    destUrl.setUserName(credentialsJob->credentialsData().value(QStringLiteral("UserName")).toString());
    destUrl.setPassword(credentialsJob->credentialsData().value(QStringLiteral("Secret")).toString());

    const QList<QUrl> sourceUrls = arrayToList(data().value(QLatin1String("urls")).toArray());

    KIO::CopyJob *copyJob = KIO::copy(sourceUrls, destUrl);

    connect(copyJob, &KIO::CopyJob::finished, this, [this, copyJob] {
        if (copyJob->error()) {
            setError(copyJob->error());
            setErrorText(copyJob->errorText());
        }
        emitResult();
    });

    copyJob->start();
}

#include "moc_nextcloudjob.cpp"
