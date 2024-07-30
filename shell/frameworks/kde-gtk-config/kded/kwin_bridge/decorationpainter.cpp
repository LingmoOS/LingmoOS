/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "decorationpainter.h"

#include <QRect>
#include <QString>

#include "auroraedecorationpainter.h"
#include "standarddecorationpainter.h"

const QRect DecorationPainter::ButtonGeometry{0, 0, 50, 50};

std::unique_ptr<DecorationPainter> DecorationPainter::fromThemeName(const QString &themeName)
{
    static const QString auroraeThemePrefix = QStringLiteral("__aurorae__svg__");
    if (themeName.startsWith(auroraeThemePrefix)) {
        QString prefixlessThemeName = themeName.mid(auroraeThemePrefix.size());
        return std::unique_ptr<AuroraeDecorationPainter>{new AuroraeDecorationPainter(prefixlessThemeName)};
    } else {
        return std::unique_ptr<StandardDecorationPainter>{new StandardDecorationPainter(themeName)};
    }
}
