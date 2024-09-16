// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogmanage.h"
#include "../window/modules/common/common.h"
#include "../window/modules/common/compixmap.h"

#include <DDialog>
#include <DSuggestButton>

#include <QLabel>
#include <QGSettings>
#include <QStorageInfo>
#include <QDebug>
#include <QApplication>

DWIDGET_USE_NAMESPACE

DialogManage::DialogManage(QObject *parent)
    : QObject(parent)
{
}

// 更新提示框
void DialogManage::showUpdateWhenScanningDialog()
{
    DDialog *dialog = new DDialog;
    dialog->setAccessibleName("updateWhenScanningDialog");
    dialog->setMinimumSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setMaximumSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setIcon(QIcon::fromTheme("dcc_login_tip"));
    // 当前正在查杀中，请结束查杀后再更新
    dialog->setMessage(tr("A task is in process, please update later"));
    dialog->setWordWrapMessage(true);
    // 设置属性当关闭时候删除自己
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // 确定   推荐
    DSuggestButton *okBtn = new DSuggestButton(dialog);
    okBtn->setText(tr("OK"));
    okBtn->setObjectName("OK");
    okBtn->setAccessibleName("updateWhenScanningDialog_okButton");
    okBtn->setAttribute(Qt::WA_NoMousePropagation);
    dialog->insertButton(0, okBtn, false);
    connect(dialog, &DDialog::buttonClicked, dialog, [dialog](int index, QString) {
        switch (index) {
        case 0:
            dialog->close();
            break;
        default:
            break;
        }
    });
    dialog->exec();
}

void DialogManage::showNormalTipDialog(TipIcon icontype, TipTextInfo textInfo)
{
    DDialog *dialog = new DDialog;
    dialog->setAccessibleName("normalTtipDialog");
    dialog->setMinimumSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setMaximumSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    // 根据图标类型设置左上角图标
    if (IconNormal == icontype) {
        dialog->setIcon(QIcon::fromTheme(DIALOG_DEFENDER));
    } else if (IconTip == icontype) {
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
    } else {
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_RED));
    }

    // 根据信息类型提示不同信息
    switch (textInfo) {
    case TipHomeChecking:
        // 提示语 首页体检正在执行快速查杀，请稍候
        dialog->setMessage(tr("Quick scanning in the Home page, please try later"));
        break;
    case TipHomeThreats:
        // 提示语：首页体检有病毒异常未处理，请前往处理
        dialog->setMessage(tr("Threats are found in the Home page, please go to remove them first"));
        break;
    case TipHomeFixing:
        // 提示语：首页体检正在执行修复任务，请稍候
        dialog->setMessage(tr("Fixing issues in the Home page, please try later"));
        break;
    case TipThreatScaning:
        // 提示语：病毒查杀正在执行查杀任务，请稍候
        dialog->setMessage(tr("Scanning virus, please try later"));
        break;
    case TipUsbScaning:
        // 提示语：正在执行U盘查杀，请稍候
        dialog->setMessage(tr("Scanning USB devices, please try later"));
        break;
    case TipUpdate:
        // 提示语：病毒库更新中，请稍候操作
        dialog->setMessage(tr("Updating the virus database, please try later"));
        break;
    case TipNetWorkEroor:
        // 提示语：网络异常，请检测网络后重新更新
        dialog->setMessage(tr("Please check your network and update again"));
        break;
    default:
        // 如果输入类型错误
        dialog->setMessage(tr("Scanning virus, please try later"));
        break;
    }

    dialog->setWordWrapMessage(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 按钮提示信息是确认
    // 因自动化测试需要,修改dialog按钮的创建过程
    DSuggestButton *confirmBtn = new DSuggestButton(dialog);
    confirmBtn->setText(tr("OK"));
    confirmBtn->setObjectName("ConfirmBtn");
    confirmBtn->setAccessibleName("normalTtipDialog_confirmBtn");
    confirmBtn->setAttribute(Qt::WA_NoMousePropagation);
    dialog->insertButton(0, confirmBtn, true);
    QLabel *dialogMessageLabel = dialog->findChild<QLabel *>("MessageLabel");
    dialogMessageLabel->setAccessibleName("normalTtipDialog_messageLable");
    connect(dialog, &DDialog::buttonClicked, this, [=](int index, QString) {
        switch (index) {
        case 0:
            emit buttonClicked();
            dialog->close();
            break;
        default:
            break;
        }
    });
    connect(dialog, &DDialog::closed, this, &DialogManage::closed);

    dialog->exec();
}

// 更新服务地址提示窗
DDialog *DialogManage::creatChangeAddressTipDialog()
{
    DDialog *dialog = new DDialog;
    dialog->setAccessibleName("addressChangeDialog");
    dialog->setMinimumSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setMaximumSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
    // 请检查你的病毒服务地址是否正常
    dialog->setMessage(tr("Trying to update the virus database, please make sure its server address is correct"));
    dialog->setWordWrapMessage(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // 确定推荐
    QPushButton *okBtn = new QPushButton(dialog);
    okBtn->setText(tr("OK"));
    okBtn->setObjectName("OK");
    okBtn->setAccessibleName("addressChangeDialog_okButton");
    okBtn->setAttribute(Qt::WA_NoMousePropagation);
    dialog->insertButton(0, okBtn, false);
    connect(dialog, &DDialog::buttonClicked, this, [=](int index, QString) {
        switch (index) {
        case 0:
            dialog->close();
            break;
        default:
            break;
        }
    });
    return dialog;
}
