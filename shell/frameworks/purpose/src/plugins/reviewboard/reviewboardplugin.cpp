/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QJsonArray>
#include <QStandardPaths>
#include <QUrl>

#include <KPluginFactory>

#include "reviewboardjobs.h"

#include "purpose/job.h"
#include "purpose/pluginbase.h"

class TheReviewboardJob : public Purpose::Job
{
    Q_OBJECT
public:
    TheReviewboardJob(QObject *object = nullptr)
        : Purpose::Job(object)
    {
    }

    void start() override
    {
        const QString baseDir(data().value(QLatin1String("baseDir")).toString());
        const QString repository(data().value(QLatin1String("repository")).toString());
        const QUrl sourceFile(data().value(QLatin1String("urls")).toArray().first().toString());
        const QString updateRR = data().value(QLatin1String("updateRR")).toString();
        const QJsonObject extraData = data().value(QLatin1String("extraData")).toObject();
        const QString username = data().value(QLatin1String("username")).toString();
        const QString password = data().value(QLatin1String("password")).toString();
        QUrl server(data().value(QLatin1String("server")).toString());
        server.setUserInfo(username + QLatin1Char(':') + password);

        KJob *job;
        if (!updateRR.isEmpty()) {
            job = new ReviewBoard::SubmitPatchRequest(server, sourceFile, baseDir, updateRR);
            connect(job, &KJob::finished, this, &TheReviewboardJob::reviewDone);
        } else {
            job = new ReviewBoard::NewRequest(server, repository);
            job->setProperty("extraData", extraData);
            connect(job, &KJob::finished, this, &TheReviewboardJob::reviewCreated);
        }
        job->setProperty("baseDir", baseDir);
        job->start();
    }

    void reviewCreated(KJob *j)
    {
        if (j->error() != 0) {
            setError(j->error());
            setErrorText(j->errorString());
            emitResult();
            return;
        }

        ReviewBoard::NewRequest const *job = qobject_cast<ReviewBoard::NewRequest *>(j);

        // This will provide things like groups and users for review from .reviewboardrc
        QVariantMap extraData = job->property("extraData").toMap();
        if (!extraData.isEmpty()) {
            KJob *updateJob = new ReviewBoard::UpdateRequest(job->server(), job->requestId(), extraData);
            updateJob->start();
        }

        // for git projects, m_source will be a VCSDiffPatchSource instance
        const QUrl sourceFile(data().value(QStringLiteral("urls")).toArray().first().toString());
        ReviewBoard::SubmitPatchRequest *submitPatchJob =
            new ReviewBoard::SubmitPatchRequest(job->server(), sourceFile, j->property("baseDir").toString(), job->requestId());
        connect(submitPatchJob, &ReviewBoard::SubmitPatchRequest::finished, this, &TheReviewboardJob::reviewDone);
        submitPatchJob->start();
    }

    void reviewDone(KJob *j)
    {
        if (j->error() != 0) {
            setError(j->error());
            setErrorText(j->errorString());
            emitResult();
            return;
        }

        ReviewBoard::SubmitPatchRequest const *job = qobject_cast<ReviewBoard::SubmitPatchRequest *>(j);
        QUrl url = job->server();
        url.setUserInfo(QString());
        QString requrl = QStringLiteral("%1/r/%2/").arg(url.toDisplayString(QUrl::PreferLocalFile), job->requestId());
        setOutput({{QStringLiteral("url"), requrl}});
        emitResult();
    }
};

class ReviewBoardPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new TheReviewboardJob;
    }
};

K_PLUGIN_CLASS_WITH_JSON(ReviewBoardPlugin, "reviewboardplugin.json")

#include "reviewboardplugin.moc"
