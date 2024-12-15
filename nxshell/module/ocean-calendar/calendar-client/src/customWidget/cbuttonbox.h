// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CBUTTONBOX_H
#define CBUTTONBOX_H

#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class CButtonBox : public DButtonBox
{
    Q_OBJECT
public:
    explicit CButtonBox(QWidget *parent = nullptr);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
signals:

public slots:
};

#endif // CBUTTONBOX_H
