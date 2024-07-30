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

#include "tablethandler.h"

#include "devicetype.h"
#include "screenmap.h"
#include "screenspace.h"
#include "stringutils.h"
#include "tabletbackendfactory.h"
#include "tabletbackendinterface.h"
#include "tabletinfo.h"

// common includes
#include "deviceprofile.h"
#include "logging.h"
#include "mainconfig.h"
#include "profilemanagement.h"
#include "profilemanager.h"
#include "screensinfo.h"
#include "tabletdatabase.h"
#include "tabletprofile.h"

#include <QGuiApplication>
#include <QList>
#include <QRect>

#include <KLocalizedString>

namespace Wacom
{
class TabletHandlerPrivate
{
public:
    MainConfig mainConfig; //!< Main config file which stores general parameters.
    QString profileFile; //!< Save which profile we should use
    QHash<QString, ProfileManager *> profileManagerList; //!< Profile manager which reads profile configuration from file.
    QHash<QString, TabletBackendInterface *> tabletBackendList; //!< Tablet backend of all currently connected tablets.
    QHash<QString, TabletInformation> tabletInformationList; //!< Information of all currently connected tablets.
    QHash<QString, QString> currentProfileList; //!< Currently active profile for each tablet.
}; // CLASS
} // NAMESPACE

using namespace Wacom;

TabletHandler::TabletHandler()
    : TabletHandlerInterface(nullptr)
    , d_ptr(new TabletHandlerPrivate)
{
    Q_D(TabletHandler);

    d->profileFile = QLatin1String("tabletprofilesrc");
    d->mainConfig.open(QLatin1String("wacomtablet-kderc"));
}

TabletHandler::TabletHandler(const QString &profileFile, const QString configFile)
    : TabletHandlerInterface(nullptr)
    , d_ptr(new TabletHandlerPrivate)
{
    Q_D(TabletHandler);

    d->profileFile = profileFile;
    d->mainConfig.open(configFile);
}

TabletHandler::~TabletHandler()
{
    qDeleteAll(d_ptr->tabletBackendList);
    qDeleteAll(d_ptr->profileManagerList);
    delete d_ptr;
}

QString TabletHandler::getProperty(const QString &tabletId, const DeviceType &deviceType, const Property &property) const
{
    Q_D(const TabletHandler);

    if (!d->tabletBackendList.contains(tabletId) || d->tabletBackendList.value(tabletId) == nullptr) {
        qCWarning(KDED) << QString::fromLatin1("Unable to get property '%1' from device '%2' as no device is currently available!")
                               .arg(property.key())
                               .arg(deviceType.key());
        return QString();
    }

    return d->tabletBackendList.value(tabletId)->getProperty(deviceType, property);
}

void TabletHandler::onTabletAdded(const TabletInformation &info)
{
    Q_D(TabletHandler);

    // if we already have a device ... skip this step
    QString tabletId = info.get(TabletInfo::TabletId);
    if (d->tabletBackendList.contains(tabletId)) {
        qCDebug(KDED) << QString::fromLatin1("Ignoring tablet '%1' as another one with same name is already connected.").arg(info.get(TabletInfo::TabletId));
        return;
    }

    qCDebug(KDED) << "Taking control of new tablet" << info.get(TabletInfo::TabletName) << "(" << info.get(TabletInfo::TabletId) << ") ["
                  << (info.hasDevice(DeviceType::Stylus) ? "stylus" : "") << (info.hasDevice(DeviceType::Eraser) ? "eraser" : "")
                  << (info.hasDevice(DeviceType::Pad) ? "pad" : "") << (info.hasDevice(DeviceType::Touch) ? "touch" : "")
                  << (info.hasDevice(DeviceType::Cursor) ? "cursor" : "") << "]";

    // create tablet backend
    TabletBackendInterface *tbi = TabletBackendFactory::createBackend(info);

    if (!tbi) {
        qCWarning(KDED) << "Could not create tablet backend interface. Ignoring Tablet";
        return; // no valid backend found
    }

    d->tabletBackendList.insert(tabletId, tbi);

    // update tablet information
    d->profileManagerList.insert(tabletId, new ProfileManager(d->profileFile));
    d->tabletInformationList.insert(tabletId, info);

    // if we found something notify about it and set the default profile to it
    emit notify(QLatin1String("tabletAdded"), i18n("Tablet Connected"), i18n("New tablet '%1' connected.", info.get(TabletInfo::TabletName)), true);

    // set profile which was last used
    auto lastProfile = d->mainConfig.getLastProfile(info.getUniqueDeviceId());
    if (lastProfile.isEmpty()) {
        // try reading profile using device name for compatibility with older config
        lastProfile = d->mainConfig.getLastProfile(info.getLegacyUniqueDeviceId());
        if (!lastProfile.isEmpty()) {
            qCDebug(KDED) << "Found legacy profile setting for" << tabletId;
        }
    }

    setProfile(tabletId, lastProfile);

    // notify everyone else about the new tablet
    emit tabletAdded(info);
}

