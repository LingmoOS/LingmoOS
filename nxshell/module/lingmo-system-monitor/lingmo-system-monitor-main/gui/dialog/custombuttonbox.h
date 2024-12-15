// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMBUTTONBOX_H
#define CUSTOMBUTTONBOX_H

#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class CustomButtonBox : public DButtonBox
{
    Q_OBJECT
public:
    explicit CustomButtonBox(QWidget *parent = nullptr);

protected:
    //!
    //! \brief focusInEvent 焦点切换事件
    //! \param event
    //!
    void focusInEvent(QFocusEvent *event);
signals:

public slots:
};

#endif // CUSTOMBUTTONBOX_H
