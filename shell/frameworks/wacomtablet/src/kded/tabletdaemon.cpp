/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tabletdaemon.h"

#include "../wacomtablet-version.h"
#include "dbustabletservice.h"
#include "globalactions.h"
#include "logging.h"
#include "tabletfinder.h"
#include "tablethandler.h"
#include "wacomadaptor.h"
#include "x11eventnotifier.h"

// common includes
#include "aboutdata.h"

// stdlib includes
#include <memory>

// KDE includes
#include <KIO/ApplicationLauncherJob>
#include <KLocalizedString>
#include <KNotification>
#include <KPluginFactory>

#include <QGuiApplication>
#include <QScreen>

#include "private/qtx11extras_p.h"

using namespace Wacom;

K_PLUGIN_FACTORY_WITH_JSON(WacomTabletFactory, "wacomtablet.json", registerPlugin<TabletDaemon>();)

namespace Wacom
{
/**
 * Private class of the TabletDaemon for the d-pointer
 */
class TabletDaemonPrivate
{
public:
    TabletDaemonPrivate()
        : tabletHandler()
        , dbusTabletService(tabletHandler)
    {
    }

    TabletHandler tabletHandler; /**< tablet handler */
    DBusTabletService dbusTabletService;
    std::shared_ptr<GlobalActions> actionCollection; /**< Collection of all global actions */

}; // CLASS
} // NAMESPACE

TabletDaemon::TabletDaemon(QObject *parent, const QVariantList &args)
    : KDEDModule(parent)
    , d_ptr(new TabletDaemonPrivate)
{
    Q_UNUSED(args);
    Q_D(TabletDaemon);

    setupApplication();
    setupDBus();
    setupEventNotifier();
    setupActions();

    // scan for connected devices
    TabletFinder::instance().scan();

    // connect profile changed handler after searching for tablets as this is only used for the global shortcut workaround.
    connect(&(d->tabletHandler), &TabletHandler::profileChanged, this, &TabletDaemon::onProfileChanged);

    // Connecting this after the device has been set up ensures that no notification is send on startup.
    connect(&(d->tabletHandler), &TabletHandler::notify, this, &TabletDaemon::onNotify);
}

TabletDaemon::~TabletDaemon()
{
    X11EventNotifier::instance().stop();
    delete this->d_ptr;
}

