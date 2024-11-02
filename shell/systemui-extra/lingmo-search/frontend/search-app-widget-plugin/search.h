/*
 *
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
 */
#ifndef SEARCH_H
#define SEARCH_H
#define signals Q_SIGNALS
#define slots Q_SLOTS

#include "lingmo/kappwidgetprovider.h"
#include "lingmo/kappwidgetmanager.h"

#undef signals
#undef slots

#include "global-settings.h"
#include <QDBusInterface>

namespace LingmoUISearch {
class  AppWidgetPlugin : public AppWidget::KAppWidgetProvider

{
    Q_OBJECT
public:
    explicit AppWidgetPlugin(QString providername = "search", QObject *parent = nullptr);
    void appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value);
    void appWidgetUpdate();

Q_SIGNALS:
    void startSearch(QString);
    void start();

private:
    QDBusInterface* m_interface = nullptr;
    AppWidget::KAppWidgetManager* m_manager = nullptr;
};
}



#endif // SEARCH_H
