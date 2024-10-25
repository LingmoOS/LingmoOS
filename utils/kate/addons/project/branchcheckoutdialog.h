/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "branchesdialog.h"

class BranchCheckoutDialog : public BranchesDialog
{
public:
    BranchCheckoutDialog(QWidget *mainWindow, QString projectPath);
    ~BranchCheckoutDialog() override;
    void openDialog();

private Q_SLOTS:
    void slotReturnPressed(const QModelIndex &index) override;
    void onCheckoutDone();

private:
    void resetValues();
    void createNewBranch(const QString &branch, const QString &fromBranch = QString());

private:
    QFutureWatcher<GitUtils::CheckoutResult> m_checkoutWatcher;
    QString m_checkoutFromBranchName;
    bool m_checkingOutFromBranch = false;
};
