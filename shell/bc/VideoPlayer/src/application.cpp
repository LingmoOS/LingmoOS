/*
 * SPDX-FileCopyrightText: 2021 Reion Wong <reion@cuteos.com>
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "application.h"
#include "mpvobject.h"
#include "playlistmodel.h"
#include "playlistitem.h"
#include "tracksmodel.h"
#include "track.h"
#include "lockmanager.h"
#include "mpris2/mediaplayer2.h"
#include "mpris2/mediaplayer2player.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QTime>
#include <QDBusConnection>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeDatabase>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QStandardPaths>
#include <QStyle>
#include <QTime>
#include <QStyleFactory>
#include <QThread>

#include <QTranslator>
#include <QLocale>

#include <memory>

static QApplication *createApplication(int &argc, char **argv, const QString &applicationName)
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setOrganizationName("lingmoos");
    QApplication::setApplicationName(applicationName);
    QApplication::setOrganizationDomain("lingmo.org");
    QApplication::setApplicationDisplayName("Video Player");
    QApplication::setWindowIcon(QIcon::fromTheme("lingmo-videoplayer"));
    QApplication *app = new QApplication(argc, argv);
    return app;
}

Application::Application(int &argc, char **argv, const QString &applicationName)
    : m_app(createApplication(argc, argv, applicationName))
{
    // register mpris dbus service
    QString mspris2Name(QStringLiteral("org.mpris.MediaPlayer2.lingmo.videoplayer"));
    QDBusConnection::sessionBus().registerService(mspris2Name);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/mpris/MediaPlayer2"), this, QDBusConnection::ExportAdaptors);
    // org.mpris.MediaPlayer2 mpris2 interface
    new MediaPlayer2(this);

    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-videoplayer/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QApplication::instance());
        if (translator->load(qmFilePath)) {
            QApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    m_engine = new QQmlApplicationEngine(this);
    setupCommandLineParser();
    registerQmlTypes();
    setupQmlContextProperties();

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    auto onObjectCreated = [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    };
    QObject::connect(m_engine, &QQmlApplicationEngine::objectCreated,
                     m_app, onObjectCreated, Qt::QueuedConnection);
    m_engine->addImportPath("qrc:/qml");

    m_engine->load(url);
}

int Application::run()
{
    return m_app->exec();
}

QString Application::mimeType(const QString &file)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(file);
    return mime.name();
}

QString Application::formatTime(const double time)
{
    QTime t(0, 0, 0);
    QString formattedTime = t.addSecs(static_cast<qint64>(time)).toString("hh:mm:ss");
    return formattedTime;
}

QString Application::argument(int key)
{
    return m_args[key];
}

void Application::addArgument(int key, const QString &value)
{
    m_args.insert(key, value);
}

void Application::hideCursor()
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void Application::showCursor()
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void Application::setupCommandLineParser()
{
    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("file"), "File to open");
    parser.process(*m_app);

    for (auto i = 0; i < parser.positionalArguments().size(); ++i) {
        addArgument(i, parser.positionalArguments().at(i));
    }
}

void Application::setupQmlContextProperties()
{
    // C++ 14
    std::unique_ptr<LockManager> lockManager = std::make_unique<LockManager>();

    m_engine->rootContext()->setContextProperty(QStringLiteral("app"), this);
    qmlRegisterUncreatableType<Application>("Application", 1, 0, "Application",
                                            QStringLiteral("Application should not be created in QML"));

    m_engine->rootContext()->setContextProperty(QStringLiteral("lockManager"), lockManager.release());
    qmlRegisterUncreatableType<LockManager>("LockManager", 1, 0, "LockManager",
                                            QStringLiteral("LockManager should not be created in QML"));

    m_engine->rootContext()->setContextProperty(QStringLiteral("mediaPlayer2Player"), new MediaPlayer2Player(this));
}

void Application::registerQmlTypes()
{
    qmlRegisterType<MpvObject>("mpv", 1, 0, "MpvObject");
    qRegisterMetaType<PlayListModel *>();
    qRegisterMetaType<PlayListItem *>();
    qRegisterMetaType<QAction *>();
    qRegisterMetaType<TracksModel *>();
}
