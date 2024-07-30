/*
    testplot_widget.h
    SPDX-FileCopyrightText: 2006 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTPLOT_WIDGET_H
#define TESTPLOT_WIDGET_H

#include <QMainWindow>

class QComboBox;
class QVBoxLayout;
class KPlotWidget;
class KPlotObject;

class TestPlot : public QMainWindow
{
    Q_OBJECT

public:
    TestPlot(QWidget *parent = nullptr);
    ~TestPlot() override
    {
    }

public Q_SLOTS:
    void slotSelectPlot(int index);

private:
    QVBoxLayout *vlay;
    QComboBox *PlotSelector;
    KPlotWidget *plot;
    KPlotObject *po1, *po2;
};

#endif
