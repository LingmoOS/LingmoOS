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

#include <QCoreApplication>

#include "private/qtx11extras_p.h"

#include "logging.h"
#include "x11eventnotifier.h"

#include "x11input.h"
#include "x11inputdevice.h"

#include <xcb/xinput.h>

namespace Wacom
{
class X11EventNotifierPrivate
{
public:
    bool isStarted = false;
};
}

using namespace Wacom;

X11EventNotifier::X11EventNotifier()
    : EventNotifier(nullptr)
    , QAbstractNativeEventFilter()
    , d_ptr(new X11EventNotifierPrivate)
{
}

X11EventNotifier::~X11EventNotifier()
{
    delete d_ptr;
}

X11EventNotifier &X11EventNotifier::instance()
{
    static X11EventNotifier instance;
    return instance;
}

void X11EventNotifier::start()
{
    Q_D(X11EventNotifier);

    if (d->isStarted) {
        return;
    }

    if (QCoreApplication::instance() != nullptr) {
        registerForNewDeviceEvent(QX11Info::connection());
        QCoreApplication::instance()->installNativeEventFilter(this);
        d->isStarted = true;
    }
}

void X11EventNotifier::stop()
{
    Q_D(X11EventNotifier);

    if (QCoreApplication::instance() != nullptr) {
        QCoreApplication::instance()->removeNativeEventFilter(this);
        d->isStarted = false;
    }
}

bool X11EventNotifier::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
    xcb_ge_generic_event_t *cookie = reinterpret_cast<xcb_ge_generic_event_t *>(message);

    if (event->response_type == XCB_GE_GENERIC && cookie->event_type == XCB_INPUT_HIERARCHY) {
        handleX11InputEvent(cookie);
    }

    // return QWidget::x11Event(event);
    return false;
}

void X11EventNotifier::handleX11InputEvent(xcb_ge_generic_event_t *event)
{
    xcb_input_hierarchy_event_t *hev = (xcb_input_hierarchy_event_t *)event;

    xcb_input_hierarchy_info_iterator_t iter;
    iter.data = reinterpret_cast<xcb_input_hierarchy_info_t *>(hev + 1);
    iter.rem = hev->num_infos;
    iter.index = reinterpret_cast<char *>(iter.data) - reinterpret_cast<char *>(hev);

    for (; iter.rem; xcb_input_hierarchy_info_next(&iter)) {
        if (iter.data->flags & XCB_INPUT_HIERARCHY_MASK_SLAVE_REMOVED) {
            qCDebug(KDED) << QString::fromLatin1("X11 device with id '%1' removed.").arg(iter.data->deviceid);
            emit tabletRemoved(iter.data->deviceid);

        } else if (iter.data->flags & XCB_INPUT_HIERARCHY_MASK_SLAVE_ADDED) {
            qCDebug(KDED) << QString::fromLatin1("X11 device with id '%1' added.").arg(iter.data->deviceid);

            X11InputDevice device(iter.data->deviceid, QLatin1String("Unknown X11 Device"));

            if (device.isOpen() && device.isTabletDevice()) {
                qCDebug(KDED) << QString::fromLatin1("Wacom tablet device with X11 id '%1' added.").arg(iter.data->deviceid);
                emit tabletAdded(iter.data->deviceid);
            }
        }
    }
}

int X11EventNotifier::registerForNewDeviceEvent(xcb_connection_t *conn)
{
    char buf[sizeof(xcb_input_event_mask_t) + sizeof(uint32_t)];

    xcb_input_event_mask_t *evmask = reinterpret_cast<xcb_input_event_mask_t *>(buf);
    evmask->deviceid = 0;
    evmask->mask_len = 1;

    uint32_t *mask_buf = xcb_input_event_mask_mask(evmask);
    mask_buf[0] = XCB_INPUT_XI_EVENT_MASK_HIERARCHY;

    xcb_input_xi_select_events(conn, QX11Info::appRootWindow(), 1, evmask);
    return 0;
}

#include "moc_x11eventnotifier.cpp"
