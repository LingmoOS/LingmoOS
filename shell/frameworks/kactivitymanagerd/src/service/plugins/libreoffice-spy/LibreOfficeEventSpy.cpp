/*
 *   SPDX-FileCopyrightText: 2023 Méven Car (meven@kde.org)
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "LibreOfficeEventSpy.h"

#include "DebugLibreOfficeEventSpy.h"

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>

#include <KApplicationTrader>
#include <KDirWatch>
#include <KService>

K_PLUGIN_CLASS(LibreOfficeEventSpyPlugin)

LibreOfficeEventSpyPlugin::LibreOfficeEventSpyPlugin(QObject *parent)
    : Plugin(parent)
    , m_resources(nullptr)
    , m_dirWatcher(new KDirWatch(this))
{
    // libreoffice config file
    // usually $HOME/.config/libreoffice/4/user/registrymodifications.xcu
    QString filename = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/libreoffice/4/user/registrymodifications.xcu");

    QFileInfo stat(filename);
    if (stat.exists()) {
        m_dirWatcher->addFile(filename);
        connect(m_dirWatcher.get(), &KDirWatch::dirty, this, &LibreOfficeEventSpyPlugin::fileUpdated);
        connect(m_dirWatcher.get(), &KDirWatch::created, this, &LibreOfficeEventSpyPlugin::fileUpdated);

        // call to initialize m_previousUrls
        fileUpdated(filename);
    } else {
        qCDebug(KAMD_LOG_PLUGIN_LIBREOFFICE_EVENTSPY) << "Could not read libreoffice config file: " << filename;
    }
}

void LibreOfficeEventSpyPlugin::fileUpdated(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(KAMD_LOG_PLUGIN_LIBREOFFICE_EVENTSPY) << "Could not read" << filename;
        return;
    }

    QUrl fileUrl;
    const static QString historyItemListAttr =
        QStringLiteral("/org.openoffice.Office.Histories/Histories/org.openoffice.Office.Histories:HistoryInfo['PickList']/OrderList");

    QXmlStreamReader reader(&file);
    bool m_item_is_history = false;
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.qualifiedName() == QLatin1String("item")) {
                if (reader.attributes().value("oor:path") == historyItemListAttr) {
                    m_item_is_history = true;
                }
            } else if (reader.qualifiedName() == QLatin1String("value") && m_item_is_history) {
                // found first history value
                fileUrl = QUrl(reader.readElementText());
                break;
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader.qualifiedName() == QLatin1String("node")) {
                m_item_is_history = false;
            }
        }
    }

    if (reader.hasError()) {
        qCWarning(KAMD_LOG_PLUGIN_LIBREOFFICE_EVENTSPY) << "could not parse" << file.fileName() << "error was " << reader.errorString();
        return;
    }

    if (m_init && m_lastUrl != fileUrl) {

        m_lastUrl = fileUrl;

        QMimeDatabase db;
        const auto mimeType = db.mimeTypeForUrl(fileUrl).name();
        auto app = KApplicationTrader::preferredService(mimeType);
        if (app) {
            addDocument(fileUrl, app->desktopEntryName(), mimeType);
        }
    }

    m_lastUrl = fileUrl;
    m_init = true;
}

void LibreOfficeEventSpyPlugin::addDocument(const QUrl &url, const QString &application, const QString &mimetype)
{
    const auto urlString = url.toString(QUrl::PreferLocalFile);
    Plugin::invoke<Qt::QueuedConnection>(m_resources,
                                         "RegisterResourceEvent",
                                         Q_ARG(QString, application), // Application
                                         Q_ARG(uint, 0), // Window ID
                                         Q_ARG(QString, urlString), // URI
                                         Q_ARG(uint, 0) // Event Activities::Accessed
    );

    Plugin::invoke<Qt::QueuedConnection>(m_resources,
                                         "RegisteredResourceMimetype",
                                         Q_ARG(QString, urlString), // uri
                                         Q_ARG(QString, mimetype) // mimetype
    );

    Plugin::invoke<Qt::QueuedConnection>(m_resources,
                                         "RegisterResourceTitle",
                                         Q_ARG(QString, urlString), // uri
                                         Q_ARG(QString, url.fileName()) // title
    );
}

LibreOfficeEventSpyPlugin::~LibreOfficeEventSpyPlugin()
{
}

bool LibreOfficeEventSpyPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_resources = modules["resources"];

    return true;
}

#include "LibreOfficeEventSpy.moc"
