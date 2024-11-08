import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import LingmoUI 1.0 as LingmoUI

Item {
    id: root

    property var heightValue: mainLayout.implicitHeight + LingmoUI.Units.largeSpacing * 2

    width: 450
    height: heightValue

    Connections {
        target: confirmation
        function onFailure() {
            doneButton.enabled = true
            passwordInput.enabled = true
        }
    }

    onHeightValueChanged: {
        rootWindow.height = heightValue
        rootWindow.maximumHeight = heightValue
        rootWindow.minimumHeight = heightValue
        rootWindow.maximumWidth = root.width
        rootWindow.minimumWidth = root.width
    }

    Keys.enabled: true
    Keys.onEscapePressed: {
        confirmation.setConfirmationResult("")
    }

    Rectangle {
        id: _background
        anchors.fill: parent
        radius: LingmoUI.Theme.bigRadius
        color: LingmoUI.Theme.secondBackgroundColor
    }

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: TapHandler.CanTakeOverFromAnything
        onActiveChanged: if (active) { windowHelper.startSystemMove(rootWindow) }
    }

    LingmoUI.WindowHelper {
        id: windowHelper
    }

    LingmoUI.WindowShadow {
        view: rootWindow
        geometry: Qt.rect(root.x, root.y, root.width, root.height)
        radius: _background.radius
    }

    FontMetrics {
        id: fontMetrics
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing

        Image {
            id: icon
            source: "qrc:/svg/emblem-warning.svg"
            sourceSize.width: 64
            sourceSize.height: 64
            smooth: true
            Layout.alignment: Qt.AlignTop
            visible: !iconImage
        }

        Image {
            id: iconImage
            source: "image://icontheme/" + confirmation.iconName
            sourceSize.width: 64
            sourceSize.height: 64
            Layout.alignment: Qt.AlignTop
            visible: state !== Image.Ready
        }

        Item {
            width: LingmoUI.Units.largeSpacing
        }

        ColumnLayout {
            id: column
            spacing: LingmoUI.Units.largeSpacing

            Text {
                text: confirmation.message
                font.bold: false
                Layout.fillWidth: true
                Layout.fillHeight: true
                maximumLineCount: 2
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                color: LingmoUI.Theme.textColor
            }

            TextField {
                id: userTextField
                text: confirmation.identity
                enabled: false
                Layout.fillWidth: true
            }

            TextField {
                id: passwordInput
                placeholderText: qsTr("Password")
                echoMode: TextField.Password
                Layout.fillWidth: true
                selectByMouse: true
                focus: true

                onAccepted: {
                    if (passwordInput.text)
                        confirmation.setConfirmationResult(passwordInput.text)
                }

                Keys.onEscapePressed: {
                    confirmation.setConfirmationResult("")
                }
            }

            RowLayout {
                spacing: LingmoUI.Units.largeSpacing

                Button {
                    text: qsTr("Cancel")
                    Layout.fillWidth: true
                    height: 50
                    onClicked: confirmation.rejected()
                }

                Button {
                    id: doneButton
                    text: qsTr("Done")
                    Layout.fillWidth: true
                    flat: true
                    height: 50
                    onClicked: {
                        doneButton.enabled = false
                        passwordInput.enabled = false
                        confirmation.setConfirmationResult(passwordInput.text)
                    }
                }
            }
        }
    }
}
