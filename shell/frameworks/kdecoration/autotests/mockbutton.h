/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/decorationbutton.h"

class MockButton : public KDecoration2::DecorationButton
{
    Q_OBJECT
public:
    MockButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent = nullptr);
    void paint(QPainter *painter, const QRect &repaintRegion) override;
};
