/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_CHARTUTIL_H
#define KUSERFEEDBACK_CONSOLE_CHARTUTIL_H

#include <qglobal.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtCharts {
class QChart;
}
using QtCharts::QChart;
#else
class QChart;
#endif

namespace KUserFeedback {
namespace Console {

namespace ChartUtil
{
void applyTheme(QChart *chart);
}

}}

#endif // KUSERFEEDBACK_CONSOLE_CHARTUTIL_H
