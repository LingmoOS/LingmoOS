/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration2 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "decorationshadow.h"

#include <QImage>

namespace KDecoration2
{
class Q_DECL_HIDDEN DecorationShadow::Private
{
public:
    explicit Private(DecorationShadow *parent);
    ~Private();
    QImage shadow;
    QRect innerShadowRect;
    QMargins padding;

private:
    DecorationShadow *q;
};

}
