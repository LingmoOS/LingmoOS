// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagedialog.h"
#include <QIcon>

MessageDialog::MessageDialog(QWidget *parent) : DDialog(parent)
{

}

MessageDialog::MessageDialog(const QString &message, QWidget *parent):
    DDialog(parent)
{
    m_msg = message;
    initUI();
}

void MessageDialog::initUI()
{
    setIcon(QIcon::fromTheme("dialog-warning"));
    addButton(tr("OK"), true, DDialog::ButtonRecommend);
    setTitle(m_msg);
}

