/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KPluginFactory>
#include <purpose/pluginbase.h>

#include <KIO/OpenUrlJob>

#include <QUrlQuery>

QList<QString> arrayToList(const QJsonArray &array)
{
    QList<QString> ret;
    for (const QJsonValue &val : array) {
        ret += val.toVariant().toString();
    }
    return ret;
}

class TwitterJob : public Purpose::Job
{
    Q_OBJECT
public:
    TwitterJob(QObject *parent)
        : Purpose::Job(parent)
    {
    }
    void start() override
    {
        const QList<QString> urls = arrayToList(data().value(QLatin1String("urls")).toArray());
        const QString text = data().value(QLatin1String("text")).toString();

        QUrlQuery query;

        if (!urls.isEmpty()) {
            query.addQueryItem(QStringLiteral("url"), urls.constFirst());
        }

        if (!text.isEmpty()) {
            query.addQueryItem(QStringLiteral("text"), text);
        }

        auto *job = new KIO::OpenUrlJob(QUrl(QStringLiteral("https://twitter.com/intent/tweet?") + query.toString()));
        connect(job, &KJob::finished, this, [this, job] {
            if (job->error()) {
                setError(job->error());
                setErrorText(job->errorText());
            }
            emitResult();
        });
        job->start();
    }
};

class TwitterPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new TwitterJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(TwitterPlugin, "twitterplugin.json")

#include "twitterplugin.moc"
