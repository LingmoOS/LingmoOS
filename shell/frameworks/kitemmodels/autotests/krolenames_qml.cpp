/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QAbstractItemModel>
#include <QJsonObject>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>

#include <memory>

#ifdef IMPORT_ITEMMODELSPLUGIN
#include <QPluginLoader>
Q_IMPORT_PLUGIN(org_kde_kitemmodelsPlugin)
#endif

class tst_KRoleNamesQml : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAttachedNonModel();
    void testAttachedInvalidRole();
    void testAttachedInvalidRoleName();
    void testAttachedRoleToRoleName();
    void testAttachedRoleNameToRole();

private:
    QAbstractItemModel *createMonthTestModel(QObject *parent = nullptr);
};

QAbstractItemModel *tst_KRoleNamesQml::createMonthTestModel(QObject *parent)
{
    auto testModel = new QStandardItemModel(parent);
    for (int i = 1; i <= 12; i++) {
        auto entry = new QStandardItem();
        entry->setData(QLocale::c().monthName(i), Qt::DisplayRole);
        entry->setData(i, Qt::UserRole);
        testModel->appendRow(entry);
    }
    testModel->setItemRoleNames({{Qt::UserRole, "user"}, {Qt::DisplayRole, "display"}});
    return testModel;
}

void tst_KRoleNamesQml::testAttachedNonModel()
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".* KRoleNames must be attached to a QAbstractItemModel"));
    QQmlApplicationEngine app;
    app.loadData(R"(
        import QtQml
        import org.kde.kitemmodels as KItemModels

        QtObject {
            property int role

            Component.onCompleted: {
                role = KItemModels.KRoleNames.role("display");
            }
        }
    )");
    QCOMPARE(app.rootObjects().count(), 1);
}

void tst_KRoleNamesQml::testAttachedInvalidRole()
{
    QQmlApplicationEngine app;
    std::unique_ptr<QStandardItemModel> model = std::make_unique<QStandardItemModel>();
    model->setItemRoleNames({});
    app.setInitialProperties({{"model", QVariant::fromValue(&*model)}});
    app.loadData(R"(
        import QtQml
        import org.kde.kitemmodels as KItemModels

        QtObject {
            required property var model
            property int role

            Component.onCompleted: {
                role = model.KItemModels.KRoleNames.role(Qt.UserRole);
            }
        }
    )");
    QCOMPARE(app.rootObjects().count(), 1);
    const auto object = app.rootObjects().first();
    const auto role = object->property("role").toInt();
    QCOMPARE(role, -1);
}

void tst_KRoleNamesQml::testAttachedInvalidRoleName()
{
    QQmlApplicationEngine app;
    std::unique_ptr<QStandardItemModel> model = std::make_unique<QStandardItemModel>();
    model->setItemRoleNames({});
    app.setInitialProperties({{"model", QVariant::fromValue(&*model)}});
    app.loadData(R"(
        import QtQml
        import org.kde.kitemmodels as KItemModels

        QtObject {
            required property var model
            property string roleName

            Component.onCompleted: {
                roleName = model.KItemModels.KRoleNames.roleName(Qt.UserRole);
            }
        }
    )");
    QCOMPARE(app.rootObjects().count(), 1);
    const auto object = app.rootObjects().first();
    const auto roleName = object->property("roleName").toString();
    QCOMPARE(roleName, "");
}

void tst_KRoleNamesQml::testAttachedRoleNameToRole()
{
    QQmlApplicationEngine app;
    std::unique_ptr<QAbstractItemModel> model{createMonthTestModel(nullptr)};
    app.setInitialProperties({{"model", QVariant::fromValue(&*model)}});
    app.loadData(R"(
        import QtQml
        import org.kde.kitemmodels as KItemModels

        QtObject {
            required property var model
            property int role

            Component.onCompleted: {
                role = model.KItemModels.KRoleNames.role("user");
            }
        }
    )");
    QCOMPARE(app.rootObjects().count(), 1);
    const auto object = app.rootObjects().first();
    const auto role = object->property("role").toInt();
    QCOMPARE(role, Qt::UserRole);
}

void tst_KRoleNamesQml::testAttachedRoleToRoleName()
{
    QQmlApplicationEngine app;
    std::unique_ptr<QAbstractItemModel> model{createMonthTestModel(nullptr)};
    app.setInitialProperties({{"model", QVariant::fromValue(&*model)}});
    app.loadData(R"(
        import QtQml
        import org.kde.kitemmodels as KItemModels

        QtObject {
            required property var model
            property string roleName

            Component.onCompleted: {
                roleName = model.KItemModels.KRoleNames.roleName(Qt.UserRole);
            }
        }
    )");
    QCOMPARE(app.rootObjects().count(), 1);
    const auto object = app.rootObjects().first();
    const auto roleName = object->property("roleName").toString();
    QCOMPARE(roleName, "user");
}

QTEST_GUILESS_MAIN(tst_KRoleNamesQml)

#include "krolenames_qml.moc"
