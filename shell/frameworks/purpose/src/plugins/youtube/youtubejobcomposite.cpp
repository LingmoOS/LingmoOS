/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "youtubejobcomposite.h"
#include "youtubejob.h"
#include <Accounts/Application>
#include <Accounts/Manager>
#include <KAccounts/Core>
#include <KAccounts/GetCredentialsJob>
#include <KLocalizedString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QStandardPaths>

QDebug operator<<(QDebug s, const Accounts::Service &service)
{
    s.nospace() << qPrintable(service.displayName()) << ',' << qPrintable(service.name()) << '\n';
    return s;
}
QDebug operator<<(QDebug s, const Accounts::Provider &provider)
{
    s.nospace() << "Provider(" << qPrintable(provider.displayName()) << ',' << qPrintable(provider.name()) << ")\n";
    return s;
}

YoutubeJobComposite::YoutubeJobComposite()
    : Purpose::Job()
{
}

void YoutubeJobComposite::start()
{
    const QJsonValue jsonId = data().value(QLatin1String("accountId"));
    if (jsonId.isNull() || jsonId.isUndefined()) {
        setError(1);
        setErrorText(i18n("No YouTube account configured in your accounts."));
        emitResult();
        return;
    }
    const Accounts::AccountId id = jsonId.toInt();

    // TODO: make async
    QByteArray accessToken;
    {
        auto job = new KAccounts::GetCredentialsJob(id, this);
        bool b = job->exec();
        if (!b) {
            qWarning() << "Couldn't fetch credentials";
            setError(job->error());
            setErrorText(job->errorText());
            emitResult();
            return;
        }
        accessToken = job->credentialsData().value(QStringLiteral("AccessToken")).toByteArray();
    }

    m_pendingJobs = 0;
    const QJsonArray urls = data().value(QLatin1String("urls")).toArray();
    for (const QJsonValue &url : urls) {
        YoutubeJob *job = new YoutubeJob(QUrl(url.toString()),
                                         accessToken,
                                         data().value(QLatin1String("videoTitle")).toString(),
                                         data().value(QLatin1String("videoTags")).toString().split(QLatin1Char(',')),
                                         data().value(QLatin1String("videoDesc")).toString(),
                                         this);
        connect(job, &KJob::finished, this, &YoutubeJobComposite::subjobFinished);
        job->start();
        m_pendingJobs++;
    }
}

void YoutubeJobComposite::subjobFinished(KJob *subjob)
{
    m_pendingJobs--;
    if (subjob->error()) {
        setError(subjob->error());
        setErrorText(subjob->errorText());
        emitResult();
        return;
    }
    if (m_pendingJobs == 0) {
        if (!error()) {
            const QJsonValue url = qobject_cast<YoutubeJob *>(subjob)->outputUrl();
            setOutput({{QStringLiteral("url"), url.toString()}});
        }
        emitResult();
    }
}
