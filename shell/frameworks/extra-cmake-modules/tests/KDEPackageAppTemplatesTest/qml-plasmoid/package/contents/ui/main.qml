/***************************************************************************
 *   Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                            *
 *                                                                         *
 * Distributed under the OSI-approved BSD License (the "License");
 * see accompanying file COPYING-CMAKE-SCRIPTS for details.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 ***************************************************************************/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.components 2.0 as LingmoComponents

Item {
    Plasmoid.fullRepresentation: ColumnLayout {
        anchors.fill: parent
        LingmoUI.Icon {
            source: "kde"
        }
        LingmoComponents.Label {
            text: "This is Lingmo!"
        }
    }
}
