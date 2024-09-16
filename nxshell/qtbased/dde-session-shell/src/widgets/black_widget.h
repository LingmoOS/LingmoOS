// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class BlackWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlackWidget(QWidget *parent = nullptr);

    void setBlackMode(const bool isBlackMode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QCursor m_cursor;
};
