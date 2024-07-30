/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents3
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid

QQC2.StackView {
    id: mainStack
    focus: true

    Layout.minimumWidth: LingmoUI.Units.gridUnit * 12
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 12

    readonly property Item activeApplet: systemTrayState.activeApplet

    /* Heading */
    property bool appletHasHeading: false
    property bool mergeHeadings: appletHasHeading && activeApplet.fullRepresentationItem.header.visible
    property int headingHeight: mergeHeadings ? activeApplet.fullRepresentationItem.header.height : 0
    /* Footer */
    property bool appletHasFooter: false
    property bool mergeFooters: appletHasFooter && activeApplet.fullRepresentationItem.footer.visible
    property int footerHeight: mergeFooters ? activeApplet.fullRepresentationItem.footer.height : 0

    onActiveAppletChanged: {
        mainStack.appletHasHeading = false
        mainStack.appletHasFooter = false

        if (activeApplet != null && activeApplet.fullRepresentationItem && !activeApplet.preferredRepresentation) {
            //reset any potential anchor
            activeApplet.fullRepresentationItem.anchors.left = undefined;
            activeApplet.fullRepresentationItem.anchors.top = undefined;
            activeApplet.fullRepresentationItem.anchors.right = undefined;
            activeApplet.fullRepresentationItem.anchors.bottom = undefined;
            activeApplet.fullRepresentationItem.anchors.centerIn = undefined;
            activeApplet.fullRepresentationItem.anchors.fill = undefined;

            if (activeApplet.fullRepresentationItem instanceof LingmoComponents3.Page ||
                activeApplet.fullRepresentationItem instanceof LingmoExtras.Representation) {
                if (activeApplet.fullRepresentationItem.header && activeApplet.fullRepresentationItem.header instanceof LingmoExtras.PlasmoidHeading) {
                    mainStack.appletHasHeading = true
                    activeApplet.fullRepresentationItem.header.background.visible = false
                }
                if (activeApplet.fullRepresentationItem.footer && activeApplet.fullRepresentationItem.footer instanceof LingmoExtras.PlasmoidHeading) {
                    mainStack.appletHasFooter = true
                    activeApplet.fullRepresentationItem.footer.background.visible = false
                }
            }

            let unFlipped = systemTrayState.oldVisualIndex < systemTrayState.newVisualIndex;
            if (Qt.application.layoutDirection !== Qt.LeftToRight) {
                unFlipped = !unFlipped;
            }

            const isTransitionEnabled = systemTrayState.expanded;
            (mainStack.empty ? mainStack.push : mainStack.replace)(activeApplet.fullRepresentationItem, {
                "width": Qt.binding(() => mainStack.width),
                "height": Qt.binding(() => mainStack.height),
                "x": 0,
                "focus": Qt.binding(() => !mainStack.busy), // QTBUG-44043: retrigger binding after StackView is ready to restore focus
                "opacity": 1,
                "KeyNavigation.up": mainStack.KeyNavigation.up,
                "KeyNavigation.backtab": mainStack.KeyNavigation.backtab,
            }, isTransitionEnabled ? (unFlipped ? QQC2.StackView.PushTransition : QQC2.StackView.PopTransition) : QQC2.StackView.Immediate);
        } else {
            mainStack.clear();
        }
    }

    onCurrentItemChanged: {
        if (currentItem !== null && root.expanded) {
            currentItem.forceActiveFocus();
        }
    }

    Connections {
        target: Plasmoid
        function onAppletRemoved(applet) {
            if (applet === systemTrayState.activeApplet) {
                mainStack.clear();
            }
        }
    }
}
