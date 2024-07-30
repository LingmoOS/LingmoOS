/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 TextArea"

    property string longText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed at volutpat nibh, non elementum nulla. Nunc sem magna, semper sit amet sollicitudin et, vestibulum sed metus. Fusce tempor dolor purus, non posuere urna sodales in. Aenean eu erat ipsum. Fusce egestas pulvinar nisi. Mauris vel enim tincidunt, elementum diam sed, tincidunt nulla. Maecenas tempus vitae ligula et convallis. Nullam justo velit, dignissim a nisl at, blandit posuere leo. Maecenas ac scelerisque odio, eget placerat ipsum. Ut iaculis, tortor et ullamcorper fringilla, mauris neque dapibus arcu, eget suscipit libero libero ut nunc. Sed maximus enim a ligula facilisis, non efficitur dolor blandit. Curabitur venenatis mattis erat ac gravida."

    contentItem: Flow {
        spacing: LingmoUI.Units.gridUnit

        LingmoComponents.TextArea {
            placeholderText: "CHEESE"
            wrapMode: TextEdit.Wrap
            width: 150
            height: 100
        }

        LingmoComponents.TextArea {
            text: root.longText
            wrapMode: TextEdit.Wrap
            width: 150
            height: 100
        }


        LingmoComponents.TextArea {
            text: root.longText
            wrapMode: TextEdit.Wrap
            width: 150
            height: 100
        }
    }
}
