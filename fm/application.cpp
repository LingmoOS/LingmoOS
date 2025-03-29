/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "application.h"
#include "dbusinterface.h"
#include "window.h" 
#include "thumbnailer/thumbnailprovider.h"
#include "filemanageradaptor.h"
#include "runtime/runtime.h"

#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDBusConnection>
#include <QPixmapCache>
#include <QTranslator>
#include <QFileInfo>
#include <QIcon>
#include <QDir>

// KIO
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KIO/PreviewJob>
#include <KIO/DeleteJob>
#include <KIO/DropJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KIO/RestoreJob>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_instance(false)
{
    if (QDBusConnection::sessionBus().registerService("com.lingmo.FileManager")) {
        setOrganizationName("lingmoos");
        setWindowIcon(QIcon::fromTheme("file-manager"));

        new FileManagerAdaptor(this);
        new DBusInterface;
        QDBusConnection::sessionBus().registerObject("/FileManager", this);

        // Translations
        QLocale locale;
        QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-filemanager/translations/").arg(locale.name());
        if (QFile::exists(qmFilePath)) {
            QTranslator *translator = new QTranslator(this);
            if (translator->load(qmFilePath)) {
                installTranslator(translator);
            } else {
                translator->deleteLater();
            }
        }

        m_instance = true;
    }
}

int Application::run()
{
    if (!parseCommandLineArgs())
        return 0;

    return QApplication::exec();
}

void Application::openFiles(const QStringList &paths)
{
    for (const QString &path : paths) {
        openWindow(path);
    }
}

void Application::moveToTrash(const QStringList &paths)
{
    if (paths.isEmpty())
        return;

    QList<QUrl> urls;

    for (const QString &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }

    KIO::Job *job = KIO::trash(urls);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, urls, QUrl(QStringLiteral("trash:/")), job);
}

void Application::emptyTrash()
{
    Window *w = new Window;
    w->load(QUrl("qrc:/qml/Dialogs/EmptyTrashDialog.qml"));
}

void Application::openWindow(const QString &path)
{
    Window *w = new Window;
    w->rootContext()->setContextProperty("arg", path);
    w->addImageProvider("thumbnailer", new ThumbnailProvider());
    w->load(QUrl("qrc:/qml/main.qml"));
}

QStringList Application::formatUriList(const QStringList &list)
{
    QStringList val;

    for (const QString &path : list) {
        val.append(path == "." ? QDir::currentPath() : path);
    }

    if (val.isEmpty()) {
        val.append(QDir::currentPath());
    }

    return val;
}

bool Application::parseCommandLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("File Manager"));
    parser.addHelpOption();

    parser.addPositionalArgument("files", "Files", "[FILE1, FILE2,...]");

    QCommandLineOption emptyTrashOption(QStringList() << "e" << "empty-trash" << "Empty Trash");
    parser.addOption(emptyTrashOption);

    QCommandLineOption runtimeOption(QStringList() << "r" << "runtime" << "Run");
    parser.addOption(runtimeOption);

    QCommandLineOption moveToTrashOption(QStringList() << "mtr" << "move-to-trash" << "Move To Trash");
    parser.addOption(moveToTrashOption);

    parser.process(arguments());

    if (m_instance) {
	QPixmapCache::setCacheLimit(2048);
	if (parser.isSet(runtimeOption)) {
            RunTime runtime;
        } else {
            openFiles(formatUriList(parser.positionalArguments()));
        }
    } else {
        QDBusInterface iface("com.lingmo.FileManager",
                             "/FileManager",
                             "com.lingmo.FileManager",
                             QDBusConnection::sessionBus(), this);

        if (parser.isSet(emptyTrashOption)) {
            // Empty Dialog
            iface.call("emptyTrash");
        } else if (parser.isSet(moveToTrashOption)) {
            iface.call("moveToTrash", parser.positionalArguments());
        } else {
            iface.call("openFiles", formatUriList(parser.positionalArguments()));
        }
    }

    return m_instance;
}
