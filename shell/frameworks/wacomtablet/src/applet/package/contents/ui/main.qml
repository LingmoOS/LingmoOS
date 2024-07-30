// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2015 Weng Xuetian <wengxt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasma5support as P5Support
import org.kde.ksvg as KSvg
import org.kde.kcmutils as KCMUtils
import org.kde.config as KConfig

PlasmoidItem {
    id: root

    function action_wacomtabletkcm() {
        var service = dataSource.serviceForSource("wacomtablet");
        var operation = service.operationDescription("RunKCM");
        service.startOperationCall(operation);
    }

    PlasmaCore.Action {
        id: configureAction
        text: i18n("&Configure Graphics Tablet...")
        icon.name: "configure"
        visible: KConfig.KAuthorized.authorizeControlModule("kcm_wacomtablet");
        onTriggered: KCMUtils.KCMLauncher.openSystemSettings("kcm_wacomtablet");
    }

    Component.onCompleted: {
        Plasmoid.setInternalAction("configure", configureAction)
    }

    property bool active: dataSource.data["wacomtablet"]["serviceAvailable"] && dataModel.count != 0

    toolTipMainText: i18n("Wacom Tablet")
    Plasmoid.status: active ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    Plasmoid.icon: "input-tablet"
    KSvg.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }

    P5Support.DataSource {
        id: dataSource
        engine: "wacomtablet"
        connectedSources: dataSource.sources
    }
    P5Support.DataModel {
        id: dataModel
        dataSource: dataSource
        sourceFilter: "Tablet.*"
    }

    fullRepresentation: FullRepresentation { }
}
