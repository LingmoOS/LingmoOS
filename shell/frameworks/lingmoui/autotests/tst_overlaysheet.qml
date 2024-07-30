/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: root

    name: "OverlaySheetTest"
    visible: true
    when: windowShown

    width: 500
    height: 500

    Component {
        id: emptyComponent
        LingmoUI.OverlaySheet {}
    }

    Component {
        id: itemContentComponent
        LingmoUI.OverlaySheet {
            Rectangle {
                implicitWidth: 100
                implicitHeight: 200
            }
        }
    }

    Component {
        id: viewContentWithImplicitSizeComponent
        LingmoUI.OverlaySheet {
            ListView {
                implicitWidth: 100
                implicitHeight: 200
            }
        }
    }

    Component {
        id: viewContentWithPreferredSizeComponent
        LingmoUI.OverlaySheet {
            ListView {
                Layout.preferredWidth: 100
                Layout.preferredHeight: 200
            }
        }
    }

    Component {
        id: itemContentWithMixedSizesComponent
        LingmoUI.OverlaySheet {
            Rectangle {
                implicitWidth: 100
                implicitHeight: 200

                Layout.preferredWidth: 300
                Layout.preferredHeight: 400
            }
        }
    }

    Component {
        id: viewContentWithMixedSizesComponent
        LingmoUI.OverlaySheet {
            ListView {
                implicitWidth: 100
                implicitHeight: 200

                Layout.preferredWidth: 300
                Layout.preferredHeight: 400
            }
        }
    }

    function setupWarningsFilter() {
        // no nullable property access errors are allowed
        failOnWarning(/\bnull\b/);
    }

    // Note about implicit size comparison: we don't know how much space
    // header+footer+paddings would take, but the sum should be no less than
    // the implicit or preferred size given. And the difference between
    // implicit and preferred size is big enough (200px) that it is unlikely
    // to get an overlap.

    function test_init() {
        setupWarningsFilter();
        {
            const sheet = createTemporaryObject(emptyComponent, null);
            verify(sheet);
            verify(!sheet.parent);
        }
        {
            const sheet = createTemporaryObject(emptyComponent, this);
            verify(sheet);
            compare(sheet.parent, this);
        }
        {
            const sheet = createTemporaryObject(viewContentWithImplicitSizeComponent, this);
            verify(sheet);
            verify(sheet.implicitWidth >= 100);
            verify(sheet.implicitHeight >= 200);
        }
        {
            const sheet = createTemporaryObject(viewContentWithPreferredSizeComponent, this);
            verify(sheet);
            verify(sheet.implicitWidth >= 100);
            verify(sheet.implicitHeight >= 200);
        }
    }

    // Layout.preferred* takes precedence over implicit size
    function test_sizeHintPrecedence() {
        setupWarningsFilter();
        {
            // first, make sure 100x200 sheet's total implicit size is smaller than 300x400
            const sheet = createTemporaryObject(itemContentComponent, this);
            verify(sheet);
            verify(sheet.implicitWidth >= 100 && sheet.implicitWidth < 300);
            verify(sheet.implicitHeight >= 200 && sheet.implicitHeight < 400);
        }
        {
            const sheet = createTemporaryObject(itemContentWithMixedSizesComponent, this);
            verify(sheet);
            verify(sheet.implicitWidth >= 300);
            verify(sheet.implicitHeight >= 400);
        }
        {
            const sheet = createTemporaryObject(viewContentWithMixedSizesComponent, this);
            verify(sheet);
            verify(sheet.implicitWidth >= 300);
            verify(sheet.implicitHeight >= 400);
        }
    }
}
