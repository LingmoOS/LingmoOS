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

DTK_USE_NAMESPACE

class StorageResultWidget : public QWidget
{
    Q_OBJECT
public:
    StorageResultWidget(QWidget *parent);
    void setIcon(QString iconPath);
    void setIcon(QString iconPath, int w, int h);
    void setTitle(QString text);
    void setToolTip(QString text);
    void setRequirement(QString text);
    void setStatus(CheckResultType status, DLabel *statusLabel, DLabel *resultTextLabel);
    void setSystemResult(QString text);
    void setSystemStatus(CheckResultType status);
    void setDataResult(QString text);
    void setDataStatus(CheckResultType status);
    CheckResultType getSystemStatus() {
        return m_systemStatus;
    }
    CheckResultType getDataStatus() {
        return m_dataStatus;
    }
    void scale(int w, int h);
    void addSpacerItem(QSpacerItem *item);
    void addWidget(QWidget *widget);

private slots:
    void openCleanupDialog();

private:
    QString         m_toolTipText = QString("");
    CheckResultType m_systemStatus;
    CheckResultType m_dataStatus;
    QHBoxLayout     *m_mainLayout;
    QVBoxLayout     *m_leftLayout;
    QVBoxLayout     *m_resultLayout;
    QHBoxLayout     *m_systemResultLayout;
    QHBoxLayout     *m_dataResultLayout;
    QHBoxLayout     *m_tipLayout;
    IconLabel       *m_checkResultIconLabel;
    DLabel          *m_requirementTitleLabel;
    SimpleLabel     *m_requirementTipLabel;
    DLabel          *m_systemResultTextLabel;
    DLabel          *m_dataResultTextLabel;
    DLabel          *m_systemStatusLabel;
    DLabel          *m_dataStatusLabel;
    DLabel          *m_cleanupSystemLabel;
    DLabel          *m_cleanupDataLabel;
    QSpacerItem     *m_cleanupSystemSpacerItem;
    QSpacerItem     *m_cleanupDataSpacerItem;

    void initUI();
    void initConnections();
};
