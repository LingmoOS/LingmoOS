/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.draganddrop as DragAndDrop
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.plasmoid

LingmoComponents.Page {
    id: root

    property int _h: 48
    property bool isDragging: false

    padding: LingmoUI.Units.largeSpacing

    contentItem: ColumnLayout {

        spacing: 0

        LingmoUI.Heading {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle

            level: 1
            text: "Drag & Drop"
        }

        RowLayout {
            spacing: 0

            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                id: dragCol

                property int itemHeight: _h*1.5

                spacing: _h/4

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                DragItem {
                    text: "Image and URL"
                    icon.name: "image-png"
                    Layout.preferredHeight: parent.itemHeight
                    DragAndDrop.DragArea {
                        objectName: "imageandurl"
                        anchors { fill: parent; }
                        //delegateImage: "akonadi"
                        mimeData.url: "https://lingmo.kde.org/"
                        onDragStarted: {
                            isDragging = true;
                            print(" drag started for " + objectName);
                            ooo.text = objectName
                        }
                        onDrop: {
                            isDragging = false;
                            print(" item dropped " + objectName);
                            ooo.text = objectName
                        }
                        //Rectangle { anchors.fill: parent; color: "blue"; opacity: 0.4; }
                    }
                }
                DragItem {
                    text: "Delegate Image"
                    icon.name: "image-png"
                    Layout.preferredHeight: parent.itemHeight
                    DragAndDrop.DragArea {
                        objectName: "image"
                        anchors { fill: parent; }
                        //delegateImage: "akonadi"
                        //mimeData.url: "https://www.kde.org/"
                        onDragStarted: {
                            isDragging = true;
                            print(" drag started for " + objectName);
                            ooo.text = objectName
                        }
                        onDrop: {
                            isDragging = false;
                            print(" item dropped " + objectName);
                            ooo.text = objectName
                        }
                        //Rectangle { anchors.fill: parent; color: "green"; opacity: 0.4; }
                    }
                }
                DragItem {
                    text: "HTML"
                    icon.name: "text-html"
                    Layout.preferredHeight: parent.itemHeight
                    DragAndDrop.DragArea {
                        objectName: "html"
                        anchors { fill: parent; }
                        mimeData.html: "<b>One <i> Two <u> Three </b> Four </i>Five </u> "
                        onDragStarted: {
                            isDragging = true;
                            print(" drag started for " + objectName);
                            ooo.text = objectName
                        }
                        onDrop: {
                            isDragging = false;
                            print(" item dropped " + objectName);
                            ooo.text = objectName
                        }
                    }
                }
                DragItem {
                    text: "Color"
                    icon.name: "preferences-color"
                    Layout.preferredHeight: parent.itemHeight
                    DragAndDrop.DragArea {
                        objectName: "color"
                        anchors { fill: parent; }
                        mimeData.color: "orange"
                        onDragStarted: {
                            isDragging = true;
                            print(" drag started for " + objectName);
                            ooo.text = objectName
                        }
                        onDrop: {
                            isDragging = false;
                            print(" item dropped " + objectName);
                            ooo.text = objectName
                            //mimeData.
                        }
                    }
                }
                DragItem {
                    text: "Lots of Stuff"
                    icon.name: "ksplash"
                    Layout.preferredHeight: parent.itemHeight

                    DragAndDrop.DragArea {
                        id: dragArea2
                        objectName: "stuff"
                        anchors.fill: parent

                        mimeData.text: "Clownfish"
                        mimeData.html: "<h2>Swimming in a Sea of Cheese</h2><pre>Primus->perform();</pre><br/>"
                        mimeData.color: "darkred"
                        mimeData.url: "https://www.kde.org/"
                        mimeData.urls: ["https://planet.kde.org", "https://fsfe.org", "https://techbase.kde.org", "https://qt.io"]

                        //Rectangle { anchors.fill: parent; color: "yellow"; opacity: 0.6; }

                        onDragStarted: {
                            isDragging = true;
                            print(" drag started for " + objectName);
                            ooo.text = objectName
                        }
                        onDrop: {
                            isDragging = false;
                            print(" item dropped " + objectName);
                            ooo.text = objectName
                        }
                    }
                }
                LingmoComponents.Label {
                    id: ooo
                }
            }

            DragAndDrop.DropArea {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                LingmoExtras.Highlight {
                    id: dropHightlight
                    anchors.fill: parent
                    opacity: 0

                    PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: 2000; }
                }

                Rectangle { id: clr; anchors.fill: parent; color: "transparent"; opacity: color != "transparent" ? 1 : 0; }

                LingmoComponents.Label {
                    id: ilabel
                    font.pointSize: _h / 2
                    text: "Drop here."
                    opacity: isDragging ? 0.7 : 0
                    anchors.centerIn: parent
                    horizontalAlignment: Text.AlignCenter
                    PropertyAnimation { properties: "opacity"; easing.type: Easing.Linear; duration: 2000; }
                }

                LingmoComponents.Label {
                    id: slabel
                    font.pointSize: _h / 4
                    //text: "Drop here."
                    //opacity: isDragging ? 1 : 0
                    //onTextChanged: print("droparea changed to " + text)
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.left: parent.left
                    //horizontalAlignment: Text.AlignCenter
                }

                onDragEnter: {
        //             slabel.text = "drop item here";
                    dropHightlight.opacity = 1;
                }
                onDragLeave: {
        //             slabel.text = "drop left";
                    dropHightlight.opacity = 0;
                }
                onDrop: {
                    var txt = event.mimeData.html;
                    txt += event.mimeData.text;
                    if (event.mimeData.url != "") {
                        txt += "<br />Url: " + event.mimeData.url;
                    }
                    var i = 0;
                    var u;
                    for (u in event.mimeData.urls) {
                        txt += "<br />  Url " + i + " : " + event.mimeData.urls[i];
                        i++;
                    }
        //             print("COLOR: " + event.mimeData.color);
                    if (event.mimeData.hasColor()) {
                        clr.color = event.mimeData.color;
                    } else {
                        clr.color = "transparent";
                    }
                    slabel.text = txt
                    dropHightlight.opacity = 0.5;
                }
            }
        }
    }
}
