// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enumtext.h"

namespace Docx {

QString underLineToString(WD_UNDERLINE underline)
{
    switch (underline) {
    case WD_UNDERLINE::None:
        return QStringLiteral("none");
    case WD_UNDERLINE::SINGLE:
        return QStringLiteral("single");
    case WD_UNDERLINE::WORDS:
        return QStringLiteral("words");
    case WD_UNDERLINE::DOUBLE:
        return QStringLiteral("double");
    case WD_UNDERLINE::DOTTED:
        return QStringLiteral("dotted");
    case WD_UNDERLINE::THICK:
        return QStringLiteral("thick");
    case WD_UNDERLINE::DASH:
        return QStringLiteral("dash");
    case WD_UNDERLINE::DOT_DASH:
        return QStringLiteral("dotDash");
    case WD_UNDERLINE::DOT_DOT_DASH:
        return QStringLiteral("dotDotDash");
    case WD_UNDERLINE::WAVY:
        return QStringLiteral("wave");
    case WD_UNDERLINE::DOTTED_HEAVY:
        return QStringLiteral("dottedHeavy");
    case WD_UNDERLINE::DASH_HEAVY:
        return QStringLiteral("dashedHeavy");
    case WD_UNDERLINE::DOT_DASH_HEAVY:
        return QStringLiteral("dashDotHeavy");
    case WD_UNDERLINE::DOT_DOT_DASH_HEAVY:
        return QStringLiteral("dashDotDotHeavy");
    case WD_UNDERLINE::WAVY_HEAVY:
        return QStringLiteral("wavyHeavy");
    case WD_UNDERLINE::DASH_LONG:
        return QStringLiteral("dashLong");
    case WD_UNDERLINE::WAVY_DOUBLE:
        return QStringLiteral("wavyDouble");
    case WD_UNDERLINE::DASH_LONG_HEAVY:
        return QStringLiteral("dashLongHeavy");
    }
    return "";
}

QString paragraphAlignToString(WD_PARAGRAPH_ALIGNMENT align)
{
    switch (align) {
    case WD_PARAGRAPH_ALIGNMENT::LEFT:
        return QStringLiteral("left");
    case WD_PARAGRAPH_ALIGNMENT::CENTER:
        return QStringLiteral("center");
    case WD_PARAGRAPH_ALIGNMENT::RIGHT:
        return QStringLiteral("right");
    case WD_PARAGRAPH_ALIGNMENT::JUSTIFY:
        return QStringLiteral("both");
    case WD_PARAGRAPH_ALIGNMENT::DISTRIBUTE:
        return QStringLiteral("distribute");
    case WD_PARAGRAPH_ALIGNMENT::JUSTIFY_MED:
        return QStringLiteral("mediumKashida");
    case WD_PARAGRAPH_ALIGNMENT::JUSTIFY_HI:
        return QStringLiteral("highKashida");
    case WD_PARAGRAPH_ALIGNMENT::JUSTIFY_LOW:
        return QStringLiteral("lowKashida");
    case WD_PARAGRAPH_ALIGNMENT::THAI_JUSTIFY:
        return QStringLiteral("thaiDistribute");
    }
    return "";
}

}


