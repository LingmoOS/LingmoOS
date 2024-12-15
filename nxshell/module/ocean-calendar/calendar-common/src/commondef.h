// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QString>
#include <QStandardPaths>
#include <QLoggingCategory>

const QString CalendarServiceName = "com.lingmo.dataserver.Calendar";
const QString CalendarPath = "/com/lingmo/dataserver/Calendar";

const QLoggingCategory CommonLogger("org.lingmo.ocean-calendar.common");
const QLoggingCategory ClientLogger("org.lingmo.ocean-calendar.client");
const QLoggingCategory ServiceLogger("org.lingmo.ocean-calendar.service");


#endif // COMMONDEF_H
