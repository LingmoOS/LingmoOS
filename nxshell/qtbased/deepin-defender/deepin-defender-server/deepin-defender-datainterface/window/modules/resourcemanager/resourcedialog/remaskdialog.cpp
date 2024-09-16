// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remaskdialog.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#include <DWidget>
#include <DLabel>
#include <DTitlebar>
#include <DWarningButton>

#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>
#include <QWindow>
#include <QHBoxLayout>
#include <QPushButton>

#define REM_ALLOW_ONCE 0 // 弹框第一个按钮
#define REM_ALLOW_ALWAYS 1 // 弹框第二个按钮
#define REM_DISABLE 2 // 弹框第三个按钮

static const int REM_ASK_TIMEOUT_COUNT = 15;
RemAskDialog::RemAskDialog(const QString &sPkgName, const QString &sAppName, const QString &sId, DDialog *parent)
    : DDialog(parent)
    , m_dataInterFaceServer(new DataInterFaceServer("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", QDBusConnection::sessionBus(), this))
    , m_monitorInterFaceServer(new MonitorInterFaceServer("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", QDBusConnection::systemBus(), this))
    , m_sAppName(sAppName)
    , m_sPkgName(sPkgName)
    , m_sId(sId)
    , m_timer(nullptr)
    , m_elapsedSeconds(REM_ASK_TIMEOUT_COUNT)
    , m_bButtonClickedFlag(false)
{
    this->setAccessibleName("remAskDialog");
    // 初始化界面
    initUi();
}

// 初始化界面
void RemAskDialog::initUi()
{
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
    txt = new DLabel(tr("%1 wants to start remote connection").arg(m_sAppName.isEmpty() ? m_sPkgName : m_sAppName));
    txt->setWordWrap(true);
    txt->setAccessibleName("remAskDialog_tipLable");

    // 字体剧中
    txt->setAlignment(Qt::AlignCenter);
    addContent(txt);

    // 针对DTK弹窗目前accessibleName会随系统语言变化的情况
    // 通过insertButton的形式定制按钮,保持accessiableName的一致性
    QPushButton *allowOnceBtn = new QPushButton(this);
    allowOnceBtn->setText(tr("Allow Once"));
    allowOnceBtn->setObjectName("allowOnceBtn");
    allowOnceBtn->setAccessibleName("remAskDialog_allowOnceButton");
    allowOnceBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(0, allowOnceBtn, false);

    QPushButton *allowAlwaysBtn = new QPushButton(this);
    allowAlwaysBtn->setText(tr("Allow Always"));
    allowAlwaysBtn->setObjectName("allowAlwaysBtn");
    allowAlwaysBtn->setAccessibleName("remAskDialog_allowAlwaysButton");
    allowAlwaysBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(1, allowAlwaysBtn, false);

    DWarningButton *disableBtn = new DWarningButton(this);
    disableBtn->setText(tr("Disable (%1)").arg(m_elapsedSeconds));
    disableBtn->setObjectName("disableBtn");
    disableBtn->setAccessibleName("remAskDialog_disableButton");
    disableBtn->setAttribute(Qt::WA_NoMousePropagation);
    this->insertButton(2, disableBtn, true);

    // 添加信号槽
    connect(this, &DDialog::buttonClicked, this, [=](int index, const QString &text) {
        m_bButtonClickedFlag = true;
        Q_UNUSED(text)

        if (index == REM_ALLOW_ONCE) {
            m_dataInterFaceServer->setRemRequestReply(m_sPkgName, m_sId, REM_ALLOW_ONCE);
        } else if (index == REM_ALLOW_ALWAYS) {
            m_dataInterFaceServer->setRemRequestReply(m_sPkgName, m_sId, REM_ALLOW_ALWAYS);
            m_monitorInterFaceServer->AddSecurityLog(SECURITY_LOG_TYPE_SAFY, tr("Changed default settings of remote connection to \"Allow always\""));
        } else {
            m_dataInterFaceServer->setRemRequestReply(m_sPkgName, m_sId, REM_DISABLE);
        }

        // 停止定时器、关闭弹框
        m_timer->stop();
        close();
    });

    // 点击关闭按钮触发禁止操作
    connect(this, &DDialog::closed, this, [=]() {
        if (!m_bButtonClickedFlag) {
            m_dataInterFaceServer->setRemRequestReply(m_sPkgName, m_sId, REM_DISABLE);
        }
    });

    // 安装定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [=]() {
        m_elapsedSeconds--;

        // 实时变化按钮的时间
        if (m_elapsedSeconds > 0) {
            setButtonText(REM_DISABLE, tr("Disable (%1)").arg(m_elapsedSeconds));
            return;
        } else {
            m_dataInterFaceServer->setRemRequestReply(m_sPkgName, m_sId, REM_DISABLE);
            // 关闭弹框
            close();
        }
    });
    m_timer->start(1000);
}

RemAskDialog::~RemAskDialog()
{
    Q_EMIT onDialogClose();

    m_timer->stop();
    m_timer->deleteLater();
}
