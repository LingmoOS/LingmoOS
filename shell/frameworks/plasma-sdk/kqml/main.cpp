/*
 SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QQmlAbstractUrlInterceptor>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>

#include <QScopedPointer>
#include <QStringList>
#include <QTimer>

#include <KDirWatch>
#include <KLocalizedContext>
#include <KLocalizedString>

#include <PlasmaQuick/QuickViewSharedEngine>
#include <PlasmaQuick/SharedQmlEngine>

#include <stdio.h>

void noFilesGiven()
{
    printf("kqml: No files specified. Terminating.\n");
    exit(1);
}

class QmlFileChangeWatcher : public QObject, public QQmlAbstractUrlInterceptor
{
    Q_OBJECT
public:
    QmlFileChangeWatcher(std::shared_ptr<QQmlEngine>, QObject *parent);
    QUrl intercept(const QUrl &path, DataType type) override;

Q_SIGNALS:
    void dirty();

private:
    void init();
    std::unique_ptr<KDirWatch> m_fileWatcher;
    QTimer m_resetTimer;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("KDE QML Runtime");
    app.setOrganizationName("KDE");
    app.setOrganizationDomain("kde.org");
    QApplication::setApplicationVersion(QLatin1String(PROJECT_VERSION));

    QStringList files;

    // Handle main arguments
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption applicationDomainOption(QStringList{QStringLiteral("d"), QStringLiteral("domain")},
                                               QStringLiteral("The main localization domain"),
                                               QStringLiteral("domain"));
    parser.addOption(applicationDomainOption);

    // Positional arguments
    parser.addPositionalArgument("files", QStringLiteral("Any number of QML files can be loaded. They will share the same engine."), "[files...]");
    parser.addPositionalArgument("args",
                                 QStringLiteral("Arguments after '--' are ignored, but passed through to the application.arguments variable in QML."),
                                 "[-- args...]");

    parser.process(app);

    QString applicationDomain;
    if (parser.isSet(applicationDomainOption)) {
        applicationDomain = parser.value(applicationDomainOption);
    } else {
        applicationDomain = QStringLiteral("kqml");
    }

    for (QString posArg : parser.positionalArguments()) {
        if (posArg == QLatin1String("--")) {
            break;
        } else {
            files.append(posArg);
        }
    }

    if (files.size() <= 0) {
        noFilesGiven();
    }

    auto engine = PlasmaQuick::SharedQmlEngine().engine();
    auto watcher = new QmlFileChangeWatcher(engine, &app);
    QObject::connect(watcher, &QmlFileChangeWatcher::dirty, [&engine, files]() {
        engine->clearComponentCache();
        // engine->clearSingletons();
    });

    // Load files
    for (const QString &path : std::as_const(files)) {
        QUrl url = QUrl::fromUserInput(path, QDir::currentPath(), QUrl::AssumeLocalFile);
        auto window = new PlasmaQuick::QuickViewSharedEngine();
        window->setTranslationDomain(applicationDomain);
        window->setTitle(url.fileName());
        window->setSource(url);
        window->show();

        QObject::connect(watcher, &QmlFileChangeWatcher::dirty, window, [window]() {
            window->setSource(QUrl());
            delete window->rootObject();
        });

        QObject::connect(
            watcher,
            &QmlFileChangeWatcher::dirty,
            window,
            [window, url]() {
                qDebug() << "Reloading";
                qDebug() << "-------------------";
                window->setSource(url);
            },
            Qt::QueuedConnection);

        QObject::connect(window, &PlasmaQuick::QuickViewSharedEngine::statusChanged, window, [window](QQmlComponent::Status status) {
            if (status == QQmlComponent::Error) {
                qWarning() << "Error loading file";
            }
        });
        // status changed
    }

    return app.exec();
}

QmlFileChangeWatcher::QmlFileChangeWatcher(std::shared_ptr<QQmlEngine> engine, QObject *parent)
    : QObject(parent)
{
    engine->addUrlInterceptor(this);
    m_resetTimer.setInterval(1000);
    m_resetTimer.setSingleShot(true);
    connect(&m_resetTimer, &QTimer::timeout, this, [this, engine]() {
        // reset watched files
        init();
        Q_EMIT dirty();
    });
    init();
}

void QmlFileChangeWatcher::init()
{
    m_fileWatcher.reset(new KDirWatch(this));
    connect(m_fileWatcher.get(), &KDirWatch::dirty, this, [this]() {
        if (!m_resetTimer.isActive()) {
            m_resetTimer.start();
        }
    });
}

QUrl QmlFileChangeWatcher::intercept(const QUrl &path, DataType type)
{
    switch (type) {
    case QmlFile:
    case JavaScriptFile:
    case QmldirFile:
        m_fileWatcher->addFile(path.toLocalFile());
        break;
    default:
        break;
    }
    return path;
}

#include "main.moc"
