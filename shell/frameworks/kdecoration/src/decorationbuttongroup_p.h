/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once
#include "decorationbuttongroup.h"

#include <QList>
#include <QRectF>

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

namespace KDecoration2
{
class Decoration;

class Q_DECL_HIDDEN DecorationButtonGroup::Private
{
public:
    explicit Private(Decoration *decoration, DecorationButtonGroup *parent);
    ~Private();

    void setGeometry(const QRectF &geometry);
    void updateLayout();

    Decoration *decoration;
    QRectF geometry;
    QList<DecorationButton *> buttons;
    qreal spacing;

private:
    DecorationButtonGroup *q;
};

} // namespace
