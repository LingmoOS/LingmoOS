/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

/**
 * Device setup for Network discovered printer devices,
 * ie. IPP/HTTP.
 *
 * Each printer device has recommended drivers.
 * (system-config-printer)
*/
BaseDevice {
    id: root
    title: settings.value("printer-make-and-model")
    subtitle: settings.value("device-desc")
    showUri: false


    // Recommended Driver list
    contentItem: Drivers {
        width: root.width

        Component.onCompleted: {
            load(settings.value("device-id")
                          , settings.value("printer-make-and-model")
                          , settings.value("device-uri"))
        }
    }
}
