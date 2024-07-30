/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "debug.h"
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIO/ApplicationLauncherJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KShell>
#include <QDebug>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <purpose/pluginbase.h>

Q_LOGGING_CATEGORY(PLUGIN_TELEGRAM, "kf.purpose.plugins.telegram")

class TelegramJob : public Purpose::Job
{
    Q_OBJECT
public:
    TelegramJob(QObject *parent)
        : Purpose::Job(parent)
    {
    }

    QList<QUrl> arrayToList(const QJsonArray &array)
    {
        QList<QUrl> ret;
        for (const QJsonValue &val : array) {
            QUrl url(val.toString());
            ret << url;
        }
        return ret;
    }

    void start() override
    {
        KService::Ptr service = KService::serviceByDesktopName(QStringLiteral("org.telegram.desktop"));

        if (!service) {
            service = KService::serviceByDesktopName(QStringLiteral("telegramdesktop"));
        }

        if (!service) {
            service = KService::serviceByDesktopName(QStringLiteral("telegram-desktop"));
        }

        if (!service) {
            // Failed to find the application
            QTimer::singleShot(0, this, [this]() {
                setError(KJob::UserDefinedError + 1);
                setErrorText(i18n("Could not find telegram"));
                setOutput({{QStringLiteral("url"), QString()}});
                emitResult();
            });
            return;
        }

        QString exec = service->exec();
        exec.replace(QLatin1String("%u"), QLatin1String("%f"));
        exec.replace(QLatin1String("@@u"), QLatin1String("@@"));
        exec.replace(QLatin1String(" -- "), QLatin1String(" -sendpath "));
        service->setExec(exec);

        auto *job = new KIO::ApplicationLauncherJob(service);
        job->setUrls(arrayToList(data().value(QLatin1String("urls")).toArray()));
        connect(job, &KIO::ApplicationLauncherJob::result, this, [this](KJob *job) {
            if (job->error()) {
                qWarning() << "telegram share error:" << job->error() << job->errorString();
                setError(job->error());
                setErrorText(job->errorString());
            } else {
                setOutput({{QStringLiteral("url"), QString()}});
            }

            emitResult();
        });
        job->start();
    }
};

class TelegramPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new TelegramJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(TelegramPlugin, "telegramplugin.json")

#include "telegramplugin.moc"
