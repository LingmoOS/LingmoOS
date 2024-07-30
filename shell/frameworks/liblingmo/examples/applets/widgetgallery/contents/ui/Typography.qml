/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmoui as LingmoUI


LingmoComponents.Page {
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height
    tools: Row {
        spacing: 5
        LingmoComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        LingmoUI.Heading {
            level: 1
            text: "Typography"
        }
    }

    LingmoExtras.ScrollArea {
        anchors.fill: parent
        Flickable {
            id: flickable
            //contentWidth: column.width
            contentHeight: column.height
            clip: true
            anchors.fill: parent

            Column {
                id: column
                width: parent.width
                anchors {
                    //fill: parent
                    margins: 12
                }
                spacing: 12

                LingmoUI.Heading {
                    level: 1
                    text: "A Title"
                }

                Repeater {
                    model: 5
                    LingmoUI.Heading {
                        level: index + 1
                        text: "Header level " + (index + 1)
                    }
                }

                LingmoUI.Heading {
                    level: 1
                    text: "Paragraphs"
                }

                LingmoComponents.Label {
                    text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet turpis eros, in luctus lectus. Curabitur pulvinar ligula at leo pellentesque non faucibus mauris elementum. Pellentesque convallis porttitor sodales. Maecenas risus erat, viverra blandit vestibulum eu, suscipit in est. Praesent quis mattis eros. Sed ante ante, adipiscing non gravida sed, ultrices ultrices urna. Etiam congue mattis convallis. Maecenas sollicitudin mauris at lorem aliquam in venenatis erat convallis. Fusce eleifend scelerisque porttitor. Praesent metus sapien, hendrerit ac congue eget, feugiat id enim. Morbi venenatis gravida felis, vitae varius nunc dictum a. Etiam accumsan, velit ac tempor convallis, leo nibh consequat purus, sit amet fringilla nisi mi et libero."
                }

                LingmoComponents.Label {
                    text: "Donec tincidunt justo eget nulla eleifend hendrerit. Ut eleifend erat nibh. Nunc sagittis tincidunt quam quis lobortis. Nam suscipit ultrices nulla. Suspendisse ullamcorper eleifend massa eu pharetra. Donec tempor iaculis elit, in suscipit velit tristique eu. Curabitur suscipit, lacus id pharetra dapibus, velit ante consectetur erat, ac dignissim quam arcu vitae diam. Suspendisse laoreet tortor nec dolor venenatis ultricies. Sed nunc erat, placerat non gravida sit amet, ullamcorper eu turpis. Nullam vestibulum lacus sed tellus fermentum mollis id at urna. Sed eleifend lobortis mollis. Donec lacus dolor, varius commodo gravida et, fringilla in justo. Nam gravida lorem in odio viverra elementum. Suspendisse non tellus at justo convallis placerat vel ac tellus. Nulla tristique tristique dui ut vestibulum."
                }


                LingmoUI.Heading {
                    level: 1
                    text: "TBD Components"
                }

                LingmoComponents.Label {
                    id: menuEntry
                    text: "Menu entry"
                }

                LingmoComponents.Label {
                    id: notificationHeader
                    text: "Notification header"
                }

                LingmoComponents.Label {
                    id: notificationContent
                    text: "Menu entry"
                }

                LingmoComponents.Label {
                    id: activeTab
                    text: "Active tab"
                }

                LingmoComponents.Label {
                    id: inactiveTab
                    text: "Intactive tab"
                }

                LingmoComponents.Label {
                    id: activeSelection
                    text: "Active selection"
                }

                LingmoComponents.Label {
                    id: inactiveSelection
                    text: "Inactive selection"
                }

                LingmoComponents.Label {
                    id: listItemPrimary
                    text: "List item: primary text"
                }

                LingmoComponents.Label {
                    id: listItemSub
                    text: "List item: secondary text"
                }

                LingmoComponents.Label {
                    id:  listItemPrimarySelected
                    text: "List item: selected, primary text"
                }

                LingmoComponents.Label {
                    id: listItemPrimarySub
                    text: "List item: selected, secondary text"
                }

                LingmoComponents.Label {
                    id: widgetHeader
                    text: "Widget headers"
                }

                LingmoComponents.Label {
                    id: widgetContent
                    text: "Widget Content"
                }

                LingmoComponents.Label {
                    id: iconTextSelected
                    text: "Icon text, selected"
                }

                LingmoComponents.Label {
                    id: iconTextUnselected
                    text: "Icon text, unselected"
                }

                LingmoComponents.Label {
                    id: groupHeader
                    text: "Group Header"
                }

                LingmoComponents.Label {
                    id: groupContent
                    text: "Group Content"
                }
            }
        }
    }
}

