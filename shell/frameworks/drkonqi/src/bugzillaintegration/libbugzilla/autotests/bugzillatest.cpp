/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include "../bugzilla.h"

#include "jobdouble.h"

namespace Bugzilla
{
class ConnectionDouble : public Connection
{
public:
    using Connection::Connection;

    void setToken(const QString &) override
    {
        Q_UNREACHABLE();
    }

    [[nodiscard]] APIJob *get(const QString &path, const Query &query = Query()) const override
    {
        if (path == "/version") {
            return new JobDouble{QFINDTESTDATA("data/bugzilla.version.json")};
        } else if (path == "/login" && query.toString() == "login=auser&password=apass&restrict_login=true") {
            return new JobDouble{QFINDTESTDATA("data/bugzilla.login.json")};
        }
        Q_ASSERT_X(false, "get", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *post(const QString &path, const QByteArray &, const Query &query = Query()) const override
    {
        Q_ASSERT_X(false, "post", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *put(const QString &path, const QByteArray &, const Query &query = Query()) const override
    {
        Q_ASSERT_X(false, "put", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }
};

} // namespace Bugzilla

class BugzillaTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Bugzilla::setConnection(m_doubleConnection);
    }

    void testVersion()
    {
        KJob *job = Bugzilla::version();
        job->start();
        QCOMPARE(Bugzilla::version(job), "5.0.0");
    }

    void testLogin()
    {
        KJob *job = Bugzilla::login("auser", "apass");
        job->start();
        auto details = Bugzilla::login(job);
        QCOMPARE(details.id, 52960);
        QCOMPARE(details.token, "52960-aaaaaaaaaa");
    }

private:
    Bugzilla::ConnectionDouble *m_doubleConnection = new Bugzilla::ConnectionDouble;
};

QTEST_GUILESS_MAIN(BugzillaTest)

#include "bugzillatest.moc"
