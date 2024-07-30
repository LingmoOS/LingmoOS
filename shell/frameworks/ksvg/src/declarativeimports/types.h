/*
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSVG_TYPES_H
#define KSVG_TYPES_H

#include <QQmlEngine>

#include <Kirigami/Platform/PlatformTheme>

#include <framesvg.h>
#include <svg.h>

struct PlatformThemeForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(Kirigami::Platform::PlatformTheme)
};

struct SvgForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(Svg)
    QML_FOREIGN(KSvg::Svg)
};

struct FrameSvgForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(FrameSvg)
    QML_FOREIGN(KSvg::FrameSvg)
};

struct ImageSetForeign {
    Q_GADGET
    QML_ELEMENT
    QML_SINGLETON
    QML_NAMED_ELEMENT(ImageSet)
    QML_FOREIGN(KSvg::ImageSet)
};

#endif
