/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QQmlContext>

#include "ItemBuilder.h"

class TestIncubationController : public QObject, public QQmlIncubationController
{
public:
    TestIncubationController()
    {
        startTimer(16);
    }

protected:
    void timerEvent(QTimerEvent *) override
    {
        incubateFor(5);
    }
};

class ItemBuilderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        m_engine = new QQmlEngine{this};
        m_component = new QQmlComponent{m_engine, this};
        m_component->setData("import QtQuick 2.15; Item { }", QUrl());
        m_item = new QQuickItem{};
    }

    void testBasic()
    {
        ItemBuilder builder;
        builder.setComponent(m_component);
        builder.setCount(10);
        builder.setIncubationMode(QQmlIncubator::IncubationMode::Synchronous);

        QSignalSpy beginCreateSpy{&builder, &ItemBuilder::beginCreate};
        QSignalSpy endCreateSpy{&builder, &ItemBuilder::endCreate};
        QSignalSpy finishedSpy{&builder, &ItemBuilder::finished};

        builder.build(m_item);

        QCOMPARE(beginCreateSpy.size(), 10);
        QCOMPARE(endCreateSpy.size(), 10);
        QCOMPARE(finishedSpy.size(), 1);

        auto items = builder.items();
        QCOMPARE(items.size(), 10);
        for (auto item : items) {
            QVERIFY(item);
            QCOMPARE(item->parentItem(), m_item);
        }


        beginCreateSpy.clear();
        endCreateSpy.clear();
        finishedSpy.clear();

        builder.setCount(20);
        QCOMPARE(builder.items().size(), 0);
        QVERIFY(!builder.isFinished());

        builder.build(m_item);

        QVERIFY(builder.isFinished());

        items = builder.items();
        QCOMPARE(items.size(), 20);
        QCOMPARE(finishedSpy.size(), 1);
        QCOMPARE(beginCreateSpy.size(), 20);
        QCOMPARE(endCreateSpy.size(), 20);

        for (auto item : items) {
            QVERIFY(item);
            QCOMPARE(item->parentItem(), m_item);
        }
    }

    void testInitialProperties()
    {
        ItemBuilder builder;
        builder.setComponent(m_component);
        builder.setCount(10);
        builder.setIncubationMode(QQmlIncubator::IncubationMode::Synchronous);

        builder.setInitialProperties({
            { QStringLiteral("width"), 100 },
            { QStringLiteral("height"), 100 }
        });

        builder.build(m_item);

        QVERIFY(builder.isFinished());

        const auto items = builder.items();
        QCOMPARE(items.size(), 10);
        for (auto item : items) {
            QVERIFY(item);
            QCOMPARE(item->parentItem(), m_item);
            QCOMPARE(item->width(), 100);
            QCOMPARE(item->height(), 100);
        }
    }

    void testAsync()
    {
        m_engine->setIncubationController(new TestIncubationController);

        ItemBuilder builder;
        builder.setComponent(m_component);
        builder.setCount(10);
        builder.setIncubationMode(QQmlIncubator::IncubationMode::Asynchronous);

        builder.setInitialProperties({
            { QStringLiteral("width"), 100 },
            { QStringLiteral("height"), 100 }
        });

        builder.build(m_item);

        QTRY_VERIFY_WITH_TIMEOUT(builder.isFinished(), 500);

        const auto items = builder.items();
        QCOMPARE(items.size(), 10);
        for (auto item : items) {
            QVERIFY(item);
            QCOMPARE(item->parentItem(), m_item);
            QCOMPARE(item->width(), 100);
            QCOMPARE(item->height(), 100);
        }

        m_engine->setIncubationController(nullptr);
    }

    void cleanupTestCase()
    {
        delete m_item;
        delete m_component;
        delete m_engine;
    }

private:
    QQmlEngine *m_engine = nullptr;
    QQmlComponent *m_component = nullptr;
    QQuickItem *m_item = nullptr;
};

QTEST_GUILESS_MAIN(ItemBuilderTest)

#include "ItemBuilderTest.moc"
