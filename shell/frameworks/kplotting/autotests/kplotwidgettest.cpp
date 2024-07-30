/*
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kplotobject.h>
#include <kplotwidget.h>

#include <qtest_widgets.h>

#include <QBrush>

class KPlotWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        widget = new KPlotWidget();
    }

    void cleanup()
    {
        delete widget;
    }

    void testPlotObjectsDefaultSize()
    {
        // verify plotObjects is empty
        QCOMPARE(widget->plotObjects().size(), 0);
    }

    void testAddPlotObject()
    {
        // Add object
        KPlotObject *object1 = new KPlotObject();
        widget->addPlotObject(object1);

        // verify plotObjects size
        QCOMPARE(widget->plotObjects().size(), 1);

        // Add object
        KPlotObject *object2 = new KPlotObject();
        widget->addPlotObject(object2);

        QCOMPARE(widget->plotObjects().size(), 2);

        QCOMPARE(widget->plotObjects().at(0), object1);
        QCOMPARE(widget->plotObjects().at(1), object2);
    }

    void testAddPlotObjects()
    {
        // Add objects
        QList<KPlotObject *> list;
        list << new KPlotObject(Qt::red) << new KPlotObject(Qt::green) << new KPlotObject(Qt::blue);

        widget->addPlotObjects(list);

        // verify plotObjects size
        QCOMPARE(widget->plotObjects().size(), 3);

        QCOMPARE(widget->plotObjects().at(0)->brush().color(), QColor(Qt::red));
        QCOMPARE(widget->plotObjects().at(1)->brush().color(), QColor(Qt::green));
        QCOMPARE(widget->plotObjects().at(2)->brush().color(), QColor(Qt::blue));
    }

    void testRemoveAllObjects()
    {
        // Add objects
        QList<KPlotObject *> list;
        list << new KPlotObject() << new KPlotObject() << new KPlotObject();
        widget->addPlotObjects(list);

        widget->removeAllPlotObjects();
        QCOMPARE(widget->plotObjects().size(), 0);
    }

    void testReplacePlotObject()
    {
        QList<KPlotObject *> list;
        list << new KPlotObject(Qt::red) << new KPlotObject(Qt::green) << new KPlotObject(Qt::blue);

        widget->addPlotObjects(list);

        KPlotObject *object = new KPlotObject(Qt::black);

        widget->replacePlotObject(1, object);

        QCOMPARE(widget->plotObjects().at(1)->brush().color(), QColor(Qt::black));
    }

    void testBackgroundColor()
    {
        widget->setBackgroundColor(Qt::blue);
        QCOMPARE(widget->backgroundColor(), QColor(Qt::blue));
    }

    void testForegroundColor()
    {
        widget->setForegroundColor(Qt::yellow);
        QCOMPARE(widget->foregroundColor(), QColor(Qt::yellow));
    }

    void testGridColor()
    {
        widget->setGridColor(Qt::red);
        QCOMPARE(widget->gridColor(), QColor(Qt::red));
    }

    void testGridShown()
    {
        widget->setShowGrid(true);
        QCOMPARE(widget->isGridShown(), true);

        widget->setShowGrid(false);
        QCOMPARE(widget->isGridShown(), false);
    }

    void testObjectToolTipShown()
    {
        widget->setObjectToolTipShown(true);
        QCOMPARE(widget->isObjectToolTipShown(), true);

        widget->setObjectToolTipShown(false);
        QCOMPARE(widget->isObjectToolTipShown(), false);
    }

    void testAntialiasing()
    {
        widget->setAntialiasing(true);
        QCOMPARE(widget->antialiasing(), true);

        widget->setAntialiasing(false);
        QCOMPARE(widget->antialiasing(), false);
    }

private:
    KPlotWidget *widget;
};

QTEST_MAIN(KPlotWidgetTest)

#include "kplotwidgettest.moc"