void TabletHandler::onTabletRemoved(const TabletInformation &info)
{
    Q_D(TabletHandler);

    TabletBackendInterface *tbi = d->tabletBackendList.value(info.get(TabletInfo::TabletId));
    TabletInformation ti = d->tabletInformationList.value(info.get(TabletInfo::TabletId));

    if (tbi && ti.getTabletSerial() == info.getTabletSerial()) {
        emit notify(QLatin1String("tabletRemoved"), i18n("Tablet removed"), i18n("Tablet %1 removed", ti.get(TabletInfo::TabletName)), false);

        QString tabletId = info.get(TabletInfo::TabletId);
        d->tabletBackendList.remove(tabletId);
        d->tabletInformationList.remove(tabletId);
        delete tbi;
        ProfileManager *pm = d->profileManagerList.take(tabletId);
        delete pm;

        emit tabletRemoved(tabletId);
    }
}

void TabletHandler::onScreenRotated(QString output, const Qt::ScreenOrientation &newScreenRotation)
{
    Q_D(TabletHandler);

    qCDebug(KDED) << "Screen" << output << "rotation has changed to" << newScreenRotation;

    // for each connected tablet, do the rotation
    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        QString curProfile = d->currentProfileList.value(tabletId);
        TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);
        ScreenRotation screenRotation = ScreenRotation::NONE;

        switch (newScreenRotation) {
        case Qt::PrimaryOrientation:
        case Qt::LandscapeOrientation:
            screenRotation = ScreenRotation::NONE;
            break;
        case Qt::PortraitOrientation:
            screenRotation = ScreenRotation::CW;
            break;
        case Qt::InvertedLandscapeOrientation:
            screenRotation = ScreenRotation::HALF;
            break;
        case Qt::InvertedPortraitOrientation:
            screenRotation = ScreenRotation::CCW;
            break;
        }

        // rotation has to be applied before screen mapping
        autoRotateTablet(tabletId, tabletProfile, output, screenRotation);

        // when the rotation changes, the screen mapping has to be applied again
        mapTabletToCurrentScreenSpace(tabletId, tabletProfile);
    }
}

void TabletHandler::onScreenAddedRemoved(QScreen *screen)
{
    Q_D(TabletHandler);

    Q_UNUSED(screen)
    qCDebug(KDED) << "Number of screens has changed";

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        QString curProfile = d->currentProfileList.value(tabletId);
        TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);

        mapTabletToCurrentScreenSpace(tabletId, tabletProfile);
    }
}

void TabletHandler::onScreenGeometryChanged()
{
    Q_D(TabletHandler);

    qCDebug(KDED) << "Screen geometry has changed";

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        QString curProfile = d->currentProfileList.value(tabletId);
        TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);

        mapTabletToCurrentScreenSpace(tabletId, tabletProfile);
    }
}

