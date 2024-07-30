/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "powerdevilapp.h"

#include "powerdevilfdoconnector.h"
#include "powermanagementadaptor.h"
#include "powermanagementpolicyagentadaptor.h"

#include "powerdevil_debug.h"
#include "powerdevil_version.h"
#include "powerdevilcore.h"
#include "../osd/osd.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QFileInfo>
#include <QKeySequence>
#include <QSessionManager>
#include <QTimer>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>

#include <KActionCollection>
#include <KGlobalAccel>

#include <qnamespace.h>

using namespace Qt::StringLiterals;

PowerDevilApp::PowerDevilApp(int &argc, char **argv)
    : QGuiApplication(argc, argv)
    , m_core(nullptr)
{
}

PowerDevilApp::~PowerDevilApp()
{
    delete m_core;
}

void PowerDevilApp::init()
{
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("powerdevil"));

    KAboutData aboutData(QStringLiteral("org_kde_powerdevil"),
                         i18n("KDE Power Management System"),
                         QStringLiteral(POWERDEVIL_VERSION_STRING),
                         i18nc("@title", "PowerDevil, an advanced, modular and lightweight power management daemon"),
                         KAboutLicense::GPL,
                         i18nc("@info:credit", "(c) 2015-2019 Kai Uwe Broulik"));
    aboutData.addAuthor(i18nc("@info:credit", "Kai Uwe Broulik"), i18nc("@info:credit", "Maintainer"), QStringLiteral("kde@privat.broulik.de"));
    aboutData.addAuthor(i18nc("@info:credit", "Dario Freddi"), i18nc("@info:credit", "Previous maintainer"), QStringLiteral("drf@kde.org"));
    aboutData.setProductName("Powerdevil");

    KAboutData::setApplicationData(aboutData);

    if (QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.PowerManagement"))
        || QDBusConnection::systemBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.Policy.Power"))) {
        qCCritical(POWERDEVIL) << "KDE Power Management system not initialized, another power manager has been detected";
        return;
    }

    // not parenting Core to PowerDevilApp as it is the deleted too late on teardown
    // where the X connection is already lost leading to a crash (Bug 371127)
    m_core = new PowerDevil::Core(nullptr /*, KComponentData(aboutData)*/);

    connect(m_core, &PowerDevil::Core::coreReady, this, &PowerDevilApp::onCoreReady);

    m_core->loadCore();
}

void PowerDevilApp::onCoreReady()
{
    qCDebug(POWERDEVIL) << "Core is ready, registering various services on the bus...";
    // DBus logic for the core
    new PowerManagementAdaptor(m_core);
    new PowerDevil::FdoConnector(m_core);

    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.Solid.PowerManagement"));
    QDBusConnection::sessionBus().registerObject(QLatin1String("/org/kde/Solid/PowerManagement"), m_core);

    QDBusConnection::systemBus().interface()->registerService(u"org.freedesktop.Policy.Power"_s);

    // Start the Policy Agent service
    qDBusRegisterMetaType<QList<InhibitionInfo>>();
    qDBusRegisterMetaType<InhibitionInfo>();
    new PowerManagementPolicyAgentAdaptor(PowerDevil::PolicyAgent::instance());

    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.Solid.PowerManagement.PolicyAgent"));
    QDBusConnection::sessionBus().registerObject(QLatin1String("/org/kde/Solid/PowerManagement/PolicyAgent"), PowerDevil::PolicyAgent::instance());

    KActionCollection *coll = new KActionCollection(this);
    QAction *action = coll->addAction(QStringLiteral("powerProfile"));
    action->setText(i18n("Switch Power Profile"));
    KGlobalAccel::self()->setGlobalShortcut(action, QList<QKeySequence>{Qt::Key_Battery, Qt::MetaModifier | Qt::Key_B});
    connect(action, &QAction::triggered, this, &PowerDevilApp::showOsd);
}

int main(int argc, char **argv)
{
    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication::setAttribute(Qt::AA_DisableSessionManager);
    PowerDevilApp app(argc, argv);
    KCrash::initialize();

    bool replace = false;
    {
        QCommandLineParser parser;
        QCommandLineOption replaceOption({QStringLiteral("replace")}, i18n("Replace an existing instance"));

        parser.addOption(replaceOption);

        KAboutData aboutData = KAboutData::applicationData();
        aboutData.setupCommandLine(&parser);

        parser.process(app);
        aboutData.processCommandLine(&parser);

        replace = parser.isSet(replaceOption);
    }
    KDBusService service(KDBusService::Unique | KDBusService::StartupOption(replace ? KDBusService::Replace : 0));
    KCrash::setFlags(KCrash::AutoRestart);

    app.setQuitOnLastWindowClosed(false);
    app.setQuitLockEnabled(false);
    app.init();

    return app.exec();
}

void PowerDevilApp::showOsd() {
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.powerdevil.powerProfileOsdService"),
                                                          QStringLiteral("/org/kde/powerdevil/powerProfileOsdService"),
                                                          QStringLiteral("org.kde.powerdevil.powerProfileOsdService"),
                                                          QStringLiteral("showOsd"));
    QDBusConnection::sessionBus().asyncCall(message);
}

#include "moc_powerdevilapp.cpp"
