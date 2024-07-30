/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: root

    name: "PaddingTest"
    visible: true
    when: windowShown

    width: 300
    height: 300

    Component {
        id: spyComponent
        SignalSpy {}
    }

    Component {
        id: emptyComponent
        LingmoUI.Padding {}
    }

    function test_empty() {
        const item = createTemporaryObject(emptyComponent, this);
        verify(item);
        verify(!item.contentItem);

        compare(item.implicitWidth, 0);
        compare(item.implicitHeight, 0);
        compare(item.width, 0);
        compare(item.height, 0);
        compare(item.implicitContentWidth, 0);
        compare(item.implicitContentHeight, 0);

        compare(item.padding, 0);
        compare(item.topPadding, 0);
        compare(item.leftPadding, 0);
        compare(item.rightPadding, 0);
        compare(item.bottomPadding, 0);
        compare(item.verticalPadding, 0);
        compare(item.horizontalPadding, 0);
        compare(item.baselineOffset, 0);

        item.topPadding = 1;
        item.leftPadding = 2;
        item.rightPadding = 30;
        item.bottomPadding = 40;
        verify(waitForRendering(item))

        compare(item.implicitWidth, 32);
        compare(item.implicitHeight, 41);
    }

    function test_paddingInheritance() {
        const item = createTemporaryObject(emptyComponent, this);
        verify(item);

        const paddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "paddingChanged" });
        verify(paddingSpy);
        const topPaddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "topPaddingChanged" });
        verify(topPaddingSpy);
        const leftPaddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "leftPaddingChanged" });
        verify(leftPaddingSpy);
        const rightPaddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "rightPaddingChanged" });
        verify(rightPaddingSpy);
        const bottomPaddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "bottomPaddingChanged" });
        verify(bottomPaddingSpy);
        const verticalPaddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "verticalPaddingChanged" });
        verify(verticalPaddingSpy);
        const horizontalPaddingSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "horizontalPaddingChanged" });
        verify(horizontalPaddingSpy);

        compare(item.padding, 0);
        compare(item.topPadding, 0);
        compare(item.leftPadding, 0);
        compare(item.rightPadding, 0);
        compare(item.bottomPadding, 0);
        compare(item.verticalPadding, 0);
        compare(item.horizontalPadding, 0);
        compare(item.baselineOffset, 0);

        item.padding = 1;
        compare(paddingSpy.count, 1), paddingSpy.clear();
        compare(topPaddingSpy.count, 1), topPaddingSpy.clear();
        compare(leftPaddingSpy.count, 1), leftPaddingSpy.clear();
        compare(rightPaddingSpy.count, 1), rightPaddingSpy.clear();
        compare(bottomPaddingSpy.count, 1), bottomPaddingSpy.clear();
        compare(verticalPaddingSpy.count, 1), verticalPaddingSpy.clear();
        compare(horizontalPaddingSpy.count, 1), horizontalPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 1);
        compare(item.leftPadding, 1);
        compare(item.rightPadding, 1);
        compare(item.bottomPadding, 1);
        compare(item.verticalPadding, 1);
        compare(item.horizontalPadding, 1);
        compare(item.baselineOffset, 0);

        item.leftPadding = 2;
        compare(leftPaddingSpy.count, 1), leftPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 1);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 1);
        compare(item.bottomPadding, 1);

        item.horizontalPadding = 3;
        compare(leftPaddingSpy.count, 0);
        compare(rightPaddingSpy.count, 1), rightPaddingSpy.clear();
        compare(horizontalPaddingSpy.count, 1), horizontalPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 1);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 3);
        compare(item.bottomPadding, 1);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 1);

        item.verticalPadding = 4;
        verify(waitForRendering(item))
        compare(topPaddingSpy.count, 1), topPaddingSpy.clear();
        compare(bottomPaddingSpy.count, 1), bottomPaddingSpy.clear();
        compare(verticalPaddingSpy.count, 1), verticalPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 4);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 3);
        compare(item.bottomPadding, 4);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 4);

        item.topPadding = 5;
        verify(waitForRendering(item))
        compare(topPaddingSpy.count, 1), topPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 5);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 3);
        compare(item.bottomPadding, 4);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 4);

        item.bottomPadding = 6;
        verify(waitForRendering(item))
        compare(bottomPaddingSpy.count, 1), bottomPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 5);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 3);
        compare(item.bottomPadding, 6);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 4);

        item.bottomPadding = undefined;
        compare(bottomPaddingSpy.count, 1), bottomPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 5);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 3);
        compare(item.bottomPadding, 4);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 4);

        item.verticalPadding = undefined;
        compare(topPaddingSpy.count, 0);
        compare(bottomPaddingSpy.count, 1), bottomPaddingSpy.clear();
        compare(verticalPaddingSpy.count, 1), verticalPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 5);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 3);
        compare(item.bottomPadding, 1);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 1);

        item.rightPadding = 7;
        verify(waitForRendering(item))
        compare(rightPaddingSpy.count, 1), rightPaddingSpy.clear();

        compare(item.padding, 1);
        compare(item.topPadding, 5);
        compare(item.leftPadding, 2);
        compare(item.rightPadding, 7);
        compare(item.bottomPadding, 1);
        compare(item.horizontalPadding, 3);
        compare(item.verticalPadding, 1);
    }

    Component {
        id: sizedComponent
        LingmoUI.Padding {
            topPadding: 1
            leftPadding: 2
            rightPadding: 30
            bottomPadding: 40

            contentItem: Rectangle {
                color: "#1EA8F7"
                implicitWidth: 100
                implicitHeight: 200
            }
        }
    }

    function test_fixedSizeComponent() {
        const item = createTemporaryObject(sizedComponent, this);
        verify(item);
        verify(waitForRendering(item))

        compare(item.implicitWidth, 132);
        compare(item.implicitHeight, 241);

        const widthSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "implicitWidthChanged" });
        const heightSpy = createTemporaryObject(spyComponent, this, { target: item, signalName: "implicitHeightChanged" });

        item.contentItem.implicitWidth = 1000;
        verify(waitForRendering(item))
        compare(widthSpy.count, 1), widthSpy.clear();
        compare(heightSpy.count, 0);

        item.contentItem.implicitHeight = 2000;
        verify(waitForRendering(item))
        compare(widthSpy.count, 0);
        compare(heightSpy.count, 1), heightSpy.clear();

        compare(item.implicitWidth, 1032);
        compare(item.implicitHeight, 2041);

        item.width = 100;
        item.height = 200;
        verify(waitForRendering(item))

        compare(item.contentItem.width, 100 - 2 - 30);
        compare(item.contentItem.height, 200 - 1 - 40);
    }

    Component {
        id: dynamicComponent
        LingmoUI.Padding {
            topPadding: 1
            leftPadding: 2
            rightPadding: 30
            bottomPadding: 40
        }
    }

    Component {
        id: contentComponent
        Rectangle {
            color: "#1EA8F7"
            implicitWidth: 100
            implicitHeight: 200
        }
    }

    function test_dynamicComponent() {
        const item = createTemporaryObject(dynamicComponent, this);
        verify(item);
        verify(waitForRendering(item))
        const content = createTemporaryObject(contentComponent, this);
        verify(content);

        compare(item.width, 32);
        compare(item.height, 41);

        item.contentItem = content;
        verify(waitForRendering(item))

        compare(item.implicitWidth, 132);
        compare(item.implicitHeight, 241);
    }
}
