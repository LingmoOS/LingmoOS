/*
    SPDX-FileCopyrightText: 2023 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QObject>
#include <QStandardPaths>
#include <QTest>

#include "kcommandbar.h"

static void setEnvironment()
{
    QStandardPaths::setTestModeEnabled(true);
}

class KCommandBarTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNoCentralWidget()
    {
        QMainWindow w;
        w.setCentralWidget(nullptr);
        w.show();

        KCommandBar b(&w);
        b.show();
    }

    void testNoMainWindowParent()
    {
        QMainWindow w;
        auto central = new QWidget(&w);
        w.setCentralWidget(central);
        auto l = new QHBoxLayout(central);
        auto lv = new QListView();
        l->addWidget(lv);
        l->addWidget(new QWidget(), 2);
        w.showMaximized();
        // QTest::qWaitForWindowExposed(&w);

        KCommandBar b(lv);
        b.show();
        // QTest::qWait(100000);
    }

    void testLastUsedActionRestored()
    {
        QMainWindow w;
        auto central = new QWidget(&w);
        w.setCentralWidget(central);

        KCommandBar::ActionGroup ag;
        ag.name = QStringLiteral("Group1");
        QAction *a = new QAction(QStringLiteral("Act1"), &w);
        a->setObjectName("act1");
        ag.actions << a;
        a = new QAction(QStringLiteral("Act2"), &w);
        a->setObjectName("act2");
        ag.actions << a;
        {
            KCommandBar b(&w);
            b.setActions({ag});

            auto treeView = b.findChild<QAbstractItemView *>();
            auto lineEdit = b.findChild<QLineEdit *>();
            QVERIFY(treeView);
            QVERIFY(lineEdit);
            QVERIFY(treeView->model());
            QCOMPARE(treeView->model()->rowCount(), 3);
            QCOMPARE(treeView->model()->index(1, 0).data().toString(), QStringLiteral("Group1: %1").arg(ag.actions[0]->text()));
            QCOMPARE(treeView->model()->index(2, 0).data().toString(), QStringLiteral("Group1: %1").arg(ag.actions[1]->text()));

            QTest::sendKeyEvent(QTest::KeyAction::Press, treeView, Qt::Key_Down, {}, Qt::NoModifier);
            QTest::sendKeyEvent(QTest::KeyAction::Press, treeView, Qt::Key_Down, {}, Qt::NoModifier);
            QCOMPARE(treeView->currentIndex().data().toString(), QStringLiteral("Group1: Act2"));

            lineEdit->returnPressed();
        }

        {
            KCommandBar b(&w);
            b.setActions({ag});

            auto treeView = b.findChild<QAbstractItemView *>();
            QCOMPARE(treeView->model()->rowCount(), 3);
            // Act2 is at the top now
            QCOMPARE(treeView->model()->index(0, 0).data().toString(), QStringLiteral("Group1: %1").arg(ag.actions[1]->text()));
            // Act1 is at the end
            QCOMPARE(treeView->model()->index(2, 0).data().toString(), QStringLiteral("Group1: %1").arg(ag.actions[0]->text()));
        }
    }
};

Q_COREAPP_STARTUP_FUNCTION(setEnvironment)

QTEST_MAIN(KCommandBarTest)

#include "kcmdbartest.moc"
