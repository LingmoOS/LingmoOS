/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QString>

#include "decorationpainter.h"

class AuroraeDecorationPainter : public DecorationPainter
{
public:
    AuroraeDecorationPainter(const QString &themeName);
    ~AuroraeDecorationPainter() = default;

    void paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState) const override;

private:
    QString buttonTypeToFileName(const QString &buttonType) const;
    QString buttonStateToElementId(const QString &buttonState) const;

    static const QString s_auroraeThemesPath;

    const QString m_themeName;
    const QString m_themePath;
};