void TabletHandler::onTogglePenMode()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        if (!hasTablet(tabletId) || !hasDevice(tabletId, DeviceType::Stylus)) {
            continue;
        }

        // read current mode and screen space from profile
        QString curProfile = d->currentProfileList.value(tabletId);
        TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);
        DeviceProfile stylusProfile = tabletProfile.getDevice(DeviceType::Stylus);

        QString trackingMode = stylusProfile.getProperty(Property::Mode);
        ScreenSpace screenSpace(stylusProfile.getProperty(Property::ScreenSpace));

        // toggle tracking mode
        if (trackingMode.contains(QLatin1String("relative"), Qt::CaseInsensitive)) {
            trackingMode = QLatin1String("absolute");
        } else {
            // if the new mode is "relative" we have to switch to full desktop
            // as screen mappings are not supported in absolute mode
            // XXX: this doesn't seem to be nescessary?
            trackingMode = QLatin1String("relative");
            // screenSpace  = ScreenSpace::desktop();
        }

        // map tablet to output which will also save the current mode in the profile
        mapDeviceToOutput(tabletId, DeviceType::Stylus, screenSpace, trackingMode, tabletProfile);
        mapDeviceToOutput(tabletId, DeviceType::Eraser, screenSpace, trackingMode, tabletProfile);

        d->profileManagerList.value(tabletId)->saveProfile(tabletProfile);
    }
}

void TabletHandler::onToggleTouch()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        if (!hasDevice(tabletId, DeviceType::Touch)) {
            continue;
        }

        QString touchMode = getProperty(tabletId, DeviceType::Touch, Property::Touch);

        // also save the touch on/off into the profile to remember the user selection after
        // the tablet was reconnected
        QString curProfile = d->currentProfileList.value(tabletId);
        TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);
        DeviceProfile touchProfile = tabletProfile.getDevice(DeviceType::Touch);

        if (touchMode.compare(QLatin1String("off"), Qt::CaseInsensitive) == 0) {
            setProperty(tabletId, DeviceType::Touch, Property::Touch, QLatin1String("on"));
            touchProfile.setProperty(Property::Touch, QLatin1String("on"));
        } else {
            setProperty(tabletId, DeviceType::Touch, Property::Touch, QLatin1String("off"));
            touchProfile.setProperty(Property::Touch, QLatin1String("off"));
        }

        tabletProfile.setDevice(touchProfile);
        d->profileManagerList.value(tabletId)->saveProfile(tabletProfile);
    }
}

void TabletHandler::onToggleScreenMapping()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        if (!hasTablet(tabletId)) {
            continue;
        }

        QString curProfile = d->currentProfileList.value(tabletId);
        TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);
        DeviceProfile stylusProfile = tabletProfile.getDevice(DeviceType::Stylus);
        ScreenSpace screenSpace = ScreenSpace(stylusProfile.getProperty(Property::ScreenSpace));

        mapPenToScreenSpace(tabletId, screenSpace.next());
    }
}

void TabletHandler::onMapToFullScreen()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        mapPenToScreenSpace(tabletId, ScreenSpace::desktop().toString());
    }
}

void TabletHandler::onMapToScreen1()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        mapPenToScreenSpace(tabletId, ScreenSpace::monitor(ScreensInfo::getPrimaryScreenName()));
    }
}

void TabletHandler::onMapToScreen2()
{
    Q_D(TabletHandler);

    if (QGuiApplication::screens().count() > 1) {
        foreach (const QString &tabletId, d->tabletInformationList.keys()) {
            mapPenToScreenSpace(tabletId, ScreenSpace::monitor(ScreensInfo::getPrimaryScreenName()).next());
        }
    }
}

void TabletHandler::onNextProfile()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        if (d->profileManagerList.value(tabletId)->profileRotationList().empty()) {
            qCDebug(KDED) << "No items in the rotation list. Nothing to rotate";
        } else {
            QString nextProfile = d->profileManagerList.value(tabletId)->nextProfile();
            setProfile(tabletId, nextProfile);
        }
    }
}

