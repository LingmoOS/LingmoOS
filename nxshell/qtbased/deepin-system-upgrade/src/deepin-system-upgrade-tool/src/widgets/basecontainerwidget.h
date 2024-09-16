// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DPushButton>
#include <DSuggestButton>

#include <QWidget>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../widgets/threedotswidget.h"

DWIDGET_USE_NAMESPACE

/**
 * @brief Container for main function interface.
 *
 */
class BaseContainerWidget : public QWidget
{
public:
    explicit BaseContainerWidget(QWidget *parent = nullptr, int aDot = 0);
    void addContentWidget(QWidget *widget);

protected:
    DPushButton     *m_cancelButton;
    DPushButton     *m_midButton;
    DSuggestButton  *m_suggestButton;
    QSpacerItem     *m_spacerItem;
    QSpacerItem     *m_buttonSpacerItem;
    QVBoxLayout     *m_mainLayout;
    QVBoxLayout     *m_contentLayout;
    QVBoxLayout     *m_bottomUpperLayout;
    QVBoxLayout     *m_bottomLayout;
    QHBoxLayout     *m_buttonLayout;
    ThreeDotsWidget *m_threeDotsWidget;

    void initUI();
};
