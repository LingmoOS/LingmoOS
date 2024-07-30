/*
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.PageIndicator {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: Kirigami.Units.largeSpacing
    spacing: Kirigami.Units.largeSpacing

    // QTBUG-115133: Kirigami/ShadowedRectangle renders smoother circles at HiDPI than plain QtQuick/Rectangle.
    delegate: Kirigami.ShadowedRectangle {
        required property int index
        // `pressed` is a context property, it can't be required in delegate.

        implicitWidth: Kirigami.Units.largeSpacing
        implicitHeight: Kirigami.Units.largeSpacing

        radius: width / 2
        color: Kirigami.Theme.textColor

        opacity: index === currentIndex ? 1 : pressed ? 0.67 : 0.33

        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.shortDuration
            }
        }
    }

    // Can't be center-aligned, because (1) an approach of setting x/width
    // manually gets overridden by automatic sizing of Control; and
    // (2) wrapping in an Item breaks delegates for which T.PageIndicator
    // injects `pressed` context property; (3) RowLayout with center
    // alignment tends to distribute its items across width, which looks odd,
    // and again due to custom C++ magic relying on children index of
    // contentItem, we can't insert fillers on the left and right.
    contentItem: Row {
        LayoutMirroring.enabled: control.mirrored
        spacing: control.spacing

        Repeater {
            model: control.count
            delegate: control.delegate
        }
    }
}
