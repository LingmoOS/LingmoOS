/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import org.kde.newstuff as NewStuff

NewStuff.Dialog {
    id: component
    configFile: knsrcfile
    Component.onCompleted: {
        open();
        if (typeof(knsProviderId) !== "undefined" && typeof(knsEntryId) !== "undefined") {
            showEntryDetails(knsProviderId, knsEntryId);
        }
    }
    onVisibleChanged: {
        if (visible === false) {
            Qt.quit();
        }
    }
}
