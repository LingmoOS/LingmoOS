// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pwdlevelchangeddlg.h"

#include <DLabel>

#include <DSuggestButton>

DEF_NAMESPACE_BEGIN
PwdLevelChangedDlg::PwdLevelChangedDlg(QWidget *parent)
    : DDialog(parent)
{
    this->setAccessibleName("pwdLevelDialog");
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setModal(true);
    setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
    clearContents(); //清除文本
    clearButtons(); //清除按钮
    DLabel *txt = new DLabel(tr("Your password policy is higher, please change your password to meet the requirements"), this);
    txt->setAccessibleName("pwdLevelDialog_tipLable");
    txt->setWordWrap(true); //设置换行
    addContent(txt);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    DSuggestButton *okBtn = new DSuggestButton(this);
    okBtn->setText(tr("OK"));
    okBtn->setObjectName("okBtn");
    okBtn->setAccessibleName("pwdLevelDialog_okBtn");
    okBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(0, okBtn, false);
}

PwdLevelChangedDlg::~PwdLevelChangedDlg()
{
}
DEF_NAMESPACE_END
