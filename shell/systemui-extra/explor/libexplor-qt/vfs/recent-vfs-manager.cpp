/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ding jing <dingjing@kylinos.cn>
 *
 */

#include "recent-vfs-manager.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>
#include <QXmlStreamWriter>

#include <gio/gio.h>
#include <stdio.h>
#include <time.h>

using namespace Peony;

RecentVFSManager* RecentVFSManager::m_instance = nullptr;

RecentVFSManager* RecentVFSManager::getInstance()
{
    if (nullptr == m_instance) {
        m_instance = new RecentVFSManager;
    }

    return m_instance;
}

void RecentVFSManager::clearAll()
{
    QFile file (m_recent_path);
    QXmlStreamWriter xmlWritter;
    xmlWritter.setAutoFormatting(true);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    xmlWritter.setDevice(&file);

    xmlWritter.writeStartDocument();
    xmlWritter.writeStartElement("xbel");
    xmlWritter.writeAttribute("version", "1.0");
    xmlWritter.writeAttribute("xmlns:bookmark", "http://www.freedesktop.org/standards/desktop-bookmarks");
    xmlWritter.writeAttribute("xmlns:mime", "http://www.freedesktop.org/standards/shared-mime-info");
    xmlWritter.writeEndElement();
    xmlWritter.writeEndDocument();
    file.close();
}

void RecentVFSManager::insert(QString uri, QString mimetype, QString name, QString exec)
{
    g_autoptr (GFile) opened_file = g_file_new_for_uri(uri.toUtf8().constData());
    g_autoptr (GFileInfo) tmp_info = g_file_query_info(opened_file, "metadata::explor-opened-count", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    QString openCountString = g_file_info_get_attribute_string(tmp_info, "metadata::explor-opened-count");
    bool hasOpenCount = false;
    int openCount = openCountString.toInt(&hasOpenCount);
    if (!hasOpenCount) {
        openCount = 0;
    }
    openCount++;
    openCountString = QString::number(openCount);
    qint64 openTime = QDateTime::currentSecsSinceEpoch();
    QString openTimeString = QString::number(openTime);
    g_file_info_set_attribute_string(tmp_info, "metadata::explor-time-opened", openTimeString.toUtf8().constData());
    g_file_info_set_attribute_string(tmp_info, "metadata::explor-opened-count", openCountString.toUtf8().constData());
    g_file_info_set_attribute_uint64(tmp_info, G_FILE_ATTRIBUTE_TIME_ACCESS, (quint64)openTime);
    g_file_set_attributes_async(opened_file, tmp_info, G_FILE_QUERY_INFO_NONE, 0, nullptr, nullptr, nullptr);

    if (!exists(uri, mimetype, name, exec)) {
        createNode(uri, mimetype, name, exec);
        write();
    }
}

RecentVFSManager::RecentVFSManager(QObject *parent) : QObject(parent)
{
    m_recent_path = QStandardPaths::locate(QStandardPaths::HomeLocation, "/.local/share/recently-used.xbel");

    QFile file (m_recent_path);
    if (!file.exists()) {
        if (m_recent_path.isEmpty()) {
            m_recent_path = QStandardPaths::locate(QStandardPaths::HomeLocation, "/.local/share",QStandardPaths::LocateDirectory)
                            + QString("/recently-used.xbel");
        }
        clearAll();
    }
    qDebug() << "recent path:" << m_recent_path;
}

bool RecentVFSManager::read()
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    QFile file (m_recent_path);
    if (!file.exists()) {
        return false;
    }

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (!m_dom_document.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
        qDebug() << "line:" << errorLine << " column:" << errorColumn << " info:" << errorStr;
        return false;
    }

    QDomElement root = m_dom_document.documentElement();
    if ("xbel" != root.tagName()) {
        qDebug () << "not xbel document";
        return false;
    } else if (root.hasAttribute("version") && "1.0" != root.attribute("version")) {
        qDebug() << "xbel version is not 1.0";
        return false;
    }

    return true;
}

bool RecentVFSManager::write()
{
//    QFile file (m_recent_path);
//    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

//    file.write(m_dom_document.toByteArray());

//    file.flush();
//    file.close();
    bool ok = g_file_set_contents(m_recent_path.toUtf8().constData(), m_dom_document.toByteArray().constData(), -1, nullptr);

    return ok;
}

bool RecentVFSManager::exists(QString uri, QString mimetype, QString name, QString exec)
{
    if (!read()) {
        qDebug() << "read error";
        return false;
    }

    QDomElement rootElement = m_dom_document.firstChildElement().firstChildElement();
    while (!rootElement.isNull()) {
        if (rootElement.hasAttribute("href") && rootElement.attribute("href") == uri) {
            qDebug() << "existed!";
            auto bookmark_file = g_bookmark_file_new();
            bool loaded = g_bookmark_file_load_from_data(bookmark_file, m_dom_document.toByteArray().constData(), -1, nullptr);
            if (loaded) {
                g_bookmark_file_set_mime_type(bookmark_file, uri.toUtf8().constData(), mimetype.toUtf8().constData());
                g_bookmark_file_add_application(bookmark_file, uri.toUtf8().constData(), name.toUtf8().constData(), exec.toUtf8().constData());
                g_bookmark_file_to_file(bookmark_file, m_recent_path.toUtf8().constData(), nullptr);
                g_bookmark_file_set_visited(bookmark_file, uri.toUtf8().constData(), g_get_monotonic_time());
            }
            g_bookmark_file_to_file (bookmark_file, m_recent_path.toUtf8().constData(), nullptr);
            g_bookmark_file_free (bookmark_file);
            return true;
        }
        rootElement = rootElement.nextSiblingElement();
    }

    return false;
}

bool RecentVFSManager::createNode(QString uri, QString mimetype, QString name, QString exec)
{
    QString dataTime = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ssZ");
    QDomElement rootElement = m_dom_document.firstChildElement();
    rootElement.setAttribute("version", "1.0");
    rootElement.setAttribute("xmlns:bookmark", "http://www.freedesktop.org/standards/desktop-bookmarks");
    rootElement.setAttribute("xmlns:mime", "http://www.freedesktop.org/standards/shared-mime-info");

    QDomElement child = m_dom_document.createElement("bookmark");
    child.setAttribute("href", uri);
    child.setAttribute("added", dataTime);
    child.setAttribute("modified", dataTime);
    child.setAttribute("visited", dataTime);

    QDomElement childInfo = m_dom_document.createElement("info");

    QDomElement childInfoMeta = m_dom_document.createElement("metadata");
    childInfoMeta.setAttribute("owner", "http://freedesktop.org");

    QDomElement childInfoMetaMime = m_dom_document.createElement("mime:mime-type");
    childInfoMetaMime.setAttribute("type", mimetype);

    QDomElement childInfoMetaApp = m_dom_document.createElement("bookmark:applications");

    // maybe not only one application
    QDomElement app = m_dom_document.createElement("bookmark:application");
    app.setAttribute("name", name);
    app.setAttribute("exec", QString("&apos;%1 %u&apos;").arg(exec));
    app.setAttribute("modified", dataTime);
    app.setAttribute("count", "1");


    childInfoMetaApp.appendChild(app);
    childInfoMeta.appendChild(childInfoMetaMime);
    childInfoMeta.appendChild(childInfoMetaApp);
    childInfo.appendChild(childInfoMeta);
    child.appendChild(childInfo);
    rootElement.appendChild(child);

    return true;
}

