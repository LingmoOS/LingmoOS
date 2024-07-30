/*
    Copyright (C) 2019 David Edmundson <davidedmundson@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <QObject>

#include <QSignalSpy>
#include <QTest>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#ifdef IMPORT_ITEMMODELSPLUGIN
#include <QPluginLoader>
Q_IMPORT_PLUGIN(org_kde_kitemmodelsPlugin)
#endif

class tst_KSortFilterProxyModelQml : public QObject
{
    Q_OBJECT

public:
    enum Roles {
        // Some non-default role ID with a QString data
        MonthNameRole = Qt::UserRole,
        IntUserRole,
    };
    Q_ENUM(Roles)

private Q_SLOTS:
    void init();
    void testFilterCallback();
    void testSortRole_data();
    void testSortRole();
    void testFilterRegExp();
    void testFilterRegExpRole();
    void testRoleVsModelRace();

private:
    QAbstractItemModel *createMonthTestModel(QObject *parent);
};

QAbstractItemModel *tst_KSortFilterProxyModelQml::createMonthTestModel(QObject *parent)
{
    auto testModel = new QStandardItemModel(parent);
    for (int i = 1; i <= 12; i++) {
        auto entry = new QStandardItem();
        const auto month = QLocale::c().monthName(i);
        entry->setData(month, Qt::DisplayRole);
        entry->setData(month, MonthNameRole);
        entry->setData(i, IntUserRole);
        testModel->appendRow(entry);
    }
    testModel->setItemRoleNames({
        {Qt::DisplayRole, "display"},
        {MonthNameRole, "month"},
        {IntUserRole, "user"},
    });
    return testModel;
}

void tst_KSortFilterProxyModelQml::init()
{
    qmlRegisterType<tst_KSortFilterProxyModelQml>("org.kde.kitemmodels.test", 1, 0, "Self");
}

void tst_KSortFilterProxyModelQml::testFilterCallback()
{
    QQmlApplicationEngine app;
    app.loadData(
        "import QtQml 2.0\n"
        "import org.kde.kitemmodels 1.0\n"
        "KSortFilterProxyModel\n"
        "{\n"
        "    property int modulo: 2\n"
        "    sourceModel: KNumberModel {\n"
        "        minimumValue: 1\n"
        "        maximumValue: 10\n"
        "    }\n"
        "    filterRowCallback: function(source_row, source_parent) {\n"
        "        return sourceModel.data(sourceModel.index(source_row, 0, source_parent), Qt.DisplayRole) % modulo == 1;\n"
        "    };\n"
        "}\n");
    QCOMPARE(app.rootObjects().count(), 1);

    auto filterModel = qobject_cast<QAbstractItemModel *>(app.rootObjects().first());
    QVERIFY(filterModel);

    QCOMPARE(filterModel->rowCount(), 5);
    QCOMPARE(filterModel->data(filterModel->index(0, 0)).toString(), "1");
    QCOMPARE(filterModel->data(filterModel->index(1, 0)).toString(), "3");
    QCOMPARE(filterModel->data(filterModel->index(2, 0)).toString(), "5");
    QCOMPARE(filterModel->data(filterModel->index(3, 0)).toString(), "7");
    QCOMPARE(filterModel->data(filterModel->index(4, 0)).toString(), "9");

    filterModel->setProperty("modulo", 3);

    // Nothing should change until we call invalidateFilter
    QCOMPARE(filterModel->rowCount(), 5);
    QCOMPARE(filterModel->data(filterModel->index(0, 0)).toString(), "1");
    QCOMPARE(filterModel->data(filterModel->index(1, 0)).toString(), "3");
    QCOMPARE(filterModel->data(filterModel->index(2, 0)).toString(), "5");
    QCOMPARE(filterModel->data(filterModel->index(3, 0)).toString(), "7");
    QCOMPARE(filterModel->data(filterModel->index(4, 0)).toString(), "9");

    // Simulate call from QML by going through metaobject rather than calling it directly
    const bool invalidateFilterCallOk = QMetaObject::invokeMethod(filterModel, "invalidateFilter");
    QVERIFY(invalidateFilterCallOk);

    QCOMPARE(filterModel->rowCount(), 4);
    QCOMPARE(filterModel->data(filterModel->index(0, 0)).toString(), "1");
    QCOMPARE(filterModel->data(filterModel->index(1, 0)).toString(), "4");
    QCOMPARE(filterModel->data(filterModel->index(2, 0)).toString(), "7");
    QCOMPARE(filterModel->data(filterModel->index(3, 0)).toString(), "10");
}

void tst_KSortFilterProxyModelQml::testSortRole_data()
{
    // test model consists of all month names + month number as Display and UserRoler respectively

    QTest::addColumn<QString>("qmlContents");
    QTest::addColumn<QString>("result");

    QTest::newRow("sort by role name - display") << R"(
        KSortFilterProxyModel {
            sourceModel: testModel
            sortRoleName: "display"
        }
    )"
                                                 << "April";

    QTest::newRow("sort by role name - value") << R"(
        KSortFilterProxyModel {
            sourceModel: testModel
            sortRoleName: "user"
        }
    )"
                                               << "January";

    QTest::newRow("sort by role name - reset") << R"(
        KSortFilterProxyModel {
            sourceModel: testModel
            sortRoleName: ""
            Component.onCompleted: sortRoleName = ""
        }
    )"
                                               << "January";
}

void tst_KSortFilterProxyModelQml::testSortRole()
{
    QQmlApplicationEngine app;
    QFETCH(QString, qmlContents);
    QFETCH(QString, result);

    qmlContents = R"(
        import org.kde.kitemmodels 1.0
        import QtQuick 2.0

    )" + qmlContents;

    app.rootContext()->setContextProperty("testModel", createMonthTestModel(&app));

    app.loadData(qmlContents.toLatin1());

    QCOMPARE(app.rootObjects().count(), 1);
    auto filterModel = qobject_cast<QAbstractItemModel *>(app.rootObjects().first());
    QVERIFY(filterModel);
    QCOMPARE(filterModel->rowCount(), 12);
    QCOMPARE(filterModel->data(filterModel->index(0, 0), Qt::DisplayRole).toString(), result);
}

void tst_KSortFilterProxyModelQml::testFilterRegExp()
{
    // filterRegExp comes from the QSortFilterProxyModel directly, confirm it still works
    QQmlApplicationEngine app;

    app.rootContext()->setContextProperty("testModel", createMonthTestModel(&app));

    auto qmlSrc = QByteArray(
        "import QtQml 2.0\n"
        "import org.kde.kitemmodels 1.0\n"
        "KSortFilterProxyModel {\n"
        " sourceModel: testModel\n"
        " filterRegularExpression: /Ma.*/\n"
        "}\n");
    app.loadData(qmlSrc);

    QCOMPARE(app.rootObjects().count(), 1);
    auto filterModel = qobject_cast<QAbstractItemModel *>(app.rootObjects().first());
    QVERIFY(filterModel);
    QCOMPARE(filterModel->rowCount(), 2);
    QCOMPARE(filterModel->data(filterModel->index(0, 0), Qt::DisplayRole).toString(), "March");
    QCOMPARE(filterModel->data(filterModel->index(1, 0), Qt::DisplayRole).toString(), "May");
}

