// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DSuggestButton>

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class MigrateFinishedWidget: public QWidget {
    Q_OBJECT
public:
    MigrateFinishedWidget(QWidget *parent = nullptr);

private:
    QLabel          *m_iconLabel;
    QLabel          *m_titleLabel;
    DSuggestButton  *m_confirmButton;
    QVBoxLayout     *m_mainLayout;
};
