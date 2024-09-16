// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "restartdefenderdialog.h"

#include <DLabel>

#define RESTART_CANCEL 0 // 弹框第一个按钮
#define RESTART_OK 1 // 弹框第二个按钮

RestartDefenderDialog::RestartDefenderDialog(DDialog *parent)
    : DDialog(parent)
    , m_daemonservice(new DaemonService("com.deepin.defender.daemonservice", "/com/deepin/defender/daemonservice", QDBusConnection::sessionBus(), this))
{
    // 初始化界面
    initUi();
}

// 初始化界面
void RestartDefenderDialog::initUi()
{
    setMinimumSize(QSize(200, 100));
    // 弹框样式
    setCloseButtonVisible(true);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setModal(true);

    // 清除布局
    clearContents();
    clearButtons();
    setIcon(QIcon::fromTheme("deepin-defender"));

    // 添加弹框文字
    DLabel *txt = nullptr;
    txt = new DLabel(tr("Changes will take effect after the application restarts"), this);
    // 字体剧中
    txt->setAlignment(Qt::AlignCenter);
    txt->setWordWrap(true);

    // 添加布局
    addContent(txt);
    addButton(tr("Cancel"));
    addButton(tr("Restart"));

    // 按钮点击信号槽
    connect(this, &RestartDefenderDialog::buttonClicked, this, &RestartDefenderDialog::doButtonClicked);
}

RestartDefenderDialog::~RestartDefenderDialog()
{
    Q_EMIT onDialogClose();
}

// 按钮点击槽
void RestartDefenderDialog::doButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text);
    if (index == RESTART_OK) {
        Q_EMIT onRestartClose();
        m_daemonservice->ExitAllService();
    }
}
