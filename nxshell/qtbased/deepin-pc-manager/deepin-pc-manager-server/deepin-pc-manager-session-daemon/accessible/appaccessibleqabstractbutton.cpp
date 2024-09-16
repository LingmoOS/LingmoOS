// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appaccessibleqabstrractbutton.h"

AppAccessibleQAbstractButton::AppAccessibleQAbstractButton(QAbstractButton *btn)
    : AppAccessibleBase(btn)
    , m_widget(btn)
{
}

QString AppAccessibleQAbstractButton::text(int startOffset, int endOffset) const
{
    Q_UNUSED(startOffset);
    Q_UNUSED(endOffset);

    if (m_widget) {
        return m_widget->text();
    }

    return QString();
}
