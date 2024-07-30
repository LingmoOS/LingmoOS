/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <memory>

class QString;
class QPainter;
class QRect;

class DecorationPainter
{
public:
    DecorationPainter() = default;
    virtual ~DecorationPainter() = default;

    static std::unique_ptr<DecorationPainter> fromThemeName(const QString &themeName);
    virtual void paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState) const = 0;

    static const QRect ButtonGeometry;
};
