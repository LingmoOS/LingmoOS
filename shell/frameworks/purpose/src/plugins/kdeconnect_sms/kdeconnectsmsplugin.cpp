/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
#include <purpose/pluginbase.h>

class KDEConnectSMSJob : public Purpose::Job
{
    Q_OBJECT
public:
    KDEConnectSMSJob(QObject *parent)
        : Purpose::Job(parent)
    {
    }

    QStringList arrayToList(const QJsonArray &array)
    {
        QStringList ret;
        for (const QJsonValue &val : array) {
            ret += val.toString();
        }
        return ret;
    }

    void start() override
    {
        QJsonArray urlsJson = data().value(QLatin1String("urls")).toArray();
        QString title = data().value(QLatin1String("title")).toString();
        QString message = i18n("%1 - %2").arg(title).arg(arrayToList(urlsJson).join(QLatin1Char(' ')));

        auto *job = new KIO::CommandLauncherJob(QStringLiteral("kdeconnect-sms"), {QStringLiteral("--message"), message});
        job->setDesktopName(QStringLiteral("org.kde.kdeconnect.sms"));
        connect(job, &KJob::finished, this, &KDEConnectSMSJob::emitResult);
        job->start();
    }
};

class KDEConnectSMSPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new KDEConnectSMSJob(nullptr);
    }
};

K_PLUGIN_FACTORY_WITH_JSON(KDEConnectSMS, "kdeconnectsmsplugin.json", registerPlugin<KDEConnectSMSPlugin>();)

#include "kdeconnectsmsplugin.moc"
