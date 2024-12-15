// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickborderimage_p.h"

#include <private/qquickitem_p.h>
#include <private/qquickborderimage_p_p.h>

DQUICK_BEGIN_NAMESPACE

DQuickBorderImage::DQuickBorderImage(QQuickItem *parent)
    : QQuickBorderImage(parent)
{
}

void DQuickBorderImage::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemDevicePixelRatioHasChanged) {
        return;
    }

    QQuickBorderImage::itemChange(change, value);
}

DQUICK_END_NAMESPACE
