// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "error_notify_dialog_helper.h"

#include <QStyle>
#include <QLabel>
#include <QPushButton>

#include <DDialog>

#include "utils/hierarchicalverify.h"

DWIDGET_USE_NAMESPACE

static const int kDefaultDialogWidth = 380;

/**
 * @class ErrorNotifyDialogHelper
 * @brief A helper class to show the error notify dialog.
 */
ErrorNotifyDialogHelper::ErrorNotifyDialogHelper(QObject *parent)
    : QObject{parent}
{
}

void ErrorNotifyDialogHelper::showHierarchicalVerifyWindow()
{
    if (!HierarchicalVerify::instance()->isValid()) {
        return;
    }

    DDialog *dialog = new DDialog();
    // limit the display width
    dialog->setFixedWidth(kDefaultDialogWidth);
    dialog->setFocusPolicy(Qt::TabFocus);
    dialog->setModal(true);
    dialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // set the information displayed by the pop-up window
    dialog->setTitle(QObject::tr("Unable to install"));
    dialog->setMessage(
        QObject::tr("This package does not have a valid digital signature and has been blocked from installing/running. "
                    "Go to Security Center > Tools > App Security to change the settings."));
    dialog->setIcon(QIcon::fromTheme("di_popwarning"));
    dialog->addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
    dialog->addButton(QObject::tr("Proceed", "button"), true, DDialog::ButtonRecommend);
    dialog->show();

    // Copy from ddialog.cpp, used to set the default dialog height.
    // Avoid incomplete display at large font size/high zoom ratio, called after show().
    auto *msgLabel = dialog->findChild<QLabel *>("MessageLabel");
    if (msgLabel) {
        auto *dialogStyle = dialog->style();
        if (dialogStyle) {
            const QSize sz =
                dialogStyle->itemTextRect(msgLabel->fontMetrics(), msgLabel->rect(), Qt::TextWordWrap, false, msgLabel->text())
                    .size();
            msgLabel->setMinimumHeight(qMax(sz.height(), msgLabel->sizeHint().height()));
        }
    }

    auto *btnPorceed = qobject_cast<QPushButton *>(dialog->getButton(1));
    if (btnPorceed) {
        btnPorceed->setFocusPolicy(Qt::TabFocus);
        btnPorceed->setFocus();
    }

    connect(dialog, &DDialog::finished, [dialog](int result) {
        if (QDialog::Accepted == result) {
            HierarchicalVerify::instance()->proceedDefenderSafetyPage();
        }
        dialog->deleteLater();
    });
}
