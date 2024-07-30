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

#ifndef TABLETDAEMON_H
#define TABLETDAEMON_H

#include <KDEDModule>
#include <QVariantList>

class QScreen;

/**
 * The wacom namespace holds all classes regarding the tablet daemon / kcmodule and applet.
 */
namespace Wacom
{
class TabletDaemonPrivate;

/**
 * This module manages the tablet device and exports a convenient DBus API
 * for tablet configuration and management.
 *
 * Its task is to monitor Hotpluging devices via solid and the detection of the tablet connected.
 * Once a tablet is recognised a default profile will be applied via the wacom::DeviceHandler.
 *
 * Profiles can be changed with the kcmodule.
 * In addition a plasma applet allows easy switching of different profiles.
 *
 * It registers the service @c "org.kde.Wacom" and exports the following objects on this service:
 *
 * @li @c /Tablet - this object. Allows one to check if a tablet is available and applies the profile
 * @li @c /Device - tablet information. Basic information about the detected tablet
 */
class TabletDaemon : public KDEDModule
{
    Q_OBJECT

public:
    /**
     * Creates a new daemon module.
     *
     * @param parent The parent object.
     * @param args   Ignored, required by KPlugin signature.
     */
    explicit TabletDaemon(QObject *parent = 0, const QVariantList &args = QVariantList());

    /**
     * Destroys this module
     */
    ~TabletDaemon() override;

public Q_SLOTS:

    /**
     * Uses the KDE notification system to display a notification to the user.
     *
     * @param eventId The event identifier.
     * @param title   The notification title.
     * @param message The notification message.
     * @param suggestConfigure Defines whether to suggest configuration.
     */
    void onNotify(const QString &eventId, const QString &title, const QString &message, bool suggestConfigure) const;

    /**
     * Called when the profile was changed.
     *
     * @param tabletId The identifier of the tablet.
     * @param profile  The name of the new profile.
     */
    void onProfileChanged(const QString &tabletId, const QString &profile);

private:
    /**
     * Sets up the global shortcut actions.
     * This method should only be called by a constructor.
     */
    void setupActions();

    /**
     * Sets up KDE application data, like i18n and the about dialog.
     * This method should only be called by a constructor.
     */
    void setupApplication();

    /**
     * Sets up the dbus interfaces.
     * This method should only be called by a constructor.
     */
    void setupDBus();

    /**
     * Sets up the X event notifier.
     * This method should only be called by a constructor.
     */
    void setupEventNotifier();

    /**
     * Helper function that sets up signals
     * monitoring screen rotations and geometry changes
     * for every existing and future screens.
     */
    void monitorAllScreensGeometry();

private Q_SLOTS:
    /**
     * Sets up signals for rotation and geometry changes
     * for a specific screen
     * @param screen Screen to monitor
     */
    void monitorScreenGeometry(QScreen *screen);

private:
    Q_DECLARE_PRIVATE(TabletDaemon)
    TabletDaemonPrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
