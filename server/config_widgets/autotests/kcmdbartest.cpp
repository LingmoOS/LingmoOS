/*
    SPDX-FileCopyrightText: 2023 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QListView>
#include <QMainWindow>
#include <QObject>
#include <QTest>

#include "kcommandbar.h"

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
};

QTEST_MAIN(KCommandBarTest)

#include "kcmdbartest.moc"
