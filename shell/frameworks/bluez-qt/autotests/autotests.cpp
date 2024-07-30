/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "autotests.h"
#include "adapter.h"
#include "battery.h"
#include "bluezqt_dbustypes.h"
#include "device.h"
#include "gattserviceremote.h"
#include "gattcharacteristicremote.h"
#include "gattdescriptorremote.h"
#include "mediaplayer.h"
#include "mediaplayertrack.h"
#include "mediatransport.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>

#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

QProcess *FakeBluez::s_process = nullptr;

class StartJob : public QObject
{
    Q_OBJECT

public:
    explicit StartJob();

    void exec();

    bool foundFakeBluez() const
    {
        return !m_fakebluezPath.isEmpty();
    }

private Q_SLOTS:
    void processError(QProcess::ProcessError error);
    void processFinished(int code, QProcess::ExitStatus status);

private:
    QString m_fakebluezPath;
    QEventLoop m_eventLoop;
};

StartJob::StartJob()
    : QObject(nullptr)
    , m_fakebluezPath(QCoreApplication::applicationDirPath() + QStringLiteral("/fakebluez"))
{
}

void StartJob::exec()
{
    QDBusServiceWatcher watcher(QStringLiteral("org.kde.bluezqt.test"), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForRegistration);

    connect(&watcher, &QDBusServiceWatcher::serviceRegistered, &m_eventLoop, &QEventLoop::quit);
    connect(FakeBluez::s_process, &QProcess::errorOccurred, this, &StartJob::processError);
    connect(FakeBluez::s_process, &QProcess::finished, this, &StartJob::processFinished);

    FakeBluez::s_process->start(m_fakebluezPath, QStringList());

    m_eventLoop.exec();
}

void StartJob::processError(QProcess::ProcessError error)
{
    QString errorString;
    switch (error) {
    case QProcess::FailedToStart:
        errorString = QStringLiteral("Failed to Start");
        break;
    case QProcess::Crashed:
        errorString = QStringLiteral("Crashed");
        break;
    case QProcess::Timedout:
        errorString = QStringLiteral("Timedout");
        break;
    default:
        errorString = QStringLiteral("Unknown");
    }

    qWarning() << "Fakebluez binary:" << m_fakebluezPath;
    qWarning() << "Process error:" << error << errorString;
    qWarning() << "Error output:" << FakeBluez::s_process->readAllStandardError();

    m_eventLoop.quit();
}

void StartJob::processFinished(int code, QProcess::ExitStatus status)
{
    QString statusString = status == QProcess::NormalExit ? QStringLiteral("Normal Exit") : QStringLiteral("Crash Exit");

    qWarning() << "Fakebluez binary:" << m_fakebluezPath;
    qWarning() << "Process finished:" << code << statusString;
    qWarning() << "Error output:" << FakeBluez::s_process->readAllStandardError();

    m_eventLoop.quit();
}

void FakeBluez::start()
{
    if (isRunning()) {
        return;
    }
    if (!s_process) {
        s_process = new QProcess();
    }

    StartJob job;
    QVERIFY(job.foundFakeBluez());
    job.exec();
}

void FakeBluez::stop()
{
    if (s_process && s_process->state() == QProcess::Running) {
        s_process->terminate();
        s_process->waitForFinished();
    }
}

bool FakeBluez::isRunning()
{
    return s_process && s_process->state() == QProcess::Running;
}

void FakeBluez::runTest(const QString &testName)
{
    if (!isRunning()) {
        return;
    }

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.bluezqt.test"),
                                                       QStringLiteral("/"),
                                                       QStringLiteral("org.kde.bluezqt.fakebluez.Test"),
                                                       QStringLiteral("runTest"));

    call << testName;
    QDBusConnection::sessionBus().call(call);
}

void FakeBluez::runAction(const QString &object, const QString &actionName, const QVariantMap &properties)
{
    if (!isRunning()) {
        return;
    }

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.bluezqt.test"),
                                                       QStringLiteral("/"),
                                                       QStringLiteral("org.kde.bluezqt.fakebluez.Test"),
                                                       QStringLiteral("runAction"));

    call << object;
    call << actionName;
    call << properties;

    QDBusConnection::sessionBus().call(call);
}

void Autotests::registerMetatypes()
{
    qRegisterMetaType<BluezQt::GattDescriptorRemotePtr>("GattDescriptorRemotePtr");
    qRegisterMetaType<BluezQt::GattCharacteristicRemotePtr>("GattCharacteristicRemotePtr");
    qRegisterMetaType<BluezQt::GattServiceRemotePtr>("GattServiceRemotePtr");
    qRegisterMetaType<BluezQt::DevicePtr>("DevicePtr");
    qRegisterMetaType<BluezQt::AdapterPtr>("AdapterPtr");
    qRegisterMetaType<BluezQt::BatteryPtr>("BatteryPtr");
    qRegisterMetaType<BluezQt::MediaPlayerPtr>("MediaPlayerPtr");
    qRegisterMetaType<BluezQt::MediaTransportPtr>("MediaTransportPtr");
}

void Autotests::verifyPropertiesChangedSignal(const QSignalSpy &spy, const QString &propertyName, const QVariant &propertyValue)
{
    int changes = 0;

    for (int i = 0; i < spy.count(); ++i) {
        QList<QVariant> arguments = spy.at(i);
        QVariantMap properties = arguments.at(1).toMap();

        QVariantMap::const_iterator it;
        for (it = properties.constBegin(); it != properties.constEnd(); ++it) {
            const QVariant &changedValue = it.value();
            const QString &property = it.key();
            if (property == propertyName && changedValue == propertyValue) {
                changes++;
            }
        }
    }

    QCOMPARE(changes, 1);
}

#include "autotests.moc"
