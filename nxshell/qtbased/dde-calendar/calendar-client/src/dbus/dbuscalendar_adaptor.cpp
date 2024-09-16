// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscalendar_adaptor.h"
#include "calendarmainwindow.h"
#include "scheduledatamanage.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QWidget>

/*
 * Implementation of adaptor class CalendarAdaptor
 */

CalendarAdaptor::CalendarAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

CalendarAdaptor::~CalendarAdaptor()
{
    // destructor
}

void CalendarAdaptor::ActiveWindow()
{
    // handle method call com.deepin.Calendar.RaiseWindow
    QMetaObject::invokeMethod(parent(), "ActiveWindow");
}

void CalendarAdaptor::RaiseWindow()
{
    QWidget *pp = qobject_cast<QWidget *>(parent());
    //取消最小化状态
    pp->setWindowState(pp->windowState() & ~Qt::WindowState::WindowMinimized);
    pp->activateWindow();
    pp->raise();
}

void CalendarAdaptor::OpenSchedule(QString job)
{
    //更新对应的槽函数
    QMetaObject::invokeMethod(parent(), "slotOpenSchedule", Q_ARG(QString, job));
}