void TabletDaemon::onNotify(const QString &eventId, const QString &title, const QString &message, bool suggestConfigure) const
{
    KNotification *notification = new KNotification(eventId);
    notification->setComponentName(QStringLiteral("wacomtablet"));
    notification->setTitle(title);
    notification->setText(message);
    notification->setIconName(QLatin1String("preferences-desktop-tablet"));

    if (suggestConfigure) {
#if QT_VERSION_MAJOR == 5
        notification->setActions(QStringList{i18nc("Button that shows up in notification of a new tablet being connected", "Configure")});
        connect(notification, &KNotification::action1Activated, this, [notification] {
#else
        KNotificationAction *configureAction =
            notification->addAction(i18nc("Button that shows up in notification "
                                          "of a new tablet being connected",
                                          "Configure"));
        connect(configureAction, &KNotificationAction::activated, this, [notification] {
#endif
            auto *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(QStringLiteral("kcm_wacomtablet")));
            job->setStartupId(notification->xdgActivationToken().toUtf8());
            job->start();
        });
    }

    notification->sendEvent();
}

void TabletDaemon::onProfileChanged(const QString &tabletId, const QString &profile)
{
    Q_UNUSED(tabletId);
    Q_UNUSED(profile);

    // When closing the KCM module the KAction destructor disables all global shortcuts.
    // Make sure the global shortcuts are restored when a profile changes. This is not
    // optimal but at least it will enable the shortcuts again.
    qCDebug(KDED) << QLatin1String("Restoring global keyboard shortcuts...");
    setupActions();
}

void TabletDaemon::setupActions()
{
    Q_D(TabletDaemon);

    // if someone adds another action also add it to kcmodule/generalwidget.cpp

    // This method is called multiple times - make sure the action collection is only created once.
    if (!d->actionCollection) {
        d->actionCollection = std::shared_ptr<GlobalActions>(new GlobalActions(false, this));
        d->actionCollection->setConfigGlobal(true);
    }

    connect(d->actionCollection.get(), &GlobalActions::toggleTouchTriggered, &(d->tabletHandler), &TabletHandler::onToggleTouch, Qt::UniqueConnection);
    connect(d->actionCollection.get(), &GlobalActions::toggleStylusTriggered, &(d->tabletHandler), &TabletHandler::onTogglePenMode, Qt::UniqueConnection);
    connect(d->actionCollection.get(),
            &GlobalActions::toggleScreenMapTriggered,
            &(d->tabletHandler),
            &TabletHandler::onToggleScreenMapping,
            Qt::UniqueConnection);
    connect(d->actionCollection.get(), &GlobalActions::mapToFullScreenTriggered, &(d->tabletHandler), &TabletHandler::onMapToFullScreen, Qt::UniqueConnection);
    connect(d->actionCollection.get(), &GlobalActions::mapToScreen1Triggered, &(d->tabletHandler), &TabletHandler::onMapToScreen1, Qt::UniqueConnection);
    connect(d->actionCollection.get(), &GlobalActions::mapToScreen2Triggered, &(d->tabletHandler), &TabletHandler::onMapToScreen2, Qt::UniqueConnection);
    connect(d->actionCollection.get(), &GlobalActions::nextProfileTriggered, &(d->tabletHandler), &TabletHandler::onNextProfile, Qt::UniqueConnection);
    connect(d->actionCollection.get(), &GlobalActions::previousProfileTriggered, &(d->tabletHandler), &TabletHandler::onPreviousProfile, Qt::UniqueConnection);
}

void TabletDaemon::setupApplication()
{
    static AboutData about(QLatin1String("wacomtablet"),
                           i18n("Graphic Tablet Configuration daemon"),
                           QLatin1String(WACOMTABLET_VERSION_STRING),
                           i18n("A Wacom tablet control daemon"));
}

void TabletDaemon::setupDBus()
{
    Q_D(TabletDaemon);

    // connect tablet handler events to D-Bus
    // this is done here and not in the D-Bus tablet service to facilitate unit testing
    connect(&(d->tabletHandler), &TabletHandler::profileChanged, &(d->dbusTabletService), &DBusTabletService::onProfileChanged);
    connect(&(d->tabletHandler), &TabletHandler::tabletAdded, &(d->dbusTabletService), &DBusTabletService::onTabletAdded);
    connect(&(d->tabletHandler), &TabletHandler::tabletRemoved, &(d->dbusTabletService), &DBusTabletService::onTabletRemoved);
}

void TabletDaemon::setupEventNotifier()
{
    Q_D(TabletDaemon);

    // Set up monitoring for individual screen geometry changes
    monitorAllScreensGeometry();

    // Set up monitoring for screens being added, removed or reordered
    connect(qApp, &QGuiApplication::primaryScreenChanged, &(d->tabletHandler), &TabletHandler::onScreenAddedRemoved);
    connect(qApp, &QGuiApplication::screenAdded, &(d->tabletHandler), &TabletHandler::onScreenAddedRemoved);
    connect(qApp, &QGuiApplication::screenRemoved, &(d->tabletHandler), &TabletHandler::onScreenAddedRemoved);

    // Set up tablet connected/disconnected signals
    connect(&X11EventNotifier::instance(), &X11EventNotifier::tabletAdded, &TabletFinder::instance(), &TabletFinder::onX11TabletAdded);
    connect(&X11EventNotifier::instance(), &X11EventNotifier::tabletRemoved, &TabletFinder::instance(), &TabletFinder::onX11TabletRemoved);

    connect(&TabletFinder::instance(), &TabletFinder::tabletAdded, &(d->tabletHandler), &TabletHandler::onTabletAdded);
    connect(&TabletFinder::instance(), &TabletFinder::tabletRemoved, &(d->tabletHandler), &TabletHandler::onTabletRemoved);

    if (QX11Info::isPlatformX11()) {
        X11EventNotifier::instance().start();
    }
}

void TabletDaemon::monitorAllScreensGeometry()
{
    // Add existing screens
    for (const auto &screen : QGuiApplication::screens()) {
        monitorScreenGeometry(screen);
    }

    // Monitor future screens
    connect(qApp, &QGuiApplication::screenAdded, this, &TabletDaemon::monitorScreenGeometry);
}

void TabletDaemon::monitorScreenGeometry(QScreen *screen)
{
    Q_D(TabletDaemon);

    const auto &tabletHandler = &(d->tabletHandler);

    connect(screen, &QScreen::orientationChanged, [=](const Qt::ScreenOrientation &newScreenRotation) {
        tabletHandler->onScreenRotated(screen->name(), newScreenRotation);
    });

    connect(screen, &QScreen::geometryChanged, &(d->tabletHandler), &TabletHandler::onScreenGeometryChanged);
}

#include "tabletdaemon.moc"

#include "moc_tabletdaemon.cpp"