void TabletHandler::onPreviousProfile()
{
    Q_D(TabletHandler);

    foreach (const QString &tabletId, d->tabletInformationList.keys()) {
        if (d->profileManagerList.value(tabletId)->profileRotationList().empty()) {
            qCDebug(KDED) << "No items in the rotation list. Nothing to rotate";
        } else {
            QString previousProfile = d->profileManagerList.value(tabletId)->previousProfile();
            setProfile(tabletId, previousProfile);
        }
    }
}

QStringList TabletHandler::listProfiles(const QString &tabletId)
{
    Q_D(TabletHandler);

    ProfileManager *pm = d->profileManagerList.value(tabletId);
    if (!pm) {
        qCWarning(KDED) << "Could not retrieve ProfileManager for tablet" << tabletId;
        return QStringList();
    }

    const TabletInformation ti = d->tabletInformationList.value(tabletId);
    pm->readProfiles(ti.getUniqueDeviceId());
    return pm->listProfiles();
}

void TabletHandler::setProfile(const QString &tabletId, const QString &profile)
{
    Q_D(TabletHandler);

    qCDebug(KDED) << QString::fromLatin1("Loading tablet profile '%1' for device '%2'...").arg(profile).arg(tabletId);

    if (!hasTablet(tabletId)) {
        qCWarning(KDED) << QString::fromLatin1("Can not set tablet profile to '%1' as no backend is available!").arg(profile);
        return;
    }

    ProfileManager *profileManager = d->profileManagerList.value(tabletId);

    if (!profileManager) {
        qCWarning(KDED) << "Could not retrieve ProfileManager for tablet" << tabletId;
        return;
    }

    TabletInformation tabletInformation = d->tabletInformationList.value(tabletId);
    profileManager->readProfiles(tabletInformation.getUniqueDeviceId(), tabletInformation.getLegacyUniqueDeviceId());
    TabletProfile tabletProfile = profileManager->loadProfile(profile);

    if (tabletProfile.listDevices().isEmpty()) {
        // may happen also if "last selected profile" was deleted.
        // thus we check if any profile exist and take the first one
        //  or create a new empty profile and apply this instead
        QStringList pList = profileManager->listProfiles();

        if (pList.isEmpty()) {
            // create a new default profile
            const QString vendorId = tabletInformation.get(TabletInfo::CompanyId);
            const QString deviceName = QString::fromLatin1("%1:%2").arg(vendorId).arg(tabletId);
            ProfileManagement *profileManagement = &ProfileManagement::instance(deviceName, tabletInformation.hasDevice(DeviceType::Touch));
            profileManagement->createNewProfile(i18nc("Name of the default profile that will be created if none exists.", "Default"));

            if (!profileManagement->availableProfiles().empty()) {
                d->currentProfileList.insert(tabletId, profileManagement->availableProfiles().first());
            } else {
                qCWarning(KDED) << "Could not create new default profile. There seems to be an error on device detection";
            }
        } else {
            qCWarning(KDED) << QString::fromLatin1("Tablet profile '%1' does not exist!").arg(profile);
            emit notify(QLatin1String("tabletError"),
                        i18n("Graphic Tablet error"),
                        i18n("Profile <b>%1</b> does not exist. Apply <b>%2</b> instead", profile, pList.first()),
                        true);

            // set first known profile instead
            d->currentProfileList.insert(tabletId, pList.first());
        }

        tabletProfile = profileManager->loadProfile(d->currentProfileList.value(tabletId));
    } else {
        // set profile
        d->currentProfileList.insert(tabletId, profile);
    }

    // Handle auto-rotation.
    // This has to be done before screen mapping!
    autoRotateTablet(tabletId, tabletProfile);

    // Map tablet to screen.
    // This is necessary to ensure the correct area map is used. Somone might have changed
    // the ScreenSpace property without updating the Area property.
    mapTabletToCurrentScreenSpace(tabletId, tabletProfile);

    // set profile on tablet
    QString currentProfile = d->currentProfileList.value(tabletId);
    d->tabletBackendList.value(tabletId)->setProfile(tabletProfile);
    d->mainConfig.setLastProfile(tabletInformation.getUniqueDeviceId(), currentProfile);

    // check profile rotation values and LEDs
    profileManager->updateCurrentProfileNumber(currentProfile);
    d->tabletBackendList.value(tabletId)->setStatusLED(profileManager->profileNumber(currentProfile));
    d->tabletBackendList.value(tabletId)->setStatusLEDBrightness(32); // TODO: Read the brightness from the settings. Add support to the kcmodule GUI for that.

    emit profileChanged(tabletId, currentProfile);

    QString touchSensorId = tabletInformation.get(TabletInfo::TouchSensorId);
    // TODO: check if for some reason someone will make an infinitely nested tablets
    if (!touchSensorId.isEmpty() && touchSensorId != tabletId) {
        setProfile(touchSensorId, profile);
    }
}

