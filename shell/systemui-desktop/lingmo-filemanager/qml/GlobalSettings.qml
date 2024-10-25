/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import Qt.labs.settings 1.0

Settings {
    property int viewMethod: 1          // controls display mode: list or grid

    // Port to C++
    // property bool showHiddenFiles: false

    // Name, Date, Size
    property int orderBy: 0
    property int sortMode: 0

    // UI
    property int width: 900
    property int height: 580
    property int desktopIconSize: 72
    property int maximumIconSize: 256
    property int minimumIconSize: 64

    property int gridIconSize: 64
}