void tst_KSortFilterProxyModelQml::testFilterRegExpRole()
{
    // filterRegExp comes from the QSortFilterProxyModel directly, confirm it still works
    QQmlApplicationEngine app;

    app.rootContext()->setContextProperty("testModel", createMonthTestModel(&app));

    auto qmlSrc = QByteArray(
        "import QtQml 2.0\n"
        "import org.kde.kitemmodels 1.0\n"
        "KSortFilterProxyModel {\n"
        " sourceModel: testModel\n"
        " filterRoleName: \"user\"\n"
        " filterRegularExpression: /1[0-9]/\n" // month value is 10 or more
        "}\n");
    app.loadData(qmlSrc);

    QCOMPARE(app.rootObjects().count(), 1);
    auto filterModel = qobject_cast<QAbstractItemModel *>(app.rootObjects().first());
    QVERIFY(filterModel);
    QCOMPARE(filterModel->rowCount(), 3);
    QCOMPARE(filterModel->data(filterModel->index(0, 0), Qt::DisplayRole).toString(), "October");
    QCOMPARE(filterModel->data(filterModel->index(1, 0), Qt::DisplayRole).toString(), "November");
    QCOMPARE(filterModel->data(filterModel->index(2, 0), Qt::DisplayRole).toString(), "December");
}

