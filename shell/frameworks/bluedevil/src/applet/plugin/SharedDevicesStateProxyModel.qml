/*
 * SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

pragma ComponentBehavior: Bound
pragma Singleton

import org.kde.bluezqt as BluezQt
import org.kde.plasma.private.bluetooth as PlasmaBt

// State of ongoing pending calls is shared because this is a singleton.
PlasmaBt.DevicesStateProxyModel {
    sourceModel: BluezQt.DevicesModel { }
}
