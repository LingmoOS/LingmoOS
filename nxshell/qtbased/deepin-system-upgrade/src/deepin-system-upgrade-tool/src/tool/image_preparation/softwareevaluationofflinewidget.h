// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DWidget>
#include <DLabel>
#include <DFontSizeManager>

#include <QWidget>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class SoftwareEvaluationOfflineWidget : public DWidget
{
    Q_OBJECT
public:
    SoftwareEvaluationOfflineWidget(QWidget *parent = nullptr);

private:
    void initUI();
    void initConnections();

    QVBoxLayout                *m_mainLayout;
    QVBoxLayout                *m_labelLayout;
    QWidget                    *m_mainWidget;
    DLabel                     *m_softwareChangeTitleLabel;
    DLabel                     *m_offlineTipsLabel;
};
