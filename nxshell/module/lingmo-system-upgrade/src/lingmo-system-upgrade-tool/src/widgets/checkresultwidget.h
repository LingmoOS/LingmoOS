// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../core/constants.h"
#include "statusicon.h"
#include "simplelabel.h"
#include "iconlabel.h"

class CheckResultWidget : public QWidget
{
public:
    CheckResultWidget(QWidget *parent);
    void setIcon(QString iconPath);
    void setIcon(QString iconPath, int w, int h);
    void setStatus(CheckResultType status);
    void setTitle(QString text);
    void setToolTip(QString text);
    void setRequirement(QString text);
    void setResult(QString text);
    void scale(int w, int h);
    void addSpacerItem(QSpacerItem *item);
    void addWidget(QWidget *widget);

    QVBoxLayout *getMainLayout() const;
    void setMainLayout(QVBoxLayout *value);

private:
    void initUI();

    QString         m_toolTipText = QString("");
    QVBoxLayout     *m_mainLayout;
    QHBoxLayout     *m_tipLayout;
    IconLabel       *m_checkResultIconLabel;
    DLabel          *m_requirementTitleLabel;
    SimpleLabel     *m_requirementTipLabel;
    DLabel          *m_resultTextLabel;
    DLabel          *m_statusLabel;
};
