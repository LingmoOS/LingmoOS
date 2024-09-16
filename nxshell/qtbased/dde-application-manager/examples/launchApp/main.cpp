// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QVariantMap>
#include <QDebug>

class Demo : public QObject
{
    Q_OBJECT
public:
    Demo()
        : ApplicationManager(u8"org.deepin.dde.ApplicationManager1",
                             u8"/org/deepin/dde/ApplicationManager1",
                             u8"org.desktopspec.ApplicationManager1")
        , JobManager(u8"org.deepin.dde.ApplicationManager1",
                     u8"/org/deepin/dde/ApplicationManager1/JobManager1",
                     u8"org.desktopspec.JobManager1")
    {
        auto con = JobManager.connection();
        if (!con.connect(JobManager.service(),
                         JobManager.path(),
                         JobManager.interface(),
                         u8"JobNew",
                         this,
                         SLOT(jobNewForward(QDBusObjectPath, QDBusObjectPath)))) {
            qFatal("connect JobNew failed.");
        }

        if (!con.connect(JobManager.service(),
                         JobManager.path(),
                         JobManager.interface(),
                         u8"JobRemoved",
                         this,
                         SLOT(jobRemovedForward(QDBusObjectPath, QString, QVariantList)))) {
            qFatal("connect JobNew failed.");
        }

        connect(this, &Demo::applicationLaunched, [](QList<QString> apps) {
            qInfo() << "application launched:";
            for (const auto &app : apps) {
                qInfo() << app;
            };
        });
    }

    void launchApp(const QString &appId)
    {
        connect(this, &Demo::amJobRemoved, [this](QDBusObjectPath job, QString status, QVariantList result) {
            if (myJob == job) {
                qInfo() << "my job" << status << result;
            }
            QList<QString> apps;
            for (const auto &app : result) {
                apps.append(app.value<QString>());
            }
            emit applicationLaunched(apps);
        });

        auto reply =
            ApplicationManager.callWithArgumentList(QDBus::Block, "Launch", {appId, QString{""}, QStringList{}, QVariantMap{}});

        if (reply.type() == QDBusMessage::ReplyMessage) {
            myJob = reply.arguments().first().value<QDBusObjectPath>();
        }
    }

public Q_SLOTS:
    void jobNewForward(QDBusObjectPath job, QDBusObjectPath source)
    {
        qInfo() << "Job New ["
                << "Job Path:" << job.path() << source.path() << "add this job].";
        emit amJobNew(job, source);
    }

    void jobRemovedForward(QDBusObjectPath job, QString status, QVariantList result)
    {
        qInfo() << "Job Removed ["
                << "Job Path:" << job.path() << "Job Status:" << status << "result:" << result;
        emit amJobRemoved(job, status, result);
    }

Q_SIGNALS:
    void amJobNew(QDBusObjectPath job, QDBusObjectPath source);
    void amJobRemoved(QDBusObjectPath job, QString status, QVariantList result);
    void applicationLaunched(QList<QString> apps);

private:
    QDBusInterface ApplicationManager;
    QDBusInterface JobManager;
    QDBusObjectPath myJob;
};

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};
    Demo demo;
    demo.launchApp("google-chrome");
    return QCoreApplication::exec();
}

#include "main.moc"
