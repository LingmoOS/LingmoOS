/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "chartutil.h"

#include <QtCharts/QChart>

#include <QApplication>

using namespace KUserFeedback::Console;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namespace QtCharts;
#endif

void ChartUtil::applyTheme(QChart *chart)
{
    chart->setTheme(qApp->palette().color(QPalette::Window).lightnessF() < 0.25 ? QChart::ChartThemeDark : QChart::ChartThemeLight);
    chart->setBackgroundVisible(false);
}
