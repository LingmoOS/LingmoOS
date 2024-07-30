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

#ifndef X11EVENTNOTIFIER_H
#define X11EVENTNOTIFIER_H

#include "eventnotifier.h"
#include <QAbstractNativeEventFilter>
#include <xcb/xcb.h>

namespace Wacom
{

class X11EventNotifierPrivate;

/**
 * @brief Singleton that listens to X11 events, mainly tablet plug/unplug events
 */
class X11EventNotifier : public EventNotifier, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    ~X11EventNotifier() override;

    /**
     * Returns an instance of this class.
     */
    static X11EventNotifier &instance();

    /**
     * @see EventNotifier::start()
     */
    void start() final override;

    /**
     * @see EventNotifier::stop()
     */
    void stop() final override;

protected:
    /**
     * Called by Qt when a new X11 event is detected.
     */
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    X11EventNotifier();
    explicit X11EventNotifier(const X11EventNotifier &notifier) = delete;
    X11EventNotifier &operator=(const X11EventNotifier &notifier) = delete;

    /**
     * Handles X11 input events which signal adding or removal of a device.
     * This method should not be called directly, but only by our X11 event
     * handler method.
     */
    void handleX11InputEvent(xcb_ge_generic_event_t *event);

    /**
     * Register the eventhandler with the X11 system
     */
    int registerForNewDeviceEvent(xcb_connection_t *display);

    Q_DECLARE_PRIVATE(X11EventNotifier)
    X11EventNotifierPrivate *const d_ptr;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
