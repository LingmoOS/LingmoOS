/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include "../clients/attachmentclient.h"

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
        Q_UNUSED(path)
        Q_UNUSED(query)
        Q_ASSERT_X(false, "get", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *post(const QString &path, const QByteArray &data, const Query &query = Query()) const override
    {
        Q_UNUSED(query);
        if (path == "/bug/1/attachment") {
            QJsonParseError error;
            QJsonDocument::fromJson(data, &error);
            Q_ASSERT(error.error == QJsonParseError::NoError);
            return new JobDouble{QFINDTESTDATA("data/attachment.new.json")};
        }
        Q_ASSERT_X(false, "post", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *put(const QString &path, const QByteArray &, const Query &query = Query()) const override
    {
        Q_UNUSED(path)
        Q_UNUSED(query)
        Q_ASSERT_X(false, "put", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }
};

class AttachmentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase()
    {
        Bugzilla::setConnection(m_doubleConnection);
    }

    void testCreate()
    {
        Bugzilla::AttachmentClient c;
        NewAttachment attachment;
        attachment.ids = {1};
        attachment.data = "123";
        attachment.file_name = "filename123";
        attachment.summary = "summary123";
        //        attachment.content_type = "123";
        attachment.comment = "comment123";
        attachment.is_patch = true;
        attachment.is_private = false;
        auto job = c.createAttachment(1, attachment);
        job->start();
        QList<int> ids = c.createAttachment(job);
        QCOMPARE(ids, QList<int>({1234}));
    }

private:
    Bugzilla::ConnectionDouble *m_doubleConnection = new Bugzilla::ConnectionDouble;
};

} // namespace Bugzilla

QTEST_MAIN(Bugzilla::AttachmentTest)

#include "attachmenttest.moc"
