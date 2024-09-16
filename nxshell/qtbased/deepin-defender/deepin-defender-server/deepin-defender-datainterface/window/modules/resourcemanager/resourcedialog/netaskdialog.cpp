// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netaskdialog.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#include <DWidget>
#include <DLabel>
#include <DTitlebar>
#include <DWarningButton>

#include <QDesktopWidget>
#include <QApplication>
#include <QWindow>
#include <QHBoxLayout>
#include <QTimer>
#include <QPushButton>

#define NET_ALLOW_ONCE 0 // 弹框第一个按钮
#define NET_ALLOW_ALWAYS 1 // 弹框第二个按钮
#define NET_DISABLE 2 // 弹框第三个按钮

static const int NET_TIMEOUT_COUNT = 15;
NetAskDialog::NetAskDialog(const QString &sPkgName, const QString &sAppName, const QString &sId, DDialog *parent)
    : DDialog(parent)
    , m_dataInterFaceServer(new DataInterFaceServer("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", QDBusConnection::sessionBus(), this))
    , m_monitorInterFaceServer(new MonitorInterFaceServer("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_sAppName(sAppName)
    , m_sPkgName(sPkgName)
    , m_sId(sId)
    , m_timer(nullptr)
    , m_elapsedSeconds(NET_TIMEOUT_COUNT)
    , m_bButtonClickedFlag(false)
{  
    this->setAccessibleName("netAskDialog");
    // 初始化界面
    initUi();
}

// 初始化界面
void NetAskDialog::initUi()
{
    // 弹框样式
    setCloseButtonVisible(true);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setModal(true);

    setIcon(QIcon::fromTheme("deepin-defender"));

    // 添加布局
    DLabel *txt = nullptr;
    txt = new DLabel(tr("%1 wants to connect to the Internet").arg(m_sAppName.isEmpty() ? m_sPkgName : m_sAppName));
    txt->setWordWrap(true);
    txt->setAccessibleName("netAskDialog_tipLable");

    // 字体剧中
    txt->setAlignment(Qt::AlignCenter);
    addContent(txt);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    QPushButton *allowOnceBtn = new QPushButton(this);
    allowOnceBtn->setText(tr("Allow Once"));
    allowOnceBtn->setObjectName("allowOnceBtn");
    allowOnceBtn->setAccessibleName("netAskDialog_allowOnceButton");
    allowOnceBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(0, allowOnceBtn, false);

    QPushButton *allowAlwaysBtn = new QPushButton(this);
    allowAlwaysBtn->setText(tr("Allow Always"));
    allowAlwaysBtn->setObjectName("allowAlwaysBtn");
    allowAlwaysBtn->setAccessibleName("netAskDialog_allowAlwaysButton");
    allowAlwaysBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(1, allowAlwaysBtn, false);

    DWarningButton *disableBtn = new DWarningButton(this);
    disableBtn->setText(tr("Disable (%1)").arg(m_elapsedSeconds));
    disableBtn->setObjectName("disableBtn");
    disableBtn->setAccessibleName("netAskDialog_disableButton");
    disableBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(2, disableBtn, true);

    // 添加信号槽
    connect(this, &DDialog::buttonClicked, this, [=](int index, const QString &text) {
        m_bButtonClickedFlag = true;
        Q_UNUSED(text)

        if (index == NET_ALLOW_ONCE) {
            m_dataInterFaceServer->setNetRequestReply(m_sPkgName, m_sId, NET_ALLOW_ONCE);
        } else if (index == NET_ALLOW_ALWAYS) {
            m_dataInterFaceServer->setNetRequestReply(m_sPkgName, m_sId, NET_ALLOW_ALWAYS);
            m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_TOOL, tr("Changed Internet connection of %1 to \"Allow\"").arg(m_sAppName));
        } else {
            m_dataInterFaceServer->setNetRequestReply(m_sPkgName, m_sId, NET_DISABLE);
        }

        // 停止定时器、关闭弹框
        m_timer->stop();
        close();
    });

    // 点击关闭按钮触发禁止操作
    connect(this, &DDialog::closed, this, [=]() {
        if (!m_bButtonClickedFlag) {
            m_dataInterFaceServer->setNetRequestReply(m_sPkgName, m_sId, NET_DISABLE);
        }
    });

    // 安装定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [=]() {
        m_elapsedSeconds--;

        // 实时变化按钮的时间
        if (m_elapsedSeconds > 0) {
            setButtonText(NET_DISABLE, tr("Disable (%1)").arg(m_elapsedSeconds));
            return;
        } else {
            m_dataInterFaceServer->setNetRequestReply(m_sPkgName, m_sId, NET_DISABLE);
            // 关闭弹框
            close();
        }
    });
    m_timer->start(1000);
}

NetAskDialog::~NetAskDialog()
{
    Q_EMIT onDialogClose();

    m_timer->stop();
    m_timer->deleteLater();
}