void tst_KSortFilterProxyModelQml::testRoleVsModelRace()
{
    // Our QML type extends base type's role property with a roleName
    // property, thus we have kinda two sources of truth, and they both reset
    // if a model is assigned after component's completion.
    // See BUG 476950
    QQmlApplicationEngine app;

    auto qmlSrcModel = QByteArray(R"(
        import QtQml
        import org.kde.kitemmodels as KItemModels
        import org.kde.kitemmodels.test as KItemModelsTest

        KItemModels.KSortFilterProxyModel {
            id: root

            sortCaseSensitivity: Qt.CaseInsensitive
            sortRole: KItemModelsTest.Self.MonthNameRole

            filterCaseSensitivity: Qt.CaseInsensitive
            filterRole: KItemModelsTest.Self.MonthNameRole
        }
    )");
    app.loadData(qmlSrcModel);
    auto filterModel = qobject_cast<QSortFilterProxyModel *>(app.rootObjects().first());
    QVERIFY(filterModel);

    // There is no model yet, so the role names are default so far
    QCOMPARE(filterModel->property("sortRole").toInt(), MonthNameRole);
    QCOMPARE(filterModel->property("sortRoleName").toString(), QStringLiteral("display"));
    QCOMPARE(filterModel->property("filterRole").toInt(), MonthNameRole);
    QCOMPARE(filterModel->property("filterRoleName").toString(), QStringLiteral("display"));

    const auto model = createMonthTestModel(&app);

    QVERIFY(filterModel->setProperty("filterString", QStringLiteral("a")));
    QCOMPARE(filterModel->rowCount(), 0);

    filterModel->setSourceModel(model);
    QCOMPARE(filterModel->rowCount(), 6);

    QCOMPARE(filterModel->property("sortRole").toInt(), MonthNameRole);
    QCOMPARE(filterModel->property("sortRoleName").toString(), QStringLiteral("month"));
    QCOMPARE(filterModel->property("filterRole").toInt(), MonthNameRole);
    QCOMPARE(filterModel->property("filterRoleName").toString(), QStringLiteral("month"));

    // and now swap the source of truth to roleName
    QVERIFY(filterModel->setProperty("sortRoleName", QStringLiteral("user")));
    QCOMPARE(filterModel->property("sortRole").toInt(), IntUserRole);
    QVERIFY(filterModel->setProperty("filterRoleName", QStringLiteral("user")));
    QCOMPARE(filterModel->property("filterRole").toInt(), IntUserRole);

    QVERIFY(filterModel->setProperty("filterString", QStringLiteral("9")));

    // Reset the model. Roles should persist.
    QCOMPARE(filterModel->rowCount(), 1);
    filterModel->setSourceModel(nullptr);
    filterModel->setSourceModel(model);
    QCOMPARE(filterModel->rowCount(), 1);

    QCOMPARE(filterModel->property("sortRole").toInt(), IntUserRole);
    QCOMPARE(filterModel->property("sortRoleName").toString(), QStringLiteral("user"));
    QCOMPARE(filterModel->property("filterRole").toInt(), IntUserRole);
    QCOMPARE(filterModel->property("filterRoleName").toString(), QStringLiteral("user"));
}

QTEST_GUILESS_MAIN(tst_KSortFilterProxyModelQml)

#include "ksortfilterproxymodel_qml.moc"
