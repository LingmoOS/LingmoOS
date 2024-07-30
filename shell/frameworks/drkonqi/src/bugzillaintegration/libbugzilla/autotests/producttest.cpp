/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDebug>
#include <QTest>

#include <clients/productclient.h>

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
        Q_UNUSED(path);
        Q_UNUSED(query);
        if (path == "/product/dragonplayer") {
            return new JobDouble{QFINDTESTDATA("data/product.dragonplayer.json")};
        }
        Q_ASSERT_X(false, "get", qUtf8Printable(QStringLiteral("unmapped: %1; %2").arg(path, query.toString())));
        return nullptr;
    }

    [[nodiscard]] APIJob *post(const QString &path, const QByteArray &, const Query &query = Query()) const override
    {
        qDebug() << path << query.toString();
        Q_UNREACHABLE();
        return nullptr;
    }

    [[nodiscard]] APIJob *put(const QString &, const QByteArray &, const Query & = Query()) const override
    {
        Q_UNREACHABLE();
    }
};

class ProductTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Bugzilla::setConnection(m_doubleConnection);
    }

    void testProduct()
    {
        KJob *job = Bugzilla::ProductClient().get("dragonplayer");
        Q_ASSERT(job);
        job->start();
        Product::Ptr product = Bugzilla::ProductClient().get(job);

        QCOMPARE(product->isActive(), true);
        QCOMPARE(product->componentNames(), QStringList({"general"}));
        QCOMPARE(product->allVersions(),
                 QStringList({"2.0", "2.0-beta1", "2.0-git", "2.0.x", "17.04", "17.08", "17.12", "18.04", "18.08", "18.12", "SVN", "unspecified"}));
        QCOMPARE(product->inactiveVersions(), QStringList({"2.0", "2.0-beta1", "2.0-git", "2.0.x", "SVN"}));

        QCOMPARE(product->versions().size(), 12);
        { // inactive version
            auto version = product->versions()[0];
            QCOMPARE(version->id(), 4408);
            QCOMPARE(version->name(), "2.0");
            QCOMPARE(version->isActive(), false);
        }
        { // active version
            auto version = product->versions()[11];
            QCOMPARE(version->id(), 3239);
            QCOMPARE(version->name(), "unspecified");
            QCOMPARE(version->isActive(), true);
        }

        QCOMPARE(product->components().size(), 1);
        auto component = product->components()[0];
        QCOMPARE(component->id(), 1200);
        QCOMPARE(component->name(), "general");
    }

private:
    Bugzilla::ConnectionDouble *m_doubleConnection = new Bugzilla::ConnectionDouble;
};

} // namespace Bugzilla

QTEST_MAIN(Bugzilla::ProductTest)

#include "producttest.moc"
