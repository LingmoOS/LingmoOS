/*
    testplot_main.cpp
    SPDX-FileCopyrightText: 2006 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "testplot_widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestPlot *tp = new TestPlot(nullptr);
    tp->show();
    return a.exec();
}
