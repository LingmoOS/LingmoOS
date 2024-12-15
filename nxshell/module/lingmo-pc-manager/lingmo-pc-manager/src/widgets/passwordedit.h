// Copyright (C) 2019 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_PASSWORDEDIT_H
#define DCC_PASSWORDEDIT_H

#include <DPasswordEdit>

class PasswordEdit : public DTK_WIDGET_NAMESPACE::DPasswordEdit
{
    Q_OBJECT
public:
    explicit PasswordEdit(QWidget *parent = 0);

protected:
    bool eventFilter(QObject *, QEvent *) override;
};
#endif // DCC_TRANSLUCENTFRAME_H
