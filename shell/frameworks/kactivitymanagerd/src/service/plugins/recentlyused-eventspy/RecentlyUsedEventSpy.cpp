/*
 *   SPDX-FileCopyrightText: 2019 Méven Car (meven.car@kdemail.net)
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "RecentlyUsedEventSpy.h"

#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>

#include <KApplicationTrader>
#include <KDirWatch>

#include "DebugPluginRecentlyUsedEventSpy.h"

K_PLUGIN_CLASS(RecentlyUsedEventSpyPlugin)

RecentlyUsedEventSpyPlugin::RecentlyUsedEventSpyPlugin(QObject *parent)
    : Plugin(parent)
    , m_resources(nullptr)
    , m_dirWatcher(new KDirWatch(this))
    , m_lastUpdate(QDateTime::currentDateTime())
{
    // recently-used xml history file
    // usually $HOME/.local/share/recently-used.xbel
    QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/recently-used.xbel");

    m_dirWatcher->addFile(filename);
    connect(m_dirWatcher.get(), &KDirWatch::dirty, this, &RecentlyUsedEventSpyPlugin::fileUpdated);
    connect(m_dirWatcher.get(), &KDirWatch::created, this, &RecentlyUsedEventSpyPlugin::fileUpdated);
}

struct Application {
    QString name;
    QDateTime modified;
};

class Bookmark
{
public:
    QUrl href;
    QDateTime added;
    QDateTime modified;
    QDateTime visited;
    QString mimetype;
    QList<Application> applications;

    QString latestApplication() const;
};

QString Bookmark::latestApplication() const
{
    Application current = applications.first();
    for (const Application &app : applications) {
        if (app.modified > current.modified) {
            current = app;
        }
    }
    return current.name;
}

void RecentlyUsedEventSpyPlugin::fileUpdated(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(KAMD_LOG_PLUGIN_RECENTLYUSED_EVENTSPY) << "Could not read" << filename;
        return;
    }

    // must parse the xbel xml file
    QXmlStreamReader reader(&file);
    QList<Bookmark> bookmarks;
    Bookmark current;
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.qualifiedName() == QLatin1String("bookmark")) {
                current = Bookmark();
                current.href = QUrl(reader.attributes().value("href").toString());
                QString added = reader.attributes().value("added").toString();
                QString modified = reader.attributes().value("modified").toString();
                QString visited = reader.attributes().value("visited").toString();
                current.added = QDateTime::fromString(added, Qt::ISODate);
                current.modified = QDateTime::fromString(modified, Qt::ISODate);
                current.visited = QDateTime::fromString(visited, Qt::ISODate);

                // application for the current bookmark
            } else if (reader.qualifiedName() == QLatin1String("bookmark:application")) {
                Application app;

                QString exec = reader.attributes().value("exec").toString();

                if (exec.startsWith(QLatin1Char('\'')) && exec.endsWith(QLatin1Char('\''))) {
                    // remove "'" characters wrapping the command
                    exec = exec.mid(1, exec.size() - 2);
                }

                // Search for applications which are executable and case-insensitively match the search term
                const KService::List services = KApplicationTrader::query([exec](const KService::Ptr &app) {
                    return app->exec().compare(exec, Qt::CaseInsensitive) == 0;
                });

                if (!services.isEmpty()) {
                    // use the first item matching
                    const auto &service = services.first();
                    app.name = service->desktopEntryName();
                } else {
                    // when no services are found, sanitize a little the exec
                    // remove space and any character after
                    const int spaceIndex = exec.indexOf(" ");
                    if (spaceIndex != -1) {
                        exec = exec.mid(0, spaceIndex);
                    }
                    app.name = exec;
                }

                app.modified = QDateTime::fromString(reader.attributes().value("modified").toString(), Qt::ISODate);

                current.applications.append(app);
            } else if (reader.qualifiedName() == QLatin1String("mime:mime-type")) {
                current.mimetype = reader.attributes().value("type").toString();
            }
        } else if (token == QXmlStreamReader::EndElement) {
            if (reader.qualifiedName() == QLatin1String("bookmark")) {
                // keep track of the finished parsed bookmark
                bookmarks << current;
            }
        }
    }

    if (reader.hasError()) {
        qCWarning(KAMD_LOG_PLUGIN_RECENTLYUSED_EVENTSPY) << "could not parse" << file.fileName() << "error was " << reader.errorString();
        return;
    }

    // then find the files that were accessed since last run
    for (const Bookmark &mark : bookmarks) {
        if (mark.added > m_lastUpdate || mark.modified > m_lastUpdate || mark.visited > m_lastUpdate) {
            addDocument(mark.href, mark.latestApplication(), mark.mimetype);
        }
    }

    m_lastUpdate = QDateTime::currentDateTime();
}

void RecentlyUsedEventSpyPlugin::addDocument(const QUrl &url, const QString &application, const QString &mimetype)
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

RecentlyUsedEventSpyPlugin::~RecentlyUsedEventSpyPlugin()
{
}

bool RecentlyUsedEventSpyPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_resources = modules["resources"];

    return true;
}

#include "RecentlyUsedEventSpy.moc"

#include "moc_RecentlyUsedEventSpy.cpp"
