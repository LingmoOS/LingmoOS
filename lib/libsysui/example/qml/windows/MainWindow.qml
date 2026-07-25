import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LingmoUI
import "../global"

LingmoWindow {
    id: window
    visible: true
    width: 1000
    height: 668
    minimumWidth: 668
    minimumHeight: 320
    title: qsTr("LingmoUI Example")
    fitsAppBarWindows: true
    showDark: true

    Item {
        id: content_item
        anchors.fill: parent

        SidebarItems {
            id: sidebar_items
        }

        Component{
            id: nav_item_right_menu
            LingmoMenu{
                width: 186
                LingmoMenuItem{
                    text: qsTr("Open in Separate Window")
                    font: LingmoTextStyle.Caption
                    onClicked: {
                        LingmoRouter.navigate("/pageWindow",{title:modelData.title,url:modelData.url})
                    }
                }
            }
        }

        Item{
            id:page_front
            anchors.fill: content_item
            LingmoNavigationView{
                property int clickCount: 0
                id:nav_view
                width: parent.width
                height: parent.height
                z:999
                //Stack模式，每次切换都会将页面压入栈中，随着栈的页面增多，消耗的内存也越多，内存消耗多就会卡顿，这时候就需要按返回将页面pop掉，释放内存。该模式可以配合LingmoPage中的launchMode属性，设置页面的启动模式
                //                pageMode: LingmoNavigationViewType.Stack
                //NoStack模式，每次切换都会销毁之前的页面然后创建一个新的页面，只需消耗少量内存
                pageMode: LingmoNavigationViewType.NoStack
                items: sidebar_items
                topPadding:{
                    if(window.useSystemAppBar){
                        return 0
                    }
                    return LingmoTools.isMacos() ? 20 : 0
                }
                displayMode: LingmoNavigationViewType.Auto
                logo: "qrc:/image/logo.svg"
                title:"LingmoUI"

                Component.onCompleted: {
                    sidebar_items.navigationView = nav_view
                    sidebar_items.panelItemMenu = nav_item_right_menu
                    window.setHitTestVisible(nav_view.buttonMenu)
                    window.setHitTestVisible(nav_view.buttonBack)
                    window.setHitTestVisible(nav_view.imageLogo)
                    setCurrentIndex(0)
                }
            }
        }
    }

}
