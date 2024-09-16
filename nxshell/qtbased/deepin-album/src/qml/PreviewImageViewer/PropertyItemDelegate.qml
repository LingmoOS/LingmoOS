// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Control {
    id: control
    property string title
    property string description
    property int corners: RoundRectangle.NoneCorner
    property string iconName

    property int contrlImplicitWidth:66
    property int contrlIntimplicitHeight:40

    width:66
    signal clicked()
    property Component action: ActionButton {
        visible: control.iconName
        Layout.alignment: Qt.AlignRight
        icon {
            width: 14
            height: 14
            name: control.iconName
        }

        onClicked: control.clicked()
    }
    padding: 5
    contentItem: ColumnLayout {

        Label {
            visible: control.title
            text: control.title
            font: DTK.fontManager.t10
        }
        RowLayout {
//            width:contrlImplicitWidth

            Item{
                Label {
                    id :showlabel
                    width:contrlImplicitWidth
                    visible: control.description
                    Layout.fillWidth: true
                    text: control.description
                    font: DTK.fontManager.t8
                    elide: Text.ElideMiddle

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true


                        AlertToolTip{
                            id:tip
                            parent: parent
                            visible:parent.focus
                            text: control.description
                        }

                        onMouseXChanged:{

                            if(tip.width<control.width+15){
                                tip.visible=false
                            }else{
                                tip.visible=true
                            }
                        }
                        onExited:{
                            tip.visible=false
                        }
                    }
                }
            }

            Loader {
                Layout.rightMargin: 5
                sourceComponent: control.action
            }

        }


    }

    background: RoundRectangle {
        implicitWidth: contrlImplicitWidth
        implicitHeight: contrlIntimplicitHeight
        color:  Qt.rgba(0, 0, 0, 0.05)
        radius: Style.control.radius
        corners: control.corners
    }
}
