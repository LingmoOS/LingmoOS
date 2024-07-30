// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>

#include <config-drkonqi.h>
#include <coredumpwatcher.h>

#include "DetailsLoader.h"
#include "Patient.h"
#include "PatientModel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("drkonqi-coredump-gui"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("tools-report-bug"), app.windowIcon()));

    KAboutData aboutData(QStringLiteral("drkonqi"),
                         i18nc("@title CLI title", "Crashed Processes Viewer"),
                         QString::fromLatin1(PROJECT_VERSION),
                         i18nc("@info program description", "Offers detailed view of past crashes"),
                         KAboutLicense::GPL,
                         i18n("(C) 2020-2022, The DrKonqi Authors"));
    aboutData.setDesktopFileName(QStringLiteral("org.kde.drkonqi.coredump.gui"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    PatientModel model(Patient::staticMetaObject);
    qmlRegisterSingletonInstance("org.kde.drkonqi.coredump.gui", 1, 0, "PatientModel", &model);
    qmlRegisterType<DetailsLoader>("org.kde.drkonqi.coredump.gui", 1, 0, "DetailsLoader");

    KLocalizedContext i18nContext;
    i18nContext.setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(&i18nContext);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qWarning() << "ABORT ABORT";
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open, SD_JOURNAL_LOCAL_ONLY);
    Q_ASSERT(expectedJournal.ret == 0);
    Q_ASSERT(expectedJournal.value);
    CoredumpWatcher watcher(std::move(expectedJournal.value), {}, {}, nullptr);
    QObject::connect(&watcher, &CoredumpWatcher::newDump, &model, [&](const Coredump &dump) {
        model.addObject(std::make_unique<Patient>(dump));
    });
    QObject::connect(&watcher, &CoredumpWatcher::atLogEnd, &model, [&]() {
        model.setReady(true);
    });
    watcher.metaObject()->invokeMethod(&watcher, &CoredumpWatcher::start, Qt::QueuedConnection);

    return app.exec();
}
