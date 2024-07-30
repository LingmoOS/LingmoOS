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

#ifndef EVENTNOTIFIER_H
#define EVENTNOTIFIER_H

#include "screenrotation.h"

#include <QWidget>

namespace Wacom
{

/**
 * Event notifier which registers with the window system and sends
 * events when tablets are connected/removed or the screen is
 * rotated.
 *
 * This interface only exists so we can create a mock object for unit testing.
 */
class EventNotifier : public QWidget
{
    Q_OBJECT

public:
    ~EventNotifier() override;

    //! Registers the notifier with the window system.
    virtual void start() = 0;

    //! Unregisters the notifier from the window system.
    virtual void stop() = 0;

Q_SIGNALS:
    /**
     * Emitted when a new tablet is connected.
     *
     * @param deviceId The tablet identifier gathered from the window system.
     */
    void tabletAdded(int deviceId);

    /**
     * Emitted when a tablet is removed.
     *
     * @param deviceId The device that was removed by identifier.
     */
    void tabletRemoved(int deviceId);

    /**
     * Emitted when the screen is rotated.
     *
     * @param screenRotation The rotation direction.
     */
    void screenRotated(const ScreenRotation &screenRotation);

protected:
    explicit EventNotifier(QWidget *parent = nullptr);

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
