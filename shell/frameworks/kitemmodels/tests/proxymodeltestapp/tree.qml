 /*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
import QtQuick 2.6
import QtQuick.Layouts 1.4
import QtQuick.Controls as QQC2
import org.kde.kitemmodels 1.0 
import org.kde.qqc2desktopstyle.private 1.0 as StylePrivate


QQC2.ScrollView {
    id: root
    width: 600
    height: 500

    ListView {
        spacing: 0
        clip: true
        add: Transition {
            NumberAnimation {
                property: "opacity"; from: 0; to: 1; duration: 250 }
        }
        addDisplaced: Transition {
            NumberAnimation { properties: "y"; duration: 250 }
        }
        remove: Transition {
            NumberAnimation {
                property: "opacity"; from: 1; to: 0; duration: 250 }
        }
        removeDisplaced: Transition {
            NumberAnimation { properties: "y"; duration: 250 }
        }
        model: KDescendantsProxyModel {
            id: descendantsModel
            expandsByDefault: false
            model: _model
        }

        delegate: QQC2.ItemDelegate {
            id: delegate
            highlighted: false

            contentItem: RowLayout {
            RowLayout {
                    Layout.topMargin: -delegate.topPadding
                    Layout.bottomMargin: -delegate.bottomPadding
                    Repeater {
                        
                        model: kDescendantLevel-1
                        
                        delegate: StylePrivate.StyleItem {
                            Layout.preferredWidth: controlRoot.width
                            Layout.fillHeight: true
                            visible: true
                        // control: controlRoot
                            elementType: "itembranchindicator"
                            properties: {
                                "isItem": false,
                                "hasSibling": kDescendantHasSiblings[modelData]
                            }
                        }
                    }
                    QQC2.Button {
                        id: controlRoot
                        Layout.preferredWidth: background.pixelMetric("treeviewindentation")
                        Layout.fillHeight: true
                        enabled: model.kDescendantExpandable
                        text: model.kDescendantExpanded ? "-" : "+"
                        onClicked: descendantsModel.toggleChildren(index)
                        background: StylePrivate.StyleItem {
                            id: styleitem
                            control: controlRoot
                            hover: controlRoot.hovered
                            elementType: "itembranchindicator"
                            on: model.kDescendantExpanded 
                            properties: {
                                "isItem": true,
                                "hasChildren": model.kDescendantExpandable,
                                "hasSibling": model.kDescendantHasSiblings[model.kDescendantHasSiblings.length - 1]
                            }
                        }
                    }
            }
                
                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.display
                }
            }
        }
    }
}
