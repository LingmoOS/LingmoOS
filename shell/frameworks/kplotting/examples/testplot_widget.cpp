/*
    testplot_widget.cpp
    SPDX-FileCopyrightText: 2006 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QComboBox>
#include <QPen>
#include <QVBoxLayout>
#include <math.h>

#include "kplotaxis.h"
#include "kplotobject.h"
#include "kplotwidget.h"
#include "testplot_widget.h"

TestPlot::TestPlot(QWidget *p)
    : QMainWindow(p)
    , po1(nullptr)
    , po2(nullptr)
{
    QWidget *w = new QWidget(this);
    vlay = new QVBoxLayout(w);

    PlotSelector = new QComboBox(w);
    PlotSelector->addItem(QStringLiteral("Points plot"));
    PlotSelector->addItem(QStringLiteral("Lines plot"));
    PlotSelector->addItem(QStringLiteral("Bars plot"));
    PlotSelector->addItem(QStringLiteral("Points plot with labels"));
    PlotSelector->addItem(QStringLiteral("Points, lines and bars"));
    PlotSelector->addItem(QStringLiteral("Points, lines and bars with labels"));

    plot = new KPlotWidget(w);
    plot->setMinimumSize(400, 400);
    plot->setAntialiasing(true);
    vlay->addWidget(PlotSelector);
    vlay->addWidget(plot);

    setCentralWidget(w);

    connect(PlotSelector, qOverload<int>(&QComboBox::activated), this, &TestPlot::slotSelectPlot);

    slotSelectPlot(PlotSelector->currentIndex());
}

void TestPlot::slotSelectPlot(int n)
{
    plot->resetPlot();

    switch (n) {
    case 0: { // Points plot
        plot->setLimits(-6.0, 11.0, -10.0, 110.0);

        po1 = new KPlotObject(Qt::white, KPlotObject::Points, 4, KPlotObject::Asterisk);
        po2 = new KPlotObject(Qt::green, KPlotObject::Points, 4, KPlotObject::Triangle);

        for (float x = -5.0; x <= 10.0; x += 1.0) {
            po1->addPoint(x, x * x);
            po2->addPoint(x, 50.0 - 5.0 * x);
        }

        plot->addPlotObject(po1);
        plot->addPlotObject(po2);

        plot->update();
        break;
    }

    case 1: { // Lines plot
        plot->setLimits(-0.1, 6.38, -1.1, 1.1);
        plot->setSecondaryLimits(-5.73, 365.55, -1.1, 1.1);
        plot->axis(KPlotWidget::TopAxis)->setTickLabelsShown(true);
        plot->axis(KPlotWidget::BottomAxis)->setLabel(QStringLiteral("Angle [radians]"));
        plot->axis(KPlotWidget::TopAxis)->setLabel(QStringLiteral("Angle [degrees]"));

        po1 = new KPlotObject(Qt::red, KPlotObject::Lines, 2);
        po2 = new KPlotObject(Qt::cyan, KPlotObject::Lines, 2);

        for (float t = 0.0; t <= 6.28; t += 0.04) {
            po1->addPoint(t, sin(t));
            po2->addPoint(t, cos(t));
        }

        plot->addPlotObject(po1);
        plot->addPlotObject(po2);

        plot->update();
        break;
    }

    case 2: { // Bars plot
        plot->setLimits(-7.0, 7.0, -5.0, 105.0);

        po1 = new KPlotObject(Qt::white, KPlotObject::Bars, 2);
        po1->setBarBrush(QBrush(Qt::green, Qt::Dense4Pattern));

        for (float x = -6.5; x <= 6.5; x += 0.5) {
            po1->addPoint(x, 100 * exp(-0.5 * x * x), QLatin1String(""), 0.5);
        }

        plot->addPlotObject(po1);

        plot->update();
        break;
    }

    case 3: { // Points plot with labels
        plot->setLimits(-1.1, 1.1, -1.1, 1.1);

        po1 = new KPlotObject(Qt::yellow, KPlotObject::Points, 10, KPlotObject::Star);
        po1->setLabelPen(QPen(Qt::green));

        po1->addPoint(0.0, 0.8, QStringLiteral("North"));
        po1->addPoint(0.57, 0.57, QStringLiteral("Northeast"));
        po1->addPoint(0.8, 0.0, QStringLiteral("East"));
        po1->addPoint(0.57, -0.57, QStringLiteral("Southeast"));
        po1->addPoint(0.0, -0.8, QStringLiteral("South"));
        po1->addPoint(-0.57, -0.57, QStringLiteral("Southwest"));
        po1->addPoint(-0.8, 0.0, QStringLiteral("West"));
        po1->addPoint(-0.57, 0.57, QStringLiteral("Northwest"));

        plot->addPlotObject(po1);

        plot->update();
        break;
    }

    case 4: { // Points, Lines and Bars plot
        plot->setLimits(-2.1, 2.1, -0.1, 4.1);

        po1 = new KPlotObject(Qt::white, KPlotObject::Points, 10, KPlotObject::Pentagon);

        po1->setShowLines(true);
        po1->setShowBars(true);
        po1->setLabelPen(QPen(QColor("#AA8800")));
        po1->setLinePen(QPen(Qt::red, 3.0, Qt::DashDotLine));
        po1->setBarBrush(QBrush(Qt::blue, Qt::BDiagPattern));

        po1->addPoint(-1.75, 0.5);
        po1->addPoint(-1.25, 1.0);
        po1->addPoint(-0.75, 1.25);
        po1->addPoint(-0.25, 1.5);
        po1->addPoint(0.25, 2.5);
        po1->addPoint(0.75, 3.0);
        po1->addPoint(1.25, 1.5);
        po1->addPoint(1.75, 1.75);

        plot->addPlotObject(po1);

        update();
        break;
    }

    case 5: { // Points, Lines and Bars plot with labels
        plot->setLimits(-2.1, 2.1, -0.1, 4.1);

        po1 = new KPlotObject(Qt::white, KPlotObject::Points, 10, KPlotObject::Pentagon);

        po1->setShowLines(true);
        po1->setShowBars(true);
        po1->setLabelPen(QPen(QColor("#AA8800")));
        po1->setLinePen(QPen(Qt::red, 3.0, Qt::DashDotLine));
        po1->setBarBrush(QBrush(Qt::blue, Qt::BDiagPattern));

        po1->addPoint(-1.75, 0.5, QStringLiteral("A"));
        po1->addPoint(-1.25, 1.0, QStringLiteral("B"));
        po1->addPoint(-0.75, 1.25, QStringLiteral("C"));
        po1->addPoint(-0.25, 1.5, QStringLiteral("D"));
        po1->addPoint(0.25, 2.5, QStringLiteral("E"));
        po1->addPoint(0.75, 3.0, QStringLiteral("F"));
        po1->addPoint(1.25, 1.5, QStringLiteral("G"));
        po1->addPoint(1.75, 1.75, QStringLiteral("H"));

        plot->addPlotObject(po1);

        update();
        break;
    }
    }
}

#include "moc_testplot_widget.cpp"
