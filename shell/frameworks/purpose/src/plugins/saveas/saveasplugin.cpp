/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <purpose/job.h>
#include <purpose/pluginbase.h>

#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QStandardPaths>

static QUrl addPathToUrl(const QUrl &url, const QString &relPath)
{
    QString path = url.path();
    if (!path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
    }
    path += relPath;
    QUrl u(url);
    u.setPath(path);
    return u;
}

class SaveAsShareJob : public Purpose::Job
{
    Q_OBJECT
public:
    SaveAsShareJob(QObject *parent)
        : Purpose::Job(parent)
    {
        setCapabilities(Killable | Suspendable);
    }

    void start() override
    {
        const QJsonArray inputUrls = data().value(QLatin1String("urls")).toArray();

        if (inputUrls.isEmpty()) {
            setErrorText(i18n("No URLs to save"));
            setError(1);
            emitResult();
            return;
        }

        QList<QUrl> urls;
        bool containsData = false;
        for (const QJsonValue &val : inputUrls) {
            urls.append(QUrl(val.toString()));
            containsData |= urls.last().scheme() == QLatin1String("data");
        }

        m_dest = QUrl(data().value(QLatin1String("destinationPath")).toString());
        if (containsData && !(urls.count() == 1 && m_dest.isLocalFile() && !QFileInfo(m_dest.toLocalFile()).isDir())) {
            for (const QUrl &url : urls) {
                QUrl dest = addPathToUrl(m_dest, QStringLiteral("data"));
                auto job = KIO::copy(url, dest);
                connect(job, &KJob::finished, this, &SaveAsShareJob::fileCopied);
                m_jobs.insert(job);
            }
        } else {
            auto job = KIO::copy(urls, m_dest);
            connect(job, &KJob::finished, this, &SaveAsShareJob::fileCopied);
            m_jobs.insert(job);
        }
    }

    bool doKill() override
    {
        bool killed = true;
        for (KJob *job : m_jobs) {
            killed &= job->kill();
        }
        return killed;
    }

    void fileCopied(KJob *job)
    {
        auto r = m_jobs.remove(job);
        Q_ASSERT(r);

        setError(error() + job->error());
        setErrorText(errorText() + QLatin1Char(' ') + job->errorText());
        if (job->error() == 0 && m_jobs.isEmpty()) {
            setOutput({{QStringLiteral("url"), m_dest.toString()}});
        }
        emitResult();
    }

private:
    QUrl m_dest;
    QSet<KJob *> m_jobs;
};

class SaveAsPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new SaveAsShareJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(SaveAsPlugin, "saveasplugin.json")

#include "saveasplugin.moc"
