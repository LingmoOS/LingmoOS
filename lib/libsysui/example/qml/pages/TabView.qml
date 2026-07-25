import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import LingmoUI

LingmoScrollablePage {
    property var colors: [LingmoColor.Yellow, LingmoColor.Orange, LingmoColor.Red, LingmoColor.Magenta, LingmoColor.Purple, LingmoColor.Blue, LingmoColor.Teal, LingmoColor.Green]

    title: qsTr("TabView")

    Component{
        id:com_page
        Rectangle{
            anchors.fill: parent
            color: argument.normal
            LingmoText{
                text: qsTr("the current index is: ")+nowIndex
            }
        }
    }

    function newTab() {
        tab_view.appendTab("qrc:/image/logo.svg", qsTr("Document ") + tab_view.count(), com_page, colors[Math.floor(Math.random() * 8)],true);
    }

    Component.onCompleted: {
        newTab()
        newTab()
        newTab()
    }

    LingmoFrame{
        Layout.fillWidth: true
        Layout.preferredHeight: 50
        topPadding: 10
        leftPadding: 10
        RowLayout{
            spacing: 14
            LingmoDropDownBox{
                id:btn_tab_width_behavior
                Layout.preferredWidth: 140
                text:"Equal"
                LingmoMenuItem{
                    text:"Equal"
                    onClicked: {
                        btn_tab_width_behavior.text = text
                        tab_view.tabWidthBehavior = LingmoTabViewType.Equal
                    }
                }
                LingmoMenuItem{
                    text:"SizeToContent"
                    onClicked: {
                        btn_tab_width_behavior.text = text
                        tab_view.tabWidthBehavior = LingmoTabViewType.SizeToContent
                    }
                }
                LingmoMenuItem{
                    text:"Compact"
                    onClicked: {
                        btn_tab_width_behavior.text = text
                        tab_view.tabWidthBehavior = LingmoTabViewType.Compact
                    }
                }
            }
            LingmoDropDownBox{
                id:btn_close_button_visibility
                text:"Always"
                Layout.preferredWidth: 120
                LingmoMenuItem{
                    text:"Never"
                    onClicked: {
                        btn_close_button_visibility.text = text
                        tab_view.closeButtonVisibility = LingmoTabViewType.Never
                    }
                }
                LingmoMenuItem{
                    text:"Always"
                    onClicked: {
                        btn_close_button_visibility.text = text
                        tab_view.closeButtonVisibility = LingmoTabViewType.Always
                    }
                }
                LingmoMenuItem{
                    text:"OnHover"
                    onClicked: {
                        btn_close_button_visibility.text = text
                        tab_view.closeButtonVisibility = LingmoTabViewType.OnHover
                    }
                }
            }
            LingmoButton{
                text: "Change Color and Text"
                onClicked: {
                    tab_view.setCurrentArgument(colors[Math.floor(Math.random() * 8)]);
                    tab_view.setCurrentText("Document "+Math.random());
                }
            }
        }
    }

    LingmoFrame{
        Layout.fillWidth: true
        Layout.topMargin: 15
        Layout.preferredHeight: 400
        LingmoTabView{
            id:tab_view
            onNewPressed:{
                newTab()
            }
        }
    }
}
