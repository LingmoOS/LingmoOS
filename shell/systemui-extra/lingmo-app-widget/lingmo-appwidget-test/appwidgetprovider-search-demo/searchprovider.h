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

#ifndef SEARCHPROVIDER_H
#define SEARCHPROVIDER_H

#include <kappwidgetprovider.h>

class QDBusInterface;
class SearchProvider : public AppWidget::KAppWidgetProvider
{
public:
    SearchProvider(QString providername = "demosearch",QObject *parent = nullptr);
    void appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value);
    void appWidgetUpdate();
    void appWidgetDisable();
    void appWidgetDeleted();
    void appWidgetEnable();
    void appWidgetClicked(const QString &widgetname, const QDBusVariant &value);

private:
    QDBusInterface *searchinterface = nullptr;
};

#endif // SEARCHPROVIDER_H
