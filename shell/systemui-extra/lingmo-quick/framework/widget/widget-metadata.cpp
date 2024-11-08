/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "widget-metadata.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonArray>
#include <QLocale>

#include <QDebug>

using namespace LingmoUIQuick;

WidgetMetadata::WidgetMetadata(const QString &root) : m_root(root)
{
    init();
}

void WidgetMetadata::init()
{
    if (!m_root.entryList(QDir::Files).contains("metadata.json")) {
        qWarning() << "loadWidgetPackage: can not find metadata.json, path:" << m_root.path();
        return;
    }

    QFile file(m_root.absoluteFilePath("metadata.json"));
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QTextStream iStream(&file);
    iStream.setCodec("UTF-8");

    QJsonParseError parseError {};
    QJsonDocument document = QJsonDocument::fromJson(iStream.readAll().toUtf8(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "metadata.json parse error:" << parseError.errorString();
        return;
    }

    if (!document.isObject()) {
        qWarning() << "metadata.json parse error: Is not object.";
        return;
    }

    QJsonObject object = document.object();
    QJsonValue id = object.value(QLatin1String("Id"));
    if (id.type() == QJsonValue::Undefined) {
        qWarning() << "metadata: Id is undefined.";
        return;
    }

    if ((m_id = id.toString()) != m_root.dirName()) {
        qWarning() << "metadata: dir not equal id.";
        return;
    }

    m_object = object;
    m_isValid = true;
}

const QDir &WidgetMetadata::root() const
{
    return m_root;
}

bool WidgetMetadata::isValid() const
{
    return m_isValid;
}

QString WidgetMetadata::id() const
{
    return m_id;
}

QString WidgetMetadata::icon() const
{
    QJsonValue value = m_object.value(QLatin1String("Icon"));
    if (value.type() == QJsonValue::Undefined) {
        return "application-x-desktop";
    }

    return value.toString();
}

QString WidgetMetadata::name() const
{
    QJsonValue value = m_object.value(localeKey("Name"));
    if (value.type() != QJsonValue::Undefined) {
        return value.toString();
    }

    value = m_object.value(QLatin1String("Name"));
    if (value.type() == QJsonValue::Undefined) {
        return m_id;
    }

    return value.toString();
}

QString WidgetMetadata::tooltip() const
{
    QJsonValue value = m_object.value(localeKey("Tooltip"));
    if (value.type() != QJsonValue::Undefined) {
        return value.toString();
    }

    value = m_object.value(QLatin1String("Tooltip"));
    if (value.type() == QJsonValue::Undefined) {
        return m_id;
    }

    return value.toString();
}

QString WidgetMetadata::version() const
{
    QJsonValue value = m_object.value(QLatin1String("Version"));
    if (value.type() == QJsonValue::Undefined) {
        return "";
    }

    return value.toString();
}

QString WidgetMetadata::website() const
{
    QJsonValue value = m_object.value(QLatin1String("Website"));
    if (value.type() == QJsonValue::Undefined) {
        return "";
    }

    return value.toString();
}

QString WidgetMetadata::bugReport() const
{
    QJsonValue value = m_object.value(QLatin1String("BugReport"));
    if (value.type() == QJsonValue::Undefined) {
        return "https://gitee.com/lingmo/lingmo-panel/issues";
    }

    return value.toString();
}

QString WidgetMetadata::description() const
{
    QJsonValue value = m_object.value(localeKey("Description"));
    if (value.type() != QJsonValue::Undefined) {
        return value.toString();
    }

    value = m_object.value(QLatin1String("Description"));
    if (value.type() == QJsonValue::Undefined) {
        return m_id;
    }

    return value.toString();
}

QVariantList WidgetMetadata::authors() const
{
    QJsonValue value = m_object.value(QLatin1String("Authors"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::Array) {
        return {};
    }

    return value.toArray().toVariantList();
}

QString WidgetMetadata::localeKey(const QString &key)
{
    return (key + "[" + QLocale::system().name() + "]");
}

QVariantMap WidgetMetadata::contents() const
{
    QJsonValue value = m_object.value(QLatin1String("Contents"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::Object) {
        return {};
    }

    return value.toObject().toVariantMap();
}

WidgetMetadata::Hosts WidgetMetadata::showIn() const
{
    QJsonValue value = m_object.value(QLatin1String("ShowIn"));
    if (value.type() == QJsonValue::Undefined || value.type() != QJsonValue::String) {
        return All;
    }
    QStringList hostList = value.toString().split(",");
    if (hostList.isEmpty()) {
        return All;
    }
    Hosts hosts = Undefined;
    for (const QString &host : hostList) {
        if (host == QStringLiteral("Panel")) {
            hosts |= Panel;
        } else if (host == QStringLiteral("SideBar")) {
            hosts |= SideBar;
        } else if (host == QStringLiteral("Desktop")) {
            hosts |= Desktop;
        } else if (host == QStringLiteral("All")) {
            hosts |= All;
        } else if (host == QStringLiteral("TaskManager")) {
            hosts |= TaskManager;
        }
    }
    return hosts;
}

Types::WidgetType WidgetMetadata::widgetType() const
{
    QJsonValue value = m_object.value(QLatin1String("WidgetType"));
    if (value.toString() == QStringLiteral("Container")) {
        return Types::Container;
    }

    return Types::Widget;
}
