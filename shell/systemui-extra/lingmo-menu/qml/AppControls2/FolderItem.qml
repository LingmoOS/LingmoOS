import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.5
import org.lingmo.menu.core 1.0
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

MouseArea {
    id: control
    property bool editStatus: false
    property bool truncate: false
    property bool isSelect: false
    hoverEnabled: true

    LingmoUIItems.Tooltip {
        anchors.fill: parent
        mainText: name
        posFollowCursor: true
        margin: 6
        visible: !editStatus && truncate
    }

    onPositionChanged: {
        if (tip.isVisible) {
            if (tip.visible) {
                tip.hide();
            } else {
                tip.show(name);
            }
        }
    }

    states: State {
        when: control.activeFocus
        PropertyChanges {
            target: controlBase
            borderColor: Platform.Theme.Highlight
            border.width: 2
        }
    }

    LingmoUIItems.StyleBackground {
        id: controlBase
        anchors.fill: parent
        radius: Platform.Theme.minRadius
        useStyleTransparency: false
        alpha: isSelect ? 0.55 : control.containsPress ? 0.82 : control.containsMouse ? 0.55 : 0.00

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            spacing: 12

            FolderIcon {
                rows: 2; columns: 2
                spacing: 2; padding: 2
                icons: icon
                alpha: 0.12; radius: Platform.Theme.minRadius
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
            }

            Item {
                id: renameArea
                Layout.fillWidth: true
                Layout.fillHeight: true

                Component {
                    id: unEditText
                    LingmoUIItems.StyleText {
                        id: textShow
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                        text: name
                        onWidthChanged: {
                            control.truncate = textShow.truncated
                        }
                    }
                }

                Component {
                    id: editText
                    LingmoUIItems.StyleBackground {
                        radius: Platform.Theme.normalRadius
                        useStyleTransparency: false
                        alpha: textChange.activeFocus ? 0.04 : 0
                        paletteRole: Platform.Theme.Text
                        border.width: 2
                        borderAlpha: textChange.activeFocus ? 1 : 0
                        borderColor: Platform.Theme.Highlight

                        TextInput {
                            id: textChange
                            text: name
                            clip: true
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: buttonMouseArea.left
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            verticalAlignment: Text.AlignVCenter
                            selectByMouse: true
                            maximumLength: 14

                            function editStatusEnd() {
                                control.editStatus = false;
                                control.focus = true;
                            }

                            function updateTextInputColor() {
                                color = Platform.Theme.text();
                                selectionColor = Platform.Theme.highlight();
                            }

                            onEditingFinished: {
                                modelManager.getAppModel().renameFolder(id, text);
                                textChange.editStatusEnd();
                            }

                            Platform.Theme.onPaletteChanged: {
                                updateTextInputColor();
                            }

                            Component.onCompleted: {
                                updateTextInputColor();
                                forceActiveFocus();
                            }
                        }

                        MouseArea {
                            id: buttonMouseArea
                            hoverEnabled: true
                            width: 16; height: width
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            visible: textChange.activeFocus

                            LingmoUIItems.Icon {
                                anchors.centerIn: parent
                                width: 16; height: width
                                source: "edit-clear-symbolic"
                            }

                            onClicked: {
                                textChange.text = name;
                                textChange.editStatusEnd();
                            }
                        }
                    }
                }

                Loader {
                    id: editLoader
                    anchors.fill: parent
                    sourceComponent: editStatus ? editText : unEditText
                }
            }
        }
    }
}
