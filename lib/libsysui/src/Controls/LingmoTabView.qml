import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LingmoUI

Item {
    property int tabWidthBehavior : LingmoTabViewType.Equal
    property int closeButtonVisibility : LingmoTabViewType.Always
    property int itemWidth: 146
    property bool addButtonVisibility: true
    property alias currentIndex: tab_nav.currentIndex
    property var currentItem: {
        return tab_model.get(currentIndex)
    }
    signal newPressed
    id:control
    implicitHeight: height
    implicitWidth: width
    anchors.fill: {
        if(parent)
            return parent
        return undefined
    }
    QtObject {
        id: d
        property int dragIndex: -1
        property bool dragBehavior: false
        property bool itemPress: false
        property int maxEqualWidth: 240
    }
    MouseArea{
        anchors.fill: parent
        preventStealing: true
    }
    ListModel{
        id:tab_model
    }
    LingmoIconButton{
        id:btn_new
        visible: addButtonVisibility
        width: 34
        height: 34
        x:Math.min(tab_nav.contentWidth,tab_nav.width)
        anchors.top: parent.top
        iconSource: LingmoIcons.Add
        onClicked: {
            newPressed()
        }
    }
    ListView{
        id:tab_nav
        height: 34
        orientation: ListView.Horizontal
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
            rightMargin: 34
        }
        interactive: false
        model: tab_model
        move: Transition {
            NumberAnimation { properties: "x"; duration: 100; easing.type: Easing.OutCubic }
            NumberAnimation { properties: "y"; duration: 100; easing.type: Easing.OutCubic }
        }
        moveDisplaced: Transition {
            NumberAnimation { properties: "x"; duration: 300; easing.type: Easing.OutCubic}
            NumberAnimation { properties: "y"; duration: 100;  easing.type: Easing.OutCubic }
        }
        clip: true
        ScrollBar.horizontal: ScrollBar{
            id: scroll_nav
            policy: ScrollBar.AlwaysOff
        }
        delegate:  Item{
            width: item_layout.width
            height: item_container.height
            z: item_mouse_drag.pressed ? 1000 : 1
            Item{
                id:item_layout
                width: item_container.width
                height: item_container.height
                Item{
                    id:item_container
                    property real timestamp: new Date().getTime()
                    height: tab_nav.height
                    width: {
                        if(tabWidthBehavior === LingmoTabViewType.Equal){
                            return Math.max(Math.min(d.maxEqualWidth,tab_nav.width/tab_nav.count),41 + item_btn_close.width)
                        }
                        if(tabWidthBehavior === LingmoTabViewType.SizeToContent){
                            return itemWidth
                        }
                        if(tabWidthBehavior === LingmoTabViewType.Compact){
                            return item_mouse_hove.containsMouse || item_btn_close.hovered || tab_nav.currentIndex === index  ? itemWidth : 41 + item_btn_close.width
                        }
                        return Math.max(Math.min(d.maxEqualWidth,tab_nav.width/tab_nav.count),41 + item_btn_close.width)
                    }
                    Behavior on x { enabled: d.dragBehavior; NumberAnimation { duration: 200 } }
                    Behavior on y { enabled: d.dragBehavior; NumberAnimation { duration: 200 } }
                    MouseArea{
                        id:item_mouse_hove
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                    LingmoTooltip{
                        visible: item_mouse_hove.containsMouse
                        text:item_text.text
                        delay: 1000
                    }
                    MouseArea{
                        id:item_mouse_drag
                        anchors.fill: parent
                        drag.target: item_container
                        drag.axis: Drag.XAxis
                        onWheel: (wheel)=>{
                                     if (wheel.angleDelta.y > 0) scroll_nav.decrease()
                                     else scroll_nav.increase()
                                 }
                        onPressed: {
                            d.itemPress = true
                            item_container.timestamp = new Date().getTime();
                            d.dragBehavior = false;
                            var pos = tab_nav.mapFromItem(item_container, 0, 0)
                            d.dragIndex = model.index
                            item_container.parent = tab_nav
                            item_container.x = pos.x
                            item_container.y = pos.y
                        }
                        onReleased: {
                            d.itemPress = false
                            timer.stop()
                            var timeDiff = new Date().getTime() - item_container.timestamp
                            if (timeDiff < 300) {
                                tab_nav.currentIndex = index
                            }
                            d.dragIndex = -1;
                            var pos = tab_nav.mapToItem(item_layout, item_container.x, item_container.y)
                            item_container.parent = item_layout;
                            item_container.x = pos.x;
                            item_container.y = pos.y;
                            d.dragBehavior = true;
                            item_container.x = 0;
                            item_container.y = 0;
                        }
                        onPositionChanged: {
                            var pos = tab_nav.mapFromItem(item_container, 0, 0)
                            updatePosition(pos)
                            if(pos.x<0){
                                timer.isIncrease = false
                                timer.restart()
                            }else if(pos.x>tab_nav.width-itemWidth){
                                timer.isIncrease = true
                                timer.restart()
                            }else{
                                timer.stop()
                            }
                        }
                        Timer{
                            id:timer
                            property bool isIncrease: true
                            interval: 10
                            repeat: true
                            onTriggered: {
                                if(isIncrease){
                                    if(tab_nav.contentX>=tab_nav.contentWidth-tab_nav.width){
                                        return
                                    }
                                    tab_nav.contentX = tab_nav.contentX+2
                                }else{
                                    if(tab_nav.contentX<=0){
                                        return
                                    }
                                    tab_nav.contentX = tab_nav.contentX-2
                                }
                                item_mouse_drag.updatePosition(tab_nav.mapFromItem(item_container, 0, 0))
                            }
                        }
                        function updatePosition(pos){
                            var idx = tab_nav.indexAt(pos.x+tab_nav.contentX+1, pos.y)
                            if(idx<0){
                                return
                            }
                            if(idx>=tab_nav.count){
                                return
                            }
                            if (d.dragIndex !== idx) {
                                tab_model.move(d.dragIndex, idx, 1)
                                d.dragIndex = idx;
                            }
                        }
                    }
                    LingmoRectangle{
                        anchors.fill: parent
                        radius: [6,6,0,0]
                        color: {
                            if(item_mouse_hove.containsMouse || item_btn_close.hovered){
                                return LingmoTheme.itemHoverColor
                            }
                            if(tab_nav.currentIndex === index){
                                return LingmoTheme.itemCheckColor
                            }
                            return LingmoTheme.itemNormalColor
                        }
                    }
                    RowLayout{
                        spacing: 0
                        height: parent.height
                        Image{
                            source:model.tab_icon
                            Layout.leftMargin: 10
                            Layout.preferredWidth: 14
                            Layout.preferredHeight: 14
                            Layout.alignment: Qt.AlignVCenter
                        }
                        LingmoText{
                            id:item_text
                            text: model.text
                            Layout.leftMargin: 10
                            visible: {
                                if(tabWidthBehavior === LingmoTabViewType.Equal){
                                    return true
                                }
                                if(tabWidthBehavior === LingmoTabViewType.SizeToContent){
                                    return true
                                }
                                if(tabWidthBehavior === LingmoTabViewType.Compact){
                                    return item_mouse_hove.containsMouse || item_btn_close.hovered || tab_nav.currentIndex === index
                                }
                                return false
                            }
                            Layout.preferredWidth: visible?item_container.width - 41 - item_btn_close.width:0
                            elide: Text.ElideRight
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                    LingmoIconButton{
                        id:item_btn_close
                        iconSource: LingmoIcons.ChromeClose
                        iconSize: 10
                        width: visible ? 24 : 0
                        height: 24
                        visible: {
                            if(closeButtonVisibility === LingmoTabViewType.Never)
                                return false
                            if(closeButtonVisibility === LingmoTabViewType.OnHover)
                                return item_mouse_hove.containsMouse || item_btn_close.hovered
                            return true
                        }
                        anchors{
                            right: parent.right
                            rightMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        onClicked: {
                            tab_model.remove(index)
                        }
                    }
                    LingmoDivider{
                        width: 1
                        height: 16
                        orientation: Qt.Vertical
                        anchors{
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                        }
                    }
                }
            }
        }
    }
    Item{
        id:container
        anchors{
            top: tab_nav.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        Repeater{
            model:tab_model
            LingmoLoader{
                property var argument: model.argument
                /**
                * Return the index of the current page of this LingmoTabView.
                * @since 3.1.0
                */
                readonly property int nowIndex: index
                anchors.fill: parent
                sourceComponent: model.page
                visible: tab_nav.currentIndex === index 
            }
        }
    }
    function createTab(tab_icon,text,page,argument={}){
        return {tab_icon:tab_icon,text:text,page:page,argument:argument} //icon->tab_icon: avoid to be same as WebEngineView's.
    }
    function appendTab(tab_icon,text,page,argument,autoSwitch){
        tab_model.append(createTab(tab_icon,text,page,argument))
        if(autoSwitch){
            switchToTab()
        }
    }
    function setTabList(list){
        tab_model.clear()
        tab_model.append(list)
    }
    function count(){
        return tab_nav.count
    }
    /**
    * Close a tab by the index. if the index is not given, it defaultly closes the last tab.
    * @param {int} index - the index of the tab to be closed. 
    * @since 3.0.0
    */
    function closeTab(index=tab_nav.count-1){
        tab_model.remove(index)
    }
    /**
    * Switch to a tab by the index. if the index is not given, it defaultly switches to the last tab.
    * @param {int} index - the index of the tab to be switched to. 
    * @since 3.0.0
    */
    function switchToTab(index=tab_nav.count-1){
        tab_nav.currentIndex = index
    }
    /**
    * Set the icon of the current tab.
    * @param {QUrl} value - the url of the icon.
    * @since 3.0.0
    */
    function setCurrentTabIcon(value){
        currentItem.tab_icon=value
    }
    /**
    * Set the text of the current tab.
    * @param {string} value - the content of the text.
    * @since 3.0.0
    */
    function setCurrentText(value){
        currentItem.text=value
    }
    /**
    * Set the arugument of the current tab.
    * @param {object} value - the new value of arugment of current ListElement.
    * @since 3.0.0
    */
    function setCurrentArgument(value){
        currentItem.argument=value
    }
    /*
    * Get the element data by index from the tab model.
    * @param {int} index - the index.
    * @since 3.1.2
    */
    function get(index){
        return tab_model.get(index)
    }
}
