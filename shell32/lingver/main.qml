import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.AboutPC 1.0

LingmoUI.Window {
    id: rootWindow
    width: 500
    height: 340
    minimumWidth: 500
    minimumHeight: 340
    maximumWidth: 500
    maximumHeight: 340
    visible: true
    title: qsTr("About This PC")

    minimizeButtonVisible: false

    flags: Qt.FramelessWindowHint

    background.opacity: LingmoUI.Theme.darkMode ? 0.6 : 0.5

    LingmoUI.WindowBlur {
        view: rootWindow
        geometry: Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height)
        windowRadius: rootWindow.windowRadius
        enabled: rootWindow.compositing
    }

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: PointerHandler.CanTakeOverFromItems
        onActiveChanged: if (active) { rootWindow.helper.startSystemMove(rootWindow) }
    }

    // 添加图片和文字的布局
    Column {
        anchors.centerIn: parent
        spacing: 20

        Image {
            id: logoImage
            source: "qrc:/images/logo.png" // 替换为你的图片路径
            width: 100
            height: 100
            fillMode: Image.PreserveAspectFit
        }

        Text {
            text: qsTr("LingmoOS - Your Personal Assistant")
            font.pixelSize: 16
            color: LingmoUI.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
        }
    }
}