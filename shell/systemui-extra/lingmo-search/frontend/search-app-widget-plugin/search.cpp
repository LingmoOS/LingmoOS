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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "search.h"

#include <QDBusPendingCall>
#include <QDBusMetaType>
#include <QTranslator>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QVariantMap>

using namespace LingmoUISearch;

AppWidgetPlugin::AppWidgetPlugin(QString providername, QObject *parent) : KAppWidgetProvider(providername, parent)
{
    QTranslator* translator = new QTranslator(this);
    if(!translator->load("/usr/share/appwidget/search/translations/" + QLocale::system().name()+ ".qm")) {
        qWarning() << "/usr/share/appwidget/search/translations/" + QLocale::system().name() << "load failed";
    }
    QApplication::installTranslator(translator);

    m_interface = new QDBusInterface("com.lingmo.search.service",
                                     "/",
                                     "org.lingmo.search.service");

    m_manager = new AppWidget::KAppWidgetManager(this);

    connect(&GlobalSettings::getInstance(), &GlobalSettings::valueChanged, [ & ] (const QString& key, QVariant value) {
        if (key == TRANSPARENCY_KEY || key == STYLE_NAME_KEY) {
            this->appWidgetUpdate();
            qDebug() << key << " has changed to" << value;
        }
    });
}

void AppWidgetPlugin::appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value)
{
    qDebug() << widgetname << value.variant();
//    if (eventname == "enter" or eventname == "return") {
//        QString keyword = value.variant().toString();
//        Q_EMIT this->startSearch(keyword);
//    } else
    if (eventname == "clicked") {
        Q_EMIT this->start();
    } else {
        qWarning() << "event:" << eventname << "has no trigger now.";
    }
}

void AppWidgetPlugin::appWidgetUpdate()
{
    QString theme = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString();
    QJsonObject obj;
    if (theme == "lingmo-dark") {
        obj.insert("red", QJsonValue(0));
        obj.insert("green", QJsonValue(0));
        obj.insert("blue", QJsonValue(0));
        obj.insert("placeHolderTextColor", QJsonValue("#72ffffff"));
    } else {
        obj.insert("red", QJsonValue(255));
        obj.insert("green", QJsonValue(255));
        obj.insert("blue", QJsonValue(255));
        obj.insert("placeHolderTextColor", QJsonValue("#72000000"));
    }

    obj.insert("alpha", QJsonValue(GlobalSettings::getInstance().getValue(TRANSPARENCY_KEY).toDouble()));
    QString jsonData = QString(QJsonDocument(obj).toJson());

    QVariantMap dataMap;
    dataMap.insert("colorData", QVariant(jsonData));

//    variant.setVariant(QGuiApplication::palette().color(QPalette::Window).name(QColor::HexArgb));
    if(m_manager) {
        m_manager->updateAppWidget("search", dataMap);
        qDebug() <<"==========update style and transparency successful.";
    }
}
