/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

CheckBox {
    id: checkBox
    property int fontSize: 12
    
    text: "hello"
    style: CheckBoxStyle {
        background: Item {}
        indicator: Image {
            source: {
                if (checkBox.enabled){
                    return control.checked ? control.hovered ? "images/checkbox_checked_hover.png" : "images/checkbox_checked_normal.png"
                    : control.hovered ? "images/checkbox_unchecked_hover.png" : "images/checkbox_unchecked_normal.png"
                }
                else{
                    return control.checked ? "images/checkbox-checked-insensitive.png" : "images/checkbox-unchecked-insensitive.png"
                }
            }
        }
        label: Text {
            color: Qt.rgba(1, 1, 1, 0.5)
            text: control.text
            font.pixelSize: control.fontSize
        }        
    }
}
