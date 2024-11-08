/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#include "clockprovider.h"
#include <kappwidgetmanager.h>

#include <QSettings>
#include <QDir>
#include <QProcess>
#include <QCoreApplication>
#include <QDebug>

ClockProvider::ClockProvider(QString providername ,QObject *parent)
    : KAppWidgetProvider(providername , parent)
{
    if(manager) {
        manager = new AppWidget::KAppWidgetManager(this);
    }
}
void ClockProvider::appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value)
{ 
    Q_UNUSED(eventname);
    Q_UNUSED(widgetname);
    Q_UNUSED(value);
    qDebug() << "TestProvider::appWidgetRecevie";
}
void ClockProvider::appWidgetUpdate()
{
    qDebug() << "TestProvider::appWidgetUpdate";
    QStringList message;
    QString path = QDir::homePath() + "/.config/appwidgetclock/config.ini";
    setting = new QSettings(path, QSettings::IniFormat);
    dateTime = QDateTime::currentDateTime();

    setting->beginGroup("AlarmClock");
    QStringList ll = setting->allKeys();
    for (int var = 0; var < ll.count(); var++) {
        message << ll.at(var);
    }
    setting->endGroup();
    setting->sync();
    //需要将传递的消息用QDBusVariant封装，并用QVariant封装传递出去
    QVariantMap m;
    m.insert("date" , dateTime.toString("yyyy-MM-dd"));
    m.insert("AlarmClock" , message);
    qDebug() << "QVariantMap:" << m;
    if(manager) {
        manager->updateAppWidget("democlock", m);
        qDebug() << "更新";
    }

    if(setting) {
        delete setting;
    }
}
void ClockProvider::appWidgetDisable()
{
    qDebug() << "TestProvider::appWidgetDisable";
    if(manager) {
        delete manager;
    }
    qApp->exit();
}
void ClockProvider::appWidgetDeleted()
{
    qDebug() << "TestProvider::appWidgetDeleted";
}
void ClockProvider::appWidgetEnable()
{
    qDebug() << "TestProvider::appWidgetEnable";
    if(!manager) {
        manager = new AppWidget::KAppWidgetManager(this);
    }
}
void ClockProvider::appWidgetClicked(const QString &widgetname, const QDBusVariant &value)
{
    qDebug() << "TestProvider::appWidgetClicked";
    Q_UNUSED(widgetname);
    Q_UNUSED(value);
    QProcess::startDetached("appwidgetprovider-clock-demo");

}
