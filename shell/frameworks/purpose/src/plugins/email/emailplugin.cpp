/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company info@kdab.com
    SPDX-FileContributor: Daniel Vrátil <daniel.vratil@kdab.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <purpose/pluginbase.h>

#include <KApplicationTrader>
#include <KEMailClientLauncherJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDebug>
#include <QDesktopServices>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

namespace
{
class EmailJob : public Purpose::Job
{
    Q_OBJECT
public:
    explicit EmailJob(QObject *parent = nullptr)
        : Purpose::Job(parent)
    {
    }

    void start() override
    {
        KEMailClientLauncherJob *job = new KEMailClientLauncherJob;

        QList<QUrl> attachments;
        QStringList bodyPieces;

        const auto urls = data().value(QLatin1String("urls")).toArray();
        for (const QJsonValue &val : urls) {
            const QUrl url = val.toVariant().toUrl();
            if (url.isLocalFile()) {
                attachments << url;
            } else {
                bodyPieces << url.toString();
            }
        }

        job->setAttachments(attachments);
        job->setBody(bodyPieces.join(QLatin1Char('\n')));
        job->setSubject(data().value(QLatin1String("title")).toString());

        connect(job, &KJob::result, this, [this](KJob *job) {
            setError(job->error());
            setErrorText(job->errorText());
            emitResult();
        });

        job->start();
    }
};

}

class EmailPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new EmailJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(EmailPlugin, "emailplugin.json")

#include "emailplugin.moc"
