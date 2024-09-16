// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPACCESSIBLEDCOMMANDLINKBUTTON_H
#define APPACCESSIBLEDCOMMANDLINKBUTTON_H

#include "appaccessiblebase.h"
#include <QAbstractButton>

class AppAccessibleQAbstractButton : public AppAccessibleBase
{
public:
    explicit AppAccessibleQAbstractButton(QAbstractButton *);
    virtual QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QAbstractButton *m_widget;
};

#endif // APPACCESSIBLEDCOMMANDLINKBUTTON_H
