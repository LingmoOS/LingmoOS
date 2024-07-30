/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRUNNER_ABSTRACTRUNNERTEST_H
#define KRUNNER_ABSTRACTRUNNERTEST_H

#include <KPluginMetaData>
#include <KRunner/AbstractRunner>
#include <KRunner/RunnerManager>
#include <QStandardPaths>

#include <QSignalSpy>
#include <QTest>
#if KRUNNER_DBUS_RUNNER_TESTING
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QProcess>
#include <QTimer>
#endif

namespace KRunner
{
/**
 * This class provides a basic structure for a runner test.
 * The compile definitions should be configured using the `krunner_configure_test` cmake macro
 * @since 5.80
 */
class AbstractRunnerTest : public QObject
{
public:
    using QObject::QObject;
    std::unique_ptr<KRunner::RunnerManager> manager = nullptr;
    KRunner::AbstractRunner *runner = nullptr;

    /**
     * Load the runner and set the manager and runner properties.
     */
    void initProperties()
    {
        qputenv("LC_ALL", "C.utf-8");
        manager.reset(new KRunner::RunnerManager());

#if KRUNNER_DBUS_RUNNER_TESTING
        auto md = manager->convertDBusRunnerToJson(QStringLiteral(KRUNNER_TEST_DESKTOP_FILE));
        QVERIFY(md.isValid());
        manager->loadRunner(md);
#else
        const QString pluginId = QFileInfo(QStringLiteral(KRUNNER_TEST_RUNNER_PLUGIN_NAME)).completeBaseName();
        auto metaData = KPluginMetaData::findPluginById(QStringLiteral(KRUNNER_TEST_RUNNER_PLUGIN_DIR), pluginId);
        QVERIFY2(metaData.isValid(),
                 qPrintable(QStringLiteral("Could not find plugin %1 in folder %2").arg(pluginId, QStringLiteral(KRUNNER_TEST_RUNNER_PLUGIN_DIR))));

        // Set internal variables
        manager->loadRunner(metaData);
#endif
        QCOMPARE(manager->runners().count(), 1);
        runner = manager->runners().constFirst();

        // Just make sure all went well
        QVERIFY(runner);
    }

    /**
     * Launch a query and wait for the RunnerManager to finish
     * @param query
     * @param runnerName
     * @return matches of the current query
     */
    QList<QueryMatch> launchQuery(const QString &query, const QString &runnerName = QString())
    {
        QSignalSpy spy(manager.get(), &KRunner::RunnerManager::queryFinished);
        manager->launchQuery(query, runnerName);
        if (!QTest::qVerify(spy.wait(), "spy.wait()", "RunnerManager did not emit the queryFinished signal", __FILE__, __LINE__)) {
            return {};
        }
        return manager->matches();
    }
#if KRUNNER_DBUS_RUNNER_TESTING
    /**
     * Launch the configured DBus executable with the given arguments and wait for the process to be started.
     * @param args
     * @param waitForService Wait for this service to be registered, this will default to the service from the metadata
     * @return Process that was successfully started
     */
    QProcess *startDBusRunnerProcess(const QStringList &args = {}, const QString waitForService = QString())
    {
        qputenv("LC_ALL", "C.utf-8");
        QProcess *process = new QProcess();
        auto md = manager->convertDBusRunnerToJson(QStringLiteral(KRUNNER_TEST_DESKTOP_FILE));
        QString serviceToWatch = waitForService;
        if (serviceToWatch.isEmpty()) {
            serviceToWatch = md.value(QStringLiteral("X-Plasma-DBusRunner-Service"));
        }
        QEventLoop loop;
        // Wait for the service to show up. Same logic as the dbusrunner
        connect(QDBusConnection::sessionBus().interface(),
                &QDBusConnectionInterface::serviceOwnerChanged,
                &loop,
                [&loop, serviceToWatch](const QString &serviceName, const QString &, const QString &newOwner) {
                    if (serviceName == serviceToWatch && !newOwner.isEmpty()) {
                        loop.quit();
                    }
                });

        // Otherwise, we just wait forever without any indication what we are waiting for
        QTimer::singleShot(10000, &loop, [&loop, process]() {
            loop.quit();

            if (process->state() == QProcess::ProcessState::NotRunning) {
                qWarning() << "stderr of" << KRUNNER_TEST_DBUS_EXECUTABLE << "is:";
                qWarning().noquote() << process->readAllStandardError();
            }
            Q_ASSERT_X(false, "AbstractRunnerTest::startDBusRunnerProcess", "DBus service was not registered within 10 seconds");
        });
        process->start(QStringLiteral(KRUNNER_TEST_DBUS_EXECUTABLE), args);
        loop.exec();
        process->waitForStarted(5);

        Q_ASSERT(process->state() == QProcess::ProcessState::Running);
        m_runningProcesses << process;
        return process;
    }

    /**
     * Kill all processes that got started with the startDBusRunnerProcess
     */
    void killRunningDBusProcesses()
    {
        for (auto &process : std::as_const(m_runningProcesses)) {
            process->kill();
            QVERIFY(process->waitForFinished());
            if (QTest::currentTestFailed()) {
                qWarning().noquote() << "Output from " << process->program() << ": " << process->readAll();
            }
        }
        qDeleteAll(m_runningProcesses);
        m_runningProcesses.clear();
    }

private:
    QList<QProcess *> m_runningProcesses;
#endif
};
}

#endif
