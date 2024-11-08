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

#ifndef CLOCKPROVIDER_H
#define CLOCKPROVIDER_H

#include <kappwidgetprovider.h>

#include <QDateTime>
#include <QVariant>

namespace AppWidget {
class KAppWidgetManager;
}

class QTimer;
class QSettings;
class QDBusInterface;
class ClockProvider:public AppWidget::KAppWidgetProvider
{
public:
    ClockProvider(QString providername = "democlock",QObject *parent = nullptr);
    void appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value);
    void appWidgetUpdate();
    void appWidgetDisable();
    void appWidgetDeleted();
    void appWidgetEnable();
    void appWidgetClicked(const QString &widgetname, const QDBusVariant &value);
private:
    AppWidget::KAppWidgetManager * manager = nullptr;
    QDBusInterface *searchinterface = nullptr;
    QVariant m_date;
    QDateTime dateTime;
    QTimer *timer;
    QSettings *setting = nullptr;
};

#endif // CLOCKPROVIDER_H
