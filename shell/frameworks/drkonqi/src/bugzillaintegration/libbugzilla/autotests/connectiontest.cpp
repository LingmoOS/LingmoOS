/*
    SPDX-FileCopyrightText: 2019-2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDebug>
#include <QMutex>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTest>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

#include "../connection.h"

namespace Bugzilla
{
class ConnectionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase()
    {
    }

    void testDefaultRoot()
    {
        // Make sure the default root is well formed.
        // This talks to bugzilla directly! To avoid flakeyness the actual
        // HTTP interaction is qwaiting and retrying a bunch of times.
        // Obviously still not ideal.
        Bugzilla::HTTPConnection c;
        QVERIFY(c.root().toString().endsWith("/rest"));
        QVERIFY(QTest::qWaitFor(
            [&]() {
                APIJob *job = c.get("/version");
                job->exec();
                try {
                    job->document();
                } catch (Bugzilla::Exception &e) {
                    QTest::qSleep(1000);
                    return false;
                }

                return true;
            },
            5000));
    }

    void testGet()
    {
        qDebug() << Q_FUNC_INFO;
        // qhttpserver is still in qt-labs. as a simple solution do some dumb
        // http socketing.
        QTcpServer t;
        QCOMPARE(t.listen(QHostAddress::LocalHost, 0), true);
        connect(&t, &QTcpServer::newConnection, &t, [&t]() {
            QTcpSocket *socket = t.nextPendingConnection();
            socket->waitForReadyRead();
            QString httpBlob = socket->readAll();
            qDebug() << httpBlob;
            // The query is important to see if this actually gets properly
            // passed along!
            // Reason it has a plus:
            // https://bugs.kde.org/show_bug.cgi?id=413920
            // QUrlQuery doesn't encode plus characters, bugzilla serverside however
            // needs it encoded which is a bit weird because it doesn't actually
            // require full-form encoding either (i.e. space becomes plus and
            // plus becomes encoded).
            //
            // This further broke because we force-recoded the query items but then that caused over-decoding (%FF)
            // because QUrlQuery internally stores the DecodeReserved variant and we blindly FullDecode leading to the
            // verbatim percent value getting decoded. At the same time we can't DecodeReserved because that would
            // still decode verbatim reserved sequences in the input password (e.g. the password containing %3C aka <).
            // https://bugs.kde.org/show_bug.cgi?id=435442
            if (httpBlob.startsWith("GET /hi?informal=yes%2Bcertainly&password=%253C___m%26T9zSZ%3E0%2Cq%25FFDN")) {
                QFile file(QFINDTESTDATA("data/hi.http"));
                file.open(QFile::ReadOnly | QFile::Text);
                socket->write(file.readAll());
                socket->waitForBytesWritten();
                socket->disconnect();
                socket->close();
                return;
            }
            qDebug() << httpBlob;
            Q_ASSERT_X(false, "server", "Unexpected request");
        });

        QUrl root("http://localhost");
        root.setPort(t.serverPort());
        HTTPConnection c(root);
        Query query;
        query.addQueryItem("informal", "yes+certainly");
        query.addQueryItem("password", "%3C___m&T9zSZ>0,q%FFDN");
        auto job = c.get("/hi", query);
        job->exec();
        QCOMPARE(job->data(), "Hello!\n");
    }

    void testGetJsonError()
    {
        qDebug() << Q_FUNC_INFO;
        // qhttpserver is still in qt-labs. as a simple solution do some dumb
        // http socketing.
        QTcpServer t;
        QCOMPARE(t.listen(QHostAddress::LocalHost, 0), true);
        connect(&t, &QTcpServer::newConnection, &t, [&t]() {
            QTcpSocket *socket = t.nextPendingConnection();
            socket->waitForReadyRead();
            QString httpBlob = socket->readAll();
            qDebug() << httpBlob;
            QFile file(QFINDTESTDATA("data/error.http"));
            file.open(QFile::ReadOnly | QFile::Text);
            socket->write(file.readAll());
            socket->waitForBytesWritten();
            socket->disconnect();
            socket->close();
            return;
        });

        QUrl root("http://localhost");
        root.setPort(t.serverPort());
        HTTPConnection c(root);
        auto job = c.get("/hi");
        job->exec();
        QVERIFY_EXCEPTION_THROWN(job->document(), Bugzilla::APIException);
    }

    void testPut()
    {
        qDebug() << Q_FUNC_INFO;
        // qhttpserver is still in qt-labs. as a simple solution do some dumb
        // http socketing.
        QThread thread;
        // On the heap lest it gets destroyed on stack unwind (which would be
        // in the wrong thread!) and may fail assertions inside Qt when built
        // in debug mode as destruction entails posting events, which is ENOGOOD
        // across threads.
        auto *server = new QTcpServer;
        server->moveToThread(&thread);

        QString readBlob; // lambda member essentially

        connect(server, &QTcpServer::newConnection, server, [server, &readBlob]() { // clazy:exclude=lambda-in-connect
            QCOMPARE(server->thread(), QThread::currentThread());
            QTcpSocket *socket = server->nextPendingConnection();
            connect(socket, &QTcpSocket::readyRead, socket, [&readBlob, socket] { // clazy:exclude=lambda-in-connect
                readBlob += socket->readAll();
                readBlob.replace("\r\n", "\n");
                auto parts = readBlob.split("\n");
                if (parts.contains("PUT /put HTTP/1.1") && parts.contains("Content-Length: 12") && parts.contains("hello there!")) {
                    QFile file(QFINDTESTDATA("data/put.http"));
                    file.open(QFile::ReadOnly | QFile::Text);
                    QByteArray ret = file.readAll();
                    ret.replace("\n", "\r\n");
                    qDebug() << ret;
                    socket->write(ret);
                    socket->waitForBytesWritten();
                    socket->disconnect();
                    socket->close();
                    qDebug() << "socket closed";
                }
            });
        });
        thread.start();

        QMutex portMutex;
        QWaitCondition portCondition;
        quint16 port;
        portMutex.lock();
        QTimer::singleShot(0, server, [server, &portMutex, &portCondition, &port]() {
            server->listen(QHostAddress::LocalHost, 0);
            QMutexLocker locker(&portMutex);
            port = server->serverPort();
            portCondition.wakeAll();
        });
        portCondition.wait(&portMutex);
        portMutex.unlock();

        QUrl root("http://localhost");
        root.setPort(server->serverPort());
        HTTPConnection c(root);
        APIJob *job = c.put("/put", "hello there!");
        KJob *kjob = job;
        QSignalSpy spy(job, &KJob::finished);
        kjob->start();
        // Because of how the request handling works the server may never return
        // anything, so wait for the reply, if it doesn't arrive something went
        // wrong with the server-side handling and the test cannot complete.
        QVERIFY(spy.wait());

        thread.quit();
        thread.wait();
        thread.terminate();

        QCOMPARE(job->error(), KJob::NoError);
        QCOMPARE(job->data(), "General Kenobi!\r\n");
    }
};

} // namespace Bugzilla

QTEST_MAIN(Bugzilla::ConnectionTest)

#include "connectiontest.moc"
