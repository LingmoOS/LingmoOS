#include "action.h"
#include <kauth/actionreply.h>
#include <kauth/executejob.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTemporaryFile>
#include <QTest>

#include <fcntl.h>
#include <unistd.h>

class FdTester : public QObject
{
    Q_OBJECT

public:
    FdTester(QObject *parent = nullptr)
        : QObject(parent)
        , m_busConnection(QDBusConnection::sessionBus())
    {
    }

    void initHandler(KAuth::Action &action, QVariantMap &actionArgs);

private Q_SLOTS:
    void testCase1();
    void cleanup();

private:
    QProcess *m_proc;
    QDBusConnection m_busConnection;
};

void FdTester::initHandler(KAuth::Action &action, QVariantMap &actionArgs)
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());

    // Write the arguments into temp file
    QByteArray blob;
    {
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream << actionArgs;
    }

    QCOMPARE(tempFile.write(blob), blob.length());
    action.setArguments(actionArgs);

    QDBusUnixFileDescriptor tempFileFd(tempFile.handle());
    tempFile.close();

    action.addArgument(QStringLiteral("tempFileDescriptor"), QVariant::fromValue(tempFileFd));

    qputenv("KAUTH_TEST_CALLER_ID", QDBusConnection::sessionBus().baseService().toUtf8());

    m_proc = new QProcess;
    QString helperExecutable = QStringLiteral(FDHELPEREXE);

    if (!QFile::exists(helperExecutable)) {
        QFAIL("Helper executable not found");
    }

    m_proc->start(helperExecutable);

    // Waiting for Helper to be started
    while (true) {
        if (m_busConnection.interface()->isServiceRegistered(QStringLiteral("org.kde.kf6auth.autotest"))) {
            break;
        }
    }
}

void FdTester::testCase1()
{
    QVariantMap actionArgs;
    KAuth::Action action(QStringLiteral("org.kde.kf6auth.autotest.standardaction"));
    action.setHelperId(QStringLiteral("org.kde.kf6auth.autotest"));
    actionArgs[QStringLiteral("Test value")] = 100;
    actionArgs[QStringLiteral("Test string")] = QStringLiteral("Hello World");
    initHandler(action, actionArgs);

    QVERIFY(action.isValid());
    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();

    bool rc = job->exec();
    if (!rc) {
        QString errorString = job->errorString();
        if (!errorString.isEmpty()) {
            qDebug() << "error" << errorString;
        }
    }
    QVERIFY(job->exec());

    QVERIFY(!job->error());
    QVERIFY(job->data().isEmpty());
}

void FdTester::cleanup()
{
    m_proc->kill();
    delete m_proc;
}

QTEST_MAIN(FdTester)
#include "FdTester.moc"
