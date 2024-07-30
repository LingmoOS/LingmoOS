/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "decorationpainter.h"
#include "dummydecorationbridge.h"

class StandardDecorationPainter : public DecorationPainter
{
public:
    StandardDecorationPainter(const QString &themeName);
    ~StandardDecorationPainter() = default;

    void paintButton(QPainter &painter, const QString &buttonType, const QString &buttonState) const override;

private:
    std::unique_ptr<KDecoration2::DummyDecorationBridge> m_bridge;
};
