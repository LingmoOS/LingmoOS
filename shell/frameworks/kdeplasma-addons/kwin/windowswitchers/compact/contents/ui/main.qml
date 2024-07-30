/*
 KWin - the KDE window manager
 This file is part of the KDE project.

 SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

 SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kwin 3.0 as KWin

KWin.TabBoxSwitcher {
    id: tabBox
    currentIndex: compactListView.currentIndex

    /**
    * Returns the caption with adjustments for minimized items.
    * @param caption the original caption
    * @param mimized whether the item is minimized
    * @return Caption adjusted for minimized state
    **/
    function itemCaption(caption, minimized) {
        if (minimized) {
            return "(" + caption + ")";
        }
        return caption;
    }

    TextMetrics {
        id: textMetrics
        property string longestCaption: tabBox.model.longestCaption() || placeholderLabel.text
        text: itemCaption(longestCaption, true)
        font.bold: true
    }

    onVisibleChanged: {
        if (visible) {
            // Window captions may have change completely
            textMetrics.longestCaption = tabBox.model.longestCaption() || placeholderLabel.text;
        }
    }
    onModelChanged: {
        textMetrics.longestCaption = tabBox.model.longestCaption() || placeholderLabel.text;
    }

    // For mouse clicking, we delay the activation just enough to have
    // visual feedback of the highlight switching to the selected item
    Timer {
        id: activationTimer
        interval: Kirigami.Units.shortDuration
        onTriggered: {
            tabBox.model.activate(compactListView.currentIndex)
        }
    }

    PlasmaCore.Dialog {
        id: dialog
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: Item {
            id: dialogMainItem

            property int optimalWidth: textMetrics.width + Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.smallSpacing + hoverItem.margins.right + hoverItem.margins.left
            property int optimalHeight: compactListView.rowHeight * (compactListView.count || 1)
            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.2, optimalWidth), tabBox.screenGeometry.width * 0.8)
            height: Math.min(optimalHeight, tabBox.screenGeometry.height * 0.8)
            focus: true

            // just to get the margin sizes
            KSvg.FrameSvgItem {
                id: hoverItem
                imagePath: "widgets/viewitem"
                prefix: "hover"
                visible: false
            }

            ListView {
                id: compactListView

                // the maximum text width + icon item width (32 + 4 margin) + margins for hover item
                property int rowHeight: Math.max(Kirigami.Units.iconSizes.small, textMetrics.height + hoverItem.margins.top + hoverItem.margins.bottom)

                anchors.fill: parent
                clip: true

                model: tabBox.model
                delegate: RowLayout {

                    width: compactListView.width
                    height: compactListView.rowHeight
                    opacity: minimized ? 0.6 : 1.0

                    spacing: 2 * Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        id: iconItem
                        source: model.icon
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.leftMargin: hoverItem.margins.left
                    }
                    PlasmaComponents3.Label {
                        id: captionItem
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignBottom
                        text: itemCaption(caption, minimized)
                        textFormat: Text.PlainText
                        font.weight: index === compactListView.currentIndex ? Font.Bold : Font.Normal
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        Layout.rightMargin: hoverItem.margins.right
                        Layout.topMargin: hoverItem.margins.top
                        Layout.bottomMargin: hoverItem.margins.bottom
                    }
                    TapHandler {
                        onTapped: {
                            compactListView.currentIndex = index;
                            activationTimer.start();
                        }
                    }
                }
                highlight: KSvg.FrameSvgItem {
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                    width: compactListView.width
                }
                highlightMoveDuration: 0
                highlightResizeDuration: 0
                boundsBehavior: Flickable.StopAtBounds
                Connections {
                    target: tabBox
                    function onCurrentIndexChanged() {compactListView.currentIndex = tabBox.currentIndex;}
                }

                RowLayout {
                    visible: compactListView.count === 0
                    anchors.centerIn: parent
                    spacing: 2 * Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source:  "edit-none"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        Layout.leftMargin: hoverItem.margins.left
                    }
                    PlasmaComponents3.Label {
                        id: placeholderLabel
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignBottom
                        text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
                        textFormat: Text.PlainText
                        Layout.fillWidth: true
                        Layout.rightMargin: hoverItem.margins.right
                        Layout.topMargin: hoverItem.margins.top
                        Layout.bottomMargin: hoverItem.margins.bottom
                    }
                }
            }
            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: {
                if (event.key == Qt.Key_Up) {
                    compactListView.decrementCurrentIndex();
                } else if (event.key == Qt.Key_Down) {
                    compactListView.incrementCurrentIndex();
                }
            }
        }
    }
}
