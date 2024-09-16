// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DTableWidget>

#include <QHeaderView>

#include "borderradiusheaderview.h"

DWIDGET_USE_NAMESPACE

class SoftwareTableWidget : public DTableWidget {
public:
    explicit SoftwareTableWidget(QWidget *parent = nullptr)
        : DTableWidget(parent)
        , m_headerView(new BorderRadiusHeaderView(Qt::Horizontal, this))
    {
        initUI();
    }

public:
    BorderRadiusHeaderView * getHeaderView();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initUI();
    void setRowColorStyles();

    BorderRadiusHeaderView *m_headerView;
};
