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

#ifndef TABLETFINDER_H
#define TABLETFINDER_H

#include "tabletinformation.h"

#include <QObject>

namespace Wacom
{

class TabletFinderPrivate;

/**
 * Uses the underlying window system and other sources to detect tablets.
 * This class needs the help of an event notifier which signals adding
 * and removal of tablet devices.
 */
class TabletFinder : public QObject
{
    Q_OBJECT

public:
    ~TabletFinder() override;

    static TabletFinder &instance();

    /**
     * Scan for devices and emit a signal for each tablet found.
     */
    bool scan();

public Q_SLOTS:

    /**
     * This slot has to be connected to the event notifier.
     */
    void onX11TabletAdded(int deviceId);

    /**
     * This slot has to be connected to the event notifier.
     */
    void onX11TabletRemoved(int deviceId);

Q_SIGNALS:

    /**
     * Emitted when a tablet is add.
     */
    void tabletAdded(TabletInformation tabletInformation);

    /**
     * Emitted when a tablet is removed.
     */
    void tabletRemoved(TabletInformation tabletInformation);

protected:
    /**
     * Protected default constructor as this class is a singleton.
     */
    TabletFinder();

    /**
     * Looks up tablet information and button mapping from the device database.
     *
     * @param info The tablet information which will be filled with the information from the database.
     *
     * @return True on success, false on error.
     */
    bool lookupInformation(TabletInformation &info);

private:
    /**
     * Copy constructor which does nothing as this class is a singleton.
     */
    explicit TabletFinder(const TabletFinder &finder) = delete;

    /**
     * Copy operator which does nothing as this class is a singleton.
     */
    TabletFinder &operator=(const TabletFinder &finder) = delete;

    Q_DECLARE_PRIVATE(TabletFinder)
    TabletFinderPrivate *const d_ptr;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
