// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class DFontSpinnerPrivate;

class DFontSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit DFontSpinner(QWidget *parent = nullptr);
    ~DFontSpinner() override;

public Q_SLOTS:
    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *e) override;

private:
// d_ptr与Qobject库中一个protected权限的指针变量重名，导致会出现警告
//    DFontSpinnerPrivate *const d_ptr {nullptr};
    DFontSpinnerPrivate *const m_ptr {nullptr};
    Q_DECLARE_PRIVATE(DFontSpinner)
};
