// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netdisabledialog.h"

#include <DWidget>
#include <DLabel>
#include <DWarningButton>

#include <QTimer>
#include <QPushButton>

#define NET_SETTING 0 // 弹框第一个按钮
#define NET_OK 1 // 弹框第二个按钮

static const int TIMEOUT_COUNT = 15;
NetDisableDialog::NetDisableDialog(const QString &sPkgName, const QString &sAppName, DDialog *parent)
    : DDialog(parent)
    , m_sAppName(sAppName)
    , m_sPkgName(sPkgName)
    , m_timer(nullptr)
    , m_elapsedSeconds(TIMEOUT_COUNT)
    , m_hmiScreen(new HmiScreen("com.deepin.defender.hmiscreen", "/com/deepin/defender/hmiscreen", QDBusConnection::sessionBus(), this))
{
    this->setAccessibleName("netDisableDialog");
    // 初始化界面
    initUi();
}

// 初始化界面
void NetDisableDialog::initUi()
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
    // 添加布局
    DLabel *txt = nullptr;
    txt = new DLabel(tr("The firewall has blocked %1 from connecting to the Internet").arg(m_sAppName.isEmpty() ? m_sPkgName : m_sAppName));
    txt->setAccessibleName("netDisableDialog_tipLable");

    // 字体剧中
    txt->setAlignment(Qt::AlignCenter);
    txt->setWordWrap(true);
    addContent(txt);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    QPushButton *goToSetBtn = new QPushButton(this);
    goToSetBtn->setText(tr("Go to Settings"));
    goToSetBtn->setObjectName("goToSettings");
    goToSetBtn->setAccessibleName("netDisableDialog_goToSetButton");
    goToSetBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(0, goToSetBtn, false);

    DWarningButton *okBtn = new DWarningButton(this);
    okBtn->setText(tr("OK (%1)").arg(m_elapsedSeconds));
    okBtn->setObjectName("okBtn");
    okBtn->setAccessibleName("netDisableDialog_okButton");
    okBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(1, okBtn, true);

    // 添加信号槽
    connect(this, &DDialog::buttonClicked, this, [=](int index, const QString &text) {
        Q_UNUSED(text)

        if (index == NET_SETTING) {
            m_hmiScreen->showFuncConnectNetControl(m_sPkgName);
        }
        // 停止定时器、关闭弹框
        m_timer->stop();
        close();
    });

    // 安装定时器
    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, [=]() {
        m_elapsedSeconds--;

        // 实时变化按钮的时间
        if (m_elapsedSeconds > 0) {
            setButtonText(NET_OK, tr("OK (%1)").arg(m_elapsedSeconds));
            return;
        } else {
            // 关闭弹框
            close();
        }
    });
    m_timer->start(1000);
}

NetDisableDialog::~NetDisableDialog()
{
    Q_EMIT onDialogClose();

    m_timer->stop();
    m_timer->deleteLater();
}
