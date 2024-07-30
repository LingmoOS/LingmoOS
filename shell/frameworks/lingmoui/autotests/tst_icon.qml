/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: testCase
    name: "IconTests"

    width: 400
    height: 400
    visible: true

    when: windowShown

    Component { id: emptyIcon; LingmoUI.Icon { } }
    Component { id: sourceOnlyIcon; LingmoUI.Icon { source: "document-new" } }
    Component { id: sizeOnlyIcon; LingmoUI.Icon { width: 50; height: 50 } }
    Component { id: sizeSourceIcon; LingmoUI.Icon { width: 50; height: 50; source: "document-new" } }
    Component { id: minimalSizeIcon; LingmoUI.Icon { width: 1; height: 1; source: "document-new" } }
    Component {
        id: absolutePathIcon;
        LingmoUI.Icon {
            id: icon
            width: 50;
            height: 50;
            source: Qt.resolvedUrl("stop-icon.svg")
        }
    }
    LingmoUI.ImageColors {
        id: imageColors
    }

    function test_create_data() {
        return [
            { tag: "Empty", component: emptyIcon },
            { tag: "Source Only", component: sourceOnlyIcon },
            { tag: "Size Only", component: sizeOnlyIcon },
            { tag: "Size & Source", component: sizeSourceIcon },
            { tag: "Minimal Size", component: minimalSizeIcon }
        ]
    }

    // Test creation of Icon objects.
    // It should not crash when certain properties are not specified and also
    // should still work when they are.
    function test_create(data) {
        var icon = createTemporaryObject(data.component, testCase)
        verify(icon)
        verify(waitForRendering(icon))
    }

    function test_absolutepath_recoloring() {
        var icon = createTemporaryObject(absolutePathIcon, testCase)
        verify(icon)
        verify(waitForRendering(icon))

        var image = icon.grabToImage(function(result) {
            // Access pixel data of the captured image
            imageColors.source = result.image
            imageColors.update()
        })
        tryCompare(imageColors, "dominant", "#2196f3")
    }
}
