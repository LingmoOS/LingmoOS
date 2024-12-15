// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QIcon>
#include <QWidget>

namespace def {
namespace widgets {
class SelectedIconWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectedIconWidget(QWidget *parent = nullptr);
    virtual ~SelectedIconWidget() override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QIcon m_icon;
};
} // namespace widgets

} // namespace def