void TabletHandler::setProperty(const QString &tabletId, const DeviceType &deviceType, const Property &property, const QString &value)
{
    Q_D(TabletHandler);

    if (!hasTablet(tabletId)) {
        qCWarning(KDED) << QString::fromLatin1("Unable to set property '%1' on device '%2' to '%3' as no device is currently available!")
                               .arg(property.key())
                               .arg(deviceType.key())
                               .arg(value);
        return;
    }

    d->tabletBackendList.value(tabletId)->setProperty(deviceType, property, value);
}

QStringList TabletHandler::getProfileRotationList(const QString &tabletId)
{
    Q_D(TabletHandler);

    if (!hasTablet(tabletId)) {
        qCWarning(KDED) << QString::fromLatin1("Unable to get profile rotation list as no device is currently available!");
        return QStringList();
    }

    return d->profileManagerList.value(tabletId)->profileRotationList();
}

void TabletHandler::setProfileRotationList(const QString &tabletId, const QStringList &rotationList)
{
    Q_D(TabletHandler);

    if (!hasTablet(tabletId)) {
        qCWarning(KDED) << QString::fromLatin1("Unable to set profile rotation list as no device is currently available!");
        return;
    }

    d->profileManagerList.value(tabletId)->setProfileRotationList(rotationList);
}

void TabletHandler::autoRotateTablet(const QString &tabletId, const TabletProfile &tabletProfile, QString output, ScreenRotation screenRotation)
{
    // determine auto-rotation configuration
    DeviceProfile stylusProfile = tabletProfile.getDevice(DeviceType::Stylus);

    QString rotateProperty = stylusProfile.getProperty(Property::Rotate);
    const ScreenRotation *lookupRotation = ScreenRotation::find(rotateProperty);
    ScreenRotation tabletRotation = (lookupRotation != nullptr) ? *lookupRotation : ScreenRotation::NONE;

    bool doAutoInvert = (tabletRotation == ScreenRotation::AUTO_INVERTED);
    bool doAutoRotation = (doAutoInvert || tabletRotation == ScreenRotation::AUTO);

    if (!doAutoRotation) {
        qCDebug(KDED) << "Auto-rotation is disabled in profile settings";
        return;
    }

    ScreenSpace stylusSpace = ScreenSpace(stylusProfile.getProperty(Property::ScreenSpace));
    if (!stylusSpace.isMonitor() && QGuiApplication::screens().count() > 1) {
        qCDebug(KDED) << "We're not mapped to a specific display, can't determine auto-rotation";
        return;
    }

    if (output.isEmpty()) {
        screenRotation = ScreensInfo::getScreenRotation(stylusSpace.toString());
    } else if (output != stylusSpace.toString() && QGuiApplication::screens().count() > 1) {
        qCDebug(KDED) << "Tablet is mapped to a different screen";
        return;
    }

    // determine new rotation and set it
    ScreenRotation newRotation = (doAutoInvert) ? screenRotation.invert() : screenRotation;

    qCDebug(KDED) << "Rotate tablet :: " << newRotation.key();

    setProperty(tabletId, DeviceType::Stylus, Property::Rotate, newRotation.key());
    setProperty(tabletId, DeviceType::Eraser, Property::Rotate, newRotation.key());

    if (hasDevice(tabletId, DeviceType::Touch)) {
        setProperty(tabletId, DeviceType::Touch, Property::Rotate, newRotation.key());
    }
}

