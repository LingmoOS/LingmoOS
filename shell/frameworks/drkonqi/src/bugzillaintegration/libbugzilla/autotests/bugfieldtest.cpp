/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <clients/bugfieldclient.h>

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
        if (path == "/field/bug/rep_platform" && query.toString() == "") {
            return new JobDouble{QFINDTESTDATA("data/field.rep_platform.json")};
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

class BugFieldTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Bugzilla::setConnection(m_doubleConnection);
    }

    void testGet()
    {
        Bugzilla::BugFieldClient client;
        KJob *job = client.getField("rep_platform");
        QVERIFY(job);
        job->start();
        auto field = client.getField(job);
        auto values = field->values();
        QCOMPARE(5, values.size());
        bool containsWindows = false;
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            if ((*it)->name() == QLatin1String("MS Windows")) {
                containsWindows = true;
                break;
            }
        }
        QVERIFY(containsWindows);
        // Run again to drive up coverage for converter registration
        job = client.getField("rep_platform");
        QVERIFY(job);
        job->start();
        client.getField(job);
    }

private:
    Bugzilla::ConnectionDouble *m_doubleConnection = new Bugzilla::ConnectionDouble;
};

} // namespace Bugzilla

QTEST_MAIN(Bugzilla::BugFieldTest)

#include "bugfieldtest.moc"
