/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include "../clients/commentclient.h"

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
        if (path == "/bug/407363/comment" && query.toString().isEmpty()) {
            return new JobDouble{QFINDTESTDATA("data/comments.json")};
        }
        if (path == "/bug/1/comment" && query.toString().isEmpty()) {
            return new JobDouble{QFINDTESTDATA("data/error.nobug.invalid.json")};
        }
        Q_ASSERT_X(false, "get", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *post(const QString &path, const QByteArray &, const Query &query = Query()) const override
    {
        Q_UNUSED(path);
        Q_UNUSED(query);
        Q_ASSERT_X(false, "post", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *put(const QString &path, const QByteArray &, const Query &query = Query()) const override
    {
        Q_UNUSED(path);
        Q_UNUSED(query);
        Q_ASSERT_X(false, "put", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }
};

class CommentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase()
    {
        Bugzilla::setConnection(m_doubleConnection);
    }

    void testSearch()
    {
        Bugzilla::CommentClient c;
        auto job = c.getFromBug(407363);
        job->start();
        auto comments = c.getFromBug(job);
        QCOMPARE(comments.size(), 3);

        Comment::Ptr uno = comments[0];
        QCOMPARE(uno->bug_id(), 407363);
        QCOMPARE(uno->text(), "uno");

        Comment::Ptr tre = comments[2];
        QCOMPARE(tre->bug_id(), 407363);
        QCOMPARE(tre->text(), "tre");
    }

    void testSearchNoBugInvalid()
    {
        // Our bugzilla has a bug where errors do not have error:true!
        // Make sure we correctly handle objects that are errors but do not
        // necessarily have error:true.
        // This is particularly relevant for comments because we make
        // expectations about bugs being valid/invalid/throwing.
        Bugzilla::CommentClient c;
        auto job = c.getFromBug(1);
        job->start();
        QVERIFY_EXCEPTION_THROWN(c.getFromBug(job), Bugzilla::APIException);
    }

private:
    Bugzilla::ConnectionDouble *m_doubleConnection = new Bugzilla::ConnectionDouble;
};

} // namespace Bugzilla

QTEST_MAIN(Bugzilla::CommentTest)

#include "commenttest.moc"
