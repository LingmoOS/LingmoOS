// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appaccessiblebutton.h"

AppAccessibleButton::AppAccessibleButton(QPushButton *btn)
    : AppAccessibleBase(btn)
    , m_widget(btn)
{
}

QString AppAccessibleButton::text(int startOffset, int endOffset) const
{
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);

    if (m_widget) {
        return m_widget->text();
    }

    return QString();
}
