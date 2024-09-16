// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>

#include <QVariant>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QWidget>
#include <QSpacerItem>

#include "../core/constants.h"
#include "../widgets/iconlabel.h"
#include "../widgets/basecontainerwidget.h"
#include "../widgets/simplelabel.h"

DWIDGET_USE_NAMESPACE

class InitialWidget: public BaseContainerWidget
{
    Q_OBJECT
public:
    explicit InitialWidget(QWidget *parent = nullptr);
    void initUI();
    void initConnections();

Q_SIGNALS:
    void upgradeCheckButtonClicked(bool checked = false);

private Q_SLOTS:
    void enableUpgradeCheckButton(int state);
    void openLicenseDetails(LicenseType type);

private:
    int         m_checkedCount;
    QHBoxLayout *m_userLicenseLayout;
    QCheckBox   *m_userLicenseCheckBox;
    DLabel      *m_userLicenseLabel;
    DLabel      *m_privacyLicenseLabel;
    QVBoxLayout *m_verticalLayout;
    IconLabel   *m_iconLabel;
    DLabel      *m_titleLabel;
    SimpleLabel *m_textLabel;
    SimpleLabel *m_currentVersionLabel;
};