bool TabletHandler::hasDevice(const QString &tabletId, const DeviceType &type) const
{
    Q_D(const TabletHandler);

    return (hasTablet(tabletId) && d->tabletInformationList.value(tabletId).hasDevice(type));
}

bool TabletHandler::hasTablet(const QString &tabletId) const
{
    Q_D(const TabletHandler);

    return (d->tabletBackendList.contains(tabletId) && d->tabletBackendList.value(tabletId) != nullptr);
}

void TabletHandler::mapDeviceToOutput(const QString &tabletId,
                                      const DeviceType &device,
                                      const ScreenSpace &screenSpace,
                                      const QString &trackingMode,
                                      TabletProfile &tabletProfile)
{
    if (!hasTablet(tabletId) || !hasDevice(tabletId, device)) {
        return; // we do not have a tablet or the requested device
    }

    ScreenSpace screen(screenSpace);

    // if the screen is missing or it's the only screen, use desktop instead
    // however do not override this in the saved profile, because it breaks user
    // settings if they disconnect external tablet
    const bool screen_is_valid =
        !screen.isMonitor() || (QGuiApplication::screens().count() > 1 && ScreensInfo::getScreenGeometries().contains(screen.toString()));

    DeviceProfile deviceProfile = tabletProfile.getDevice(device);
    ScreenMap screenMap(deviceProfile.getProperty(Property::ScreenMap));
    QString tabletArea = screenMap.getMappingAsString(screen);

    setProperty(tabletId, device, Property::Mode, trackingMode);
    setProperty(tabletId, device, Property::ScreenSpace, screen_is_valid ? screen.toString() : ScreenSpace::desktop().toString());
    setProperty(tabletId, device, Property::Area, tabletArea);

    deviceProfile.setProperty(Property::Mode, trackingMode);
    deviceProfile.setProperty(Property::ScreenSpace, screen.toString());
    deviceProfile.setProperty(Property::Area, tabletArea);

    tabletProfile.setDevice(deviceProfile);
}

void TabletHandler::mapPenToScreenSpace(const QString &tabletId, const ScreenSpace &screenSpace, const QString &trackingMode)
{
    Q_D(TabletHandler);

    if (!hasTablet(tabletId)) {
        return; // we do not have a tablet
    }

    QString curProfile = d->currentProfileList.value(tabletId);
    TabletProfile tabletProfile = d->profileManagerList.value(tabletId)->loadProfile(curProfile);

    mapDeviceToOutput(tabletId, DeviceType::Stylus, screenSpace, trackingMode, tabletProfile);
    mapDeviceToOutput(tabletId, DeviceType::Eraser, screenSpace, trackingMode, tabletProfile);

    d->profileManagerList.value(tabletId)->saveProfile(tabletProfile);
}

void TabletHandler::mapTabletToCurrentScreenSpace(const QString &tabletId, TabletProfile &tabletProfile)
{
    Q_D(TabletHandler);

    DeviceProfile stylusProfile = tabletProfile.getDevice(DeviceType::Stylus);
    DeviceProfile touchProfile = tabletProfile.getDevice(DeviceType::Touch);

    QString stylusMode = stylusProfile.getProperty(Property::Mode);
    ScreenSpace stylusSpace = ScreenSpace(stylusProfile.getProperty(Property::ScreenSpace));
    QString touchMode = touchProfile.getProperty(Property::Mode);
    ScreenSpace touchSpace = ScreenSpace(touchProfile.getProperty(Property::ScreenSpace));

    mapDeviceToOutput(tabletId, DeviceType::Stylus, stylusSpace, stylusMode, tabletProfile);
    mapDeviceToOutput(tabletId, DeviceType::Eraser, stylusSpace, stylusMode, tabletProfile);
    mapDeviceToOutput(tabletId, DeviceType::Touch, touchSpace, touchMode, tabletProfile);

    d->profileManagerList.value(tabletId)->saveProfile(tabletProfile);
}

#include "moc_tablethandler.cpp"
