import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

LingmoUI.Window {
    id: rootWindow
    visible: true
    width: 900
    height: 600
    title: qsTr("Lingmo Clock")

    // 注册 NotificationDialog 组件
    Component {
        id: notificationDialogComponent
        NotificationDialog { }
    }

     property alias stackView: _stackView

    background.opacity: LingmoUI.Theme.darkMode ? 0.7 : 0.5
    header.height: 40
    contentTopMargin: 0

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    LingmoUI.WindowBlur {
        view: rootWindow
        geometry: Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height)
        windowRadius: rootWindow.windowRadius
        enabled: true
    }   

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideBar {
            id: sideBar
            Layout.fillHeight: true

            onCurrentIndexChanged: {
                switchPageFromIndex(currentIndex)
            }
        }
        

        StackView {
            id: _stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: Qt.resolvedUrl(sideBar.model.get(0).page)
            clip: true

            pushEnter: Transition {}
            pushExit: Transition {}
        }
    }

    function addPage(title,name,page,iconSource,category) {
       sideBar.model.append(
        {
            "title": title,
            "name": name,
            "page": page,
            "iconSource": iconSource,
            "iconColor": iconColor
        } 
    );
    }
    
    function switchPageFromIndex(index) {
        _stackView.pop()
        _stackView.push(Qt.resolvedUrl(sideBar.model.get(index).page))
    }

    function switchPageFromName(pageName) {
        for (var i = 0; i < sideBar.model.count; ++i) {
            if (pageName === sideBar.model.get(i).name) {
                switchPageFromIndex(i)
                sideBar.view.currentIndex = i
            }
        }

        // If the window is minimized, it needs to be displayed again.
        rootWindow.show()
        rootWindow.raise()
    }

    onClosing: {
            //do something
        if (rootWindow.updating) {
            close.accepted = false
            return
        }

        close.accepted = true
        // close.accepted = false;
        logger.lInfoTag("main.qml", "Close clicked!");
        Qt.callLater(Qt.quit);
    }
}