// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: monthImage
    property var paths
    property double displayFlushHelper
    property Item content: null

    color: "white"
    function createImage() {
        if (paths.length === 1)
            content = monthComponent_1pic.createObject(monthImage)
        else if (paths.length === 2)
            content = monthComponent_2pic.createObject(monthImage)
        else if (paths.length === 3)
            content = monthComponent_3pic.createObject(monthImage)
        else if (paths.length === 4)
            content = monthComponent_4pic.createObject(monthImage)
        else if (paths.length === 5)
            content = monthComponent_5pic.createObject(monthImage)
        else if (paths.length === 6)
            content = monthComponent_6pic.createObject(monthImage)
    }

    Component {
        id: monthComponent_1pic

        BorderImageEx {
            width: monthImage.width
            height: monthImage.height
            source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_1_" + paths[0]
            url: paths[0]
        }
    }

    Component {
        id: monthComponent_2pic

        Row {
            spacing: 2
            Repeater {
                model: paths
                BorderImageEx {
                    width: monthImage.width / 2
                    height: monthImage.height
                    source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_1_" + modelData
                    url: modelData
                }
            }
        }
    }

    Component {
        id: monthComponent_3pic

        Row {
            spacing: 2
            BorderImageEx {
                width: monthImage.width / 2
                height: monthImage.height
                source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_1_" + paths[0]
                url: paths[0]
            }

            Column {
                spacing: 2
                Repeater {
                    model: [paths[1], paths[2]]
                    BorderImageEx {
                        width: monthImage.width / 2
                        height: monthImage.height / 2
                        source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_2_" + modelData
                        url: modelData
                    }
                }
            }
        }
    }

    Component {
        id: monthComponent_4pic

        Grid {
            columns: 2
            rows: 2
            spacing: 2
            Repeater {
                model: paths
                BorderImageEx {
                    width: monthImage.width / 2
                    height: monthImage.height / 2
                    source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_2_" + modelData
                    url: modelData
                }
            }
        }
    }

    Component {
        id: monthComponent_5pic

        Column {
            spacing: 2
            BorderImageEx {
                width: monthImage.width
                height: monthImage.height * 0.618
                source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_3_" + paths[0]
                url: paths[0]
            }

            Row {
                spacing: 2
                Repeater {
                    model: [paths[1], paths[2], paths[3], paths[4]]
                    BorderImageEx {
                        width: monthImage.width / 4
                        height: monthImage.height * (1 - 0.618)
                        source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_4_" + modelData
                        url: modelData
                    }
                }
            }
        }
    }

    Component {
        id: monthComponent_6pic

        Column {
            spacing: 2
            BorderImageEx {
                width: monthImage.width
                height: monthImage.height * 0.618
                source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_3_" + paths[0]
                url: paths[0]
            }

            Row {
                spacing: 2
                Repeater {
                    model: [paths[1], paths[2], paths[3], paths[4], paths[5]]
                    BorderImageEx {
                        width: monthImage.width / 5
                        height: monthImage.height * (1 - 0.618)
                        source: "image://collectionPublisher/" + monthImage.displayFlushHelper.toString() + "_M_5_" + modelData
                        url: modelData
                    }
                }
            }
        }
    }
}

