/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "servercontroller.h"

#include <rest/restapi.h>
#include <rest/restclient.h>

#include <core/product.h>
#include <core/schemaentryelement.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QNetworkReply>
#include <QObject>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KUserFeedback::Console;

class ProductApiTest : public QObject
{
    Q_OBJECT
private:
    ServerController m_server;

    ServerInfo testServer() const
    {
        ServerInfo s;
        s.setUrl(m_server.url());
        return s;
    }

    bool waitForFinished(QNetworkReply *reply)
    {
        Q_ASSERT(reply);
        QSignalSpy spy(reply, &QNetworkReply::finished);
        Q_ASSERT(spy.isValid());
        return spy.wait();
    }

    Product findProduct(const QVector<Product> &products, const QString &name)
    {
        const auto it = std::find_if(products.cbegin(), products.cend(), [name](const Product &p) {
            return p.name() == name;
        });
        if (it != products.cend())
            return *it;
        return {};
    }

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        QVERIFY(m_server.start());
    }

    void testInvalidList()
    {
        RESTClient client;
        auto serverInfo = testServer();
        serverInfo.setPassword(QStringLiteral("wrong password"));
        client.setServerInfo(serverInfo);
        client.setConnected(true);
        QVERIFY(client.isConnected());

        auto reply = RESTApi::listProducts(&client);
        QVERIFY(waitForFinished(reply));
        QEXPECT_FAIL("", "local PHP tests don't authenticate", Continue);
        QVERIFY(reply->error() != QNetworkReply::NoError);
    }

    void testProductCRUD()
    {
        RESTClient client;
        client.setServerInfo(testServer());
        client.setConnected(true);
        QVERIFY(client.isConnected());
        Product newProduct;
        newProduct.setName(QStringLiteral("org.kde.UserFeedback.UnitTestProduct"));

        // clean up from previous failed runs, if needed
        auto reply = RESTApi::deleteProduct(&client, newProduct);
        waitForFinished(reply);

        // list existing products
        reply = RESTApi::listProducts(&client);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QVERIFY(reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith(QLatin1String("text/json")));
        const auto products = Product::fromJson(reply->readAll());
        QVERIFY(!findProduct(products, QLatin1String("org.kde.UserFeedback.UnitTestProduct")).isValid());

        // add new product
        SchemaEntry entry;
        entry.setName(QStringLiteral("entry1"));
        SchemaEntryElement elem1;
        elem1.setName(QStringLiteral("elem11"));
        entry.setElements({elem1});
        newProduct.setSchema({entry});
        QVERIFY(newProduct.isValid());
        reply = RESTApi::createProduct(&client, newProduct);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // verify the new product is there
        reply = RESTApi::listProducts(&client);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        auto newProducts = Product::fromJson(reply->readAll());
        QCOMPARE(newProducts.size(), products.size() + 1);
        const auto createdProduct = findProduct(newProducts, QLatin1String("org.kde.UserFeedback.UnitTestProduct"));
        QVERIFY(createdProduct.isValid());
        QCOMPARE(createdProduct.schema(), newProduct.schema());

        // update a product
        reply = RESTApi::updateProduct(&client, newProduct);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // try to add an already existing product
        reply = RESTApi::createProduct(&client, newProduct);
        QVERIFY(waitForFinished(reply));
        QVERIFY(reply->error() != QNetworkReply::NoError);

        // verify it really wasn't added
        reply = RESTApi::listProducts(&client);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        newProducts = Product::fromJson(reply->readAll());
        QCOMPARE(newProducts.size(), products.size() + 1);

        // delete a product
        reply = RESTApi::deleteProduct(&client, newProduct);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);

        // verify it's gone, and nothing else was harmed
        reply = RESTApi::listProducts(&client);
        QVERIFY(waitForFinished(reply));
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        newProducts = Product::fromJson(reply->readAll());
        QCOMPARE(newProducts.size(), products.size());
        QVERIFY(!findProduct(newProducts, QLatin1String("org.kde.UserFeedback.UnitTestProduct")).isValid());
    }

    void testInvalidProductOperations()
    {
        RESTClient client;
        client.setServerInfo(testServer());
        client.setConnected(true);
        QVERIFY(client.isConnected());

        Product invalidProduct;
        invalidProduct.setName(QStringLiteral("org.kde.UserFeedback.Invalid"));
        QVERIFY(invalidProduct.isValid());

        // update a non-existing product
        auto reply = RESTApi::updateProduct(&client, invalidProduct);
        QVERIFY(waitForFinished(reply));
        qDebug() << reply->readAll();
        QVERIFY(reply->error() != QNetworkReply::NoError);

        // delete a non-existing product
        reply = RESTApi::deleteProduct(&client, invalidProduct);
        QVERIFY(waitForFinished(reply));
        QVERIFY(reply->error() != QNetworkReply::NoError);
    }
};

QTEST_MAIN(ProductApiTest)

#include "productapitest.moc"
