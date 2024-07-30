/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <selectionratiosource.h>

#include <QDebug>
#include <QItemSelectionModel>
#include <QtTest/qtest.h>
#include <QObject>
#include <QSettings>
#include <QStandardItemModel>
#include <QStandardPaths>

using namespace KUserFeedback;

class SelectionRatioSourceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
        QStandardPaths::setTestModeEnabled(true);
    }

    void testPropertyRatioSource()
    {
        QStandardItemModel model;
        model.appendRow(new QStandardItem(QStringLiteral("row1")));
        model.appendRow(new QStandardItem(QStringLiteral("row2")));
        model.appendRow(new QStandardItem(QStringLiteral("row3")));

        QItemSelectionModel sel(&model);
        sel.select(model.index(1, 0), QItemSelectionModel::ClearAndSelect);

        SelectionRatioSource src(&sel, QStringLiteral("selection"));
        QTest::qWait(1);

        auto v = src.data();
        QVERIFY(v.canConvert<QVariantMap>());
        auto o = v.toMap();
        QCOMPARE(o.size(), 0); // nothing recorded

        QTest::qWait(1200);
        v = src.data();
        o = v.toMap();
        QCOMPARE(o.size(), 1);
        QVERIFY(o.contains(QLatin1String("row2")));
        v = o.value(QLatin1String("row2")).toMap().value(QLatin1String("property"));
        QCOMPARE(v.type(), QVariant::Double);

        sel.select(model.index(2, 0), QItemSelectionModel::ClearAndSelect);
        QTest::qWait(1200);
        v = src.data();
        o = v.toMap();
        QCOMPARE(o.size(), 2);
        QVERIFY(o.contains(QLatin1String("row2")));
        v = o.value(QLatin1String("row2")).toMap().value(QLatin1String("property"));
        QCOMPARE(v.type(), QVariant::Double);
    }
};

QTEST_MAIN(SelectionRatioSourceTest)

#include "selectionratiosourcetest.moc"
