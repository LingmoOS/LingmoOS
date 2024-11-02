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

#ifndef KAPPWIDGETCOMMUNICATION_H
#define KAPPWIDGETCOMMUNICATION_H

#include <QObject>
#include <QDBusInterface>

class QGSettings;
namespace AppWidget {
class KAppWidgetManager;
class KAppWidgetConnect : public QObject
{
    Q_OBJECT

public:
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.appwidget")
    explicit KAppWidgetConnect(QObject *parent = nullptr);
    ~KAppWidgetConnect();

    Q_INVOKABLE QMap<QString, QVariant> getData();
    Q_INVOKABLE bool creatGsetting(const QString &gsetting);
    Q_INVOKABLE bool deleteGsetting(const QString &gsetting);
    Q_INVOKABLE QVariant getGsettingValue(const QString &gsetting , const QString &key);
    Q_INVOKABLE void registerAppWidget(const QString &appname, const QString &username);

signals:
    void clickedSignal(const QString &widgetname, const QDBusVariant &value);
    void defineEventSignal(const QString &eventname, const QString &widgetname, const QDBusVariant &value);
    void updatesignal();

    void gsettingChanged(const QString &key, const QVariant &value);
    void userStatusChanged(const QString &status);

public slots:
    void onClickedEvent(const QString &widgetname, const QVariant &value);
    void onDefineEvent(const QString &eventname, const QString &widgetname, const QVariant &value);
    void updateAppWidget();
    void updateUserStatus(const QString &status);

private:
    void initConnToManager(const QString &appname, const QString &username);
    void registerAppWidgetDbus(const QString &username);

    QString m_appname;
    QString m_username;
    int m_id = -1;

    /**
     * @brief gsettings容器，QMap键值对说明<生成的QGSettings的schema_id, 生成的QGSettings *对象>
     */
    QMap<QString, QGSettings *> m_gsettingmap;
};
} // namespace AppWidget
#endif // KAPPWIDGETCOMMUNICATION_H
