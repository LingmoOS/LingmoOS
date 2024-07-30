/*
    This file is part of the KDE frameworks
    SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <kcolumnresizertest.h>

#include <KColumnResizer>

#include "ui_kcolumnresizertest-forms.h"
#include "ui_kcolumnresizertest-grid-and-form.h"
#include "ui_kcolumnresizertest-grids.h"
#include <QTest>

QTEST_MAIN(KColumnResizerTest)

void KColumnResizerTest::test_data()
{
    QTest::addColumn<QWidget *>("parent");

    QWidget *forms = new QWidget;
    Ui::KColumnResizerTestForms().setupUi(forms);
    QTest::newRow("forms") << forms;

    QWidget *grids = new QWidget;
    Ui::KColumnResizerTestGrids().setupUi(grids);
    QTest::newRow("grids") << grids;

    QWidget *gridAndForm = new QWidget;
    Ui::KColumnResizerTestGridAndForms().setupUi(gridAndForm);
    QTest::newRow("grid-and-form") << gridAndForm;
}

void KColumnResizerTest::test()
{
    // This test checks the x coordinate of the widgets from the column
    // immediately after the resized column, rather than checking the width of
    // the resized column itself because checking the width of the column cannot
    // be done the same way for all layout types.
    QFETCH(QWidget *, parent);
    QVERIFY(parent);

    auto layout1 = parent->findChild<QLayout *>(QStringLiteral("layout1"));
    auto layout2 = parent->findChild<QLayout *>(QStringLiteral("layout2"));
    auto widget1 = parent->findChild<QWidget *>(QStringLiteral("widget1"));
    auto widget2 = parent->findChild<QWidget *>(QStringLiteral("widget2"));
    QVERIFY(layout1);
    QVERIFY(layout2);
    QVERIFY(widget1);
    QVERIFY(widget2);

    // Show the widget so that geometries are updated
    parent->show();

    int widget1x = widget1->x();
    int widget2x = widget2->x();
    QVERIFY(widget1x < widget2x);

    auto resizer = new KColumnResizer(parent);
    resizer->addWidgetsFromLayout(layout1);
    resizer->addWidgetsFromLayout(layout2);

    // Wait for resizer to do the work
    QCoreApplication::processEvents();
    // Now wait for the layout change to propagate
    QCoreApplication::processEvents();

    QCOMPARE(widget1->x(), widget2x);
    QCOMPARE(widget2->x(), widget2x);

    delete parent;
}

#include "moc_kcolumnresizertest.cpp"
