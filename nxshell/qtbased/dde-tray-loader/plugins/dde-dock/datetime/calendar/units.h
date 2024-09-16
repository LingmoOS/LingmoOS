// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UNITS_H
#define UNITS_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QPoint>

//是否在显示时间范围内1900-2100
bool withinTimeFrame(const QDate &date);

const QPoint scaledPosition(const QPoint &xpos);

#endif // UNITS_H
