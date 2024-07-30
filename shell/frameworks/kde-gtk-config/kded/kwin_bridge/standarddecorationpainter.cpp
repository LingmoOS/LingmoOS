/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "standarddecorationpainter.h"

StandardDecorationPainter::StandardDecorationPainter(const QString &themeName)
    : DecorationPainter()
    , m_bridge(new KDecoration2::DummyDecorationBridge(themeName))
{
}

void StandardDecorationPainter::paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState) const
{
    m_bridge->paintButton(painter, buttonType, buttonState);
}
