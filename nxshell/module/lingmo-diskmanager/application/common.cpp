// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "common.h"

QIcon Common::getIcon(const QString &iconName)
{
    return QIcon::fromTheme("dm_" + iconName);
}

int Common::getLabelAdjustHeight(const int &width, const QString &text, const QFont &font)
{
    DLabel label;
    label.setFont(font);
    label.setWordWrap(true);
    label.setFixedWidth(width);
    label.setText(text);
    label.adjustSize();

    return label.height();
}
