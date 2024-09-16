// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <QString>
#include <QStandardPaths>
#include <QLoggingCategory>

const QString CalendarServiceName = "com.deepin.dataserver.Calendar";
const QString CalendarPath = "/com/deepin/dataserver/Calendar";

const QLoggingCategory CommonLogger("org.deepin.dde-calendar.common");
const QLoggingCategory ClientLogger("org.deepin.dde-calendar.client");
const QLoggingCategory ServiceLogger("org.deepin.dde-calendar.service");


#endif // COMMONDEF_H
