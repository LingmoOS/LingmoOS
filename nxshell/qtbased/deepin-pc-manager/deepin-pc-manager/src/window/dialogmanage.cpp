// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogmanage.h"

#include "modules/common/common.h"
#include "modules/common/compixmap.h"

#include <DDialog>
#include <DSuggestButton>

#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QScrollArea>
#include <QStorageInfo>

DWIDGET_USE_NAMESPACE

DialogManage::DialogManage(QObject *parent)
    : QObject(parent)
{
}

// 更新提示框
void DialogManage::showUpdateWhenScanningDialog()
{
    DDialog *dialog = new DDialog;
    dialog->setFont(Utils::getFixFontSize(14));
    dialog->setAccessibleName("updateWhenScanningDialog");
    dialog->setFixedSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setIcon(QIcon::fromTheme("dcc_login_tip"));
    // 当前正在查杀中，请结束查杀后再更新
    dialog->setMessage(tr("A task is in process, please update later"));
    dialog->setWordWrapMessage(true);
    // 设置属性当关闭时候删除自己
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // 确定   推荐
    DSuggestButton *okBtn = new DSuggestButton(dialog);
    okBtn->setFont(Utils::getFixFontSize(14));
    okBtn->setText(tr("OK", "button"));
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
    dialog->setFont(Utils::getFixFontSize(14));
    dialog->setAccessibleName("normalTtipDialog");
    dialog->setFixedSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    // 根据图标类型设置左上角图标
    if (IconNormal == icontype) {
        dialog->setIcon(QIcon::fromTheme(DIALOG_PC_MANAGER));
    } else if (IconTip == icontype) {
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
    } else {
        dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_RED));
    }

    QString strTips;

    // 根据信息类型提示不同信息
    switch (textInfo) {
    case TipHomeChecking:
        // 提示语 首页体检正在执行快速查杀，请稍候
        strTips = tr("Quick scanning on the Full Check page, please try later");
        break;
    case TipHomeThreats:
        // 提示语：首页体检有病毒异常未处理，请前往处理
        strTips = tr("Threats are found on the Full Check page, please go to remove them first");
        break;
    case TipHomeFixing:
        // 提示语：首页体检正在执行修复任务，请稍候
        strTips = tr("Fixing issues on the Full Check page, please try later");
        break;
    case TipThreatScaning:
        // 提示语：病毒查杀正在执行查杀任务，请稍候
        strTips = tr("Scanning virus, please try later");
        break;
    case TipUsbScaning:
        // 提示语：正在执行U盘查杀，请稍候
        strTips = tr("Scanning USB devices, please try later");
        break;
    case TipUpdate:
        // 提示语：病毒库更新中，请稍候操作
        strTips = tr("Updating the virus database, please try later");
        break;
    case TipNetWorkEroor:
        // 提示语：网络异常，请检测网络后重新更新
        strTips = tr("Please check your network and update again");
        break;
    default:
        // 如果输入类型错误
        strTips = tr("Scanning virus, please try later");
        break;
    }

    QScrollArea *mainScrollArea = new QScrollArea(dialog);
    mainScrollArea->setContentsMargins(0, 0, 0, 0);
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    // 设置滑动条区域背景颜色为透明
    QPalette pa = mainScrollArea->palette();
    pa.setBrush(QPalette::Window, Qt::transparent);
    mainScrollArea->setPalette(pa);

    QLabel *txtLabel = new QLabel(mainScrollArea);
    txtLabel->setFont(Utils::getFixFontSize(14));
    txtLabel->setText(strTips);
    txtLabel->setWordWrap(true);
    txtLabel->setAlignment(Qt::AlignCenter);

    mainScrollArea->setWidget(txtLabel);

    dialog->addContent(mainScrollArea);

    dialog->setWordWrapMessage(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 按钮提示信息是确认
    // 因自动化测试需要,修改dialog按钮的创建过程
    DSuggestButton *confirmBtn = new DSuggestButton(dialog);
    confirmBtn->setFont(Utils::getFixFontSize(14));
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
    dialog->setFont(Utils::getFixFontSize(14));
    dialog->setAccessibleName("addressChangeDialog");
    dialog->setFixedSize(QSize(TIP_DIALOG_WIDTH, TIP_DIALOG_HEIGHT));
    dialog->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));

    // 创建滑动区----请检查你的病毒服务地址是否正常
    QScrollArea *mainScrollArea = new QScrollArea(dialog);
    mainScrollArea->setContentsMargins(0, 0, 0, 0);
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    // 设置滑动条区域背景颜色为透明
    QPalette pa = mainScrollArea->palette();
    pa.setBrush(QPalette::Window, Qt::transparent);
    mainScrollArea->setPalette(pa);

    QLabel *txtLabel = new QLabel(mainScrollArea);
    txtLabel->setFont(Utils::getFixFontSize(14));
    txtLabel->setText(
        tr("Trying to update the virus database, please make sure its server address is correct"));
    txtLabel->setWordWrap(true);
    txtLabel->setAlignment(Qt::AlignCenter);

    mainScrollArea->setWidget(txtLabel);

    dialog->addContent(mainScrollArea);

    dialog->setWordWrapMessage(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // 确定推荐
    QPushButton *okBtn = new QPushButton(dialog);
    okBtn->setFont(Utils::getFixFontSize(14));
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
