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

    currentIndex: icons.currentIndex

    PlasmaCore.Dialog {
        location: PlasmaCore.Types.Floating
        visible: tabBox.visible
        flags: Qt.X11BypassWindowManagerHint
        x: tabBox.screenGeometry.x + tabBox.screenGeometry.width * 0.5 - dialogMainItem.width * 0.5
        y: tabBox.screenGeometry.y + tabBox.screenGeometry.height * 0.5 - dialogMainItem.height * 0.5

        mainItem: ColumnLayout {
            id: dialogMainItem
            spacing: Kirigami.Units.smallSpacing * 2

            width: Math.min(Math.max(tabBox.screenGeometry.width * 0.3, icons.implicitWidth), tabBox.screenGeometry.width * 0.9)

            ListView {
                id: icons

                readonly property int iconSize: Kirigami.Units.iconSizes.enormous
                readonly property int delegateWidth: iconSize + (highlightItem ? highlightItem.margins.left + highlightItem.margins.right : 0)
                readonly property int delegateHeight: iconSize + (highlightItem ? highlightItem.margins.top + highlightItem.margins.bottom : 0)

                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: tabBox.screenGeometry.width * 0.9

                implicitWidth: contentWidth || delegateWidth * 4
                implicitHeight: delegateHeight

                focus: true
                orientation: ListView.Horizontal

                model: tabBox.model
                delegate: Kirigami.Icon {
                    property string caption: model.caption

                    width: icons.delegateWidth
                    height: icons.delegateHeight

                    source: model.icon
                    active: index == icons.currentIndex

                    TapHandler {
                        onSingleTapped: {
                            if (index === icons.currentIndex) {
                                icons.model.activate(index);
                                return;
                            }
                            icons.currentIndex = index;
                        }
                        onDoubleTapped: icons.model.activate(index)
                    }
                }

                highlight: KSvg.FrameSvgItem {
                    id: highlightItem
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                    width: icons.iconSize + margins.left + margins.right
                    height: icons.iconSize + margins.top + margins.bottom
                }

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: Math.round(captionLabel.height / 2)
                    width: parent.width - Kirigami.Units.largeSpacing * 2
                    icon.source: "edit-none"
                    text: i18ndc("kwin", "@info:placeholder no entries in the task switcher", "No open windows")
                    visible: icons.count === 0
                }

                highlightMoveDuration: 0
                highlightResizeDuration: 0
                boundsBehavior: Flickable.StopAtBounds
            }

            PlasmaComponents3.Label {
                id: captionLabel
                text: icons.currentItem ? icons.currentItem.caption : ""
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
                font.weight: Font.Bold
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
            }

            Connections {
                target: tabBox
                function onCurrentIndexChanged() {
                    icons.currentIndex = tabBox.currentIndex;
                }
            }

            /*
            * Key navigation on outer item for two reasons:
            * @li we have to emit the change signal
            * @li on multiple invocation it does not work on the list view. Focus seems to be lost.
            **/
            Keys.onPressed: event => {
                if (event.key == Qt.Key_Left) {
                    icons.decrementCurrentIndex();
                } else if (event.key == Qt.Key_Right) {
                    icons.incrementCurrentIndex();
                }
            }
        }
    }
}
