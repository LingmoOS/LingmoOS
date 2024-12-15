// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitytoolsmodule.h"
#include "securitytoolswidget.h"
#include "startup/startupwidget.h"
#include "internetcontrol/netcontrolwidget.h"
#include "datausage/datausagemodel.h"
#include "datausage/datausagewidget.h"
#include "usbconnection/usbconnectionwork.h"
#include "usbconnection/usbconnectionwidget.h"
#include "loginsafety/loginsafetymodel.h"
#include "loginsafety/loginsafetywidget.h"
#include "window/modules/common/invokers/invokerfactory.h"
#include "window/modules/common/invokers/invokerinterface.h"

#include <QModelIndex>
#include <QStandardItemModel>

SecurityToolsModule::SecurityToolsModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frame)
    , m_securityToolsWidget(nullptr)
    , m_startupWidget(nullptr)
    , m_netControlWidget(nullptr)
    , m_dataUsageModel(nullptr)
    , m_dataUsageWidget(nullptr)
    , m_usbConnectionWork(nullptr)
    , m_usbConnectionWidget(nullptr)
    , m_loginSafetyModel(nullptr)
    , m_loginSafetyWidget(nullptr)
{
}

SecurityToolsModule::~SecurityToolsModule()
{
    deleteSunWidget();
}

void SecurityToolsModule::initialize()
{
}

void SecurityToolsModule::preInitialize()
{
    m_dataUsageModel = new DataUsageModel(new InvokerFactory(this), this);
    m_usbConnectionWork = new UsbConnectionWork(new InvokerFactory(this), this);

    m_loginSafetyModel = new LoginSafetyModel(new InvokerFactory(this), this);
}

const QString SecurityToolsModule::name() const
{
    return MODULE_SECURITY_TOOLS_NAME;
}

void SecurityToolsModule::active(int index)
{
    m_securityToolsWidget = new SecurityToolsWidget();
    m_securityToolsWidget->show();

    connect(m_securityToolsWidget, SIGNAL(notifyShowStartupWidget()), this, SLOT(showStartupWidget()));
    connect(m_securityToolsWidget, SIGNAL(notifyShowNetControlWidget()), this, SLOT(showNetControlWidget()));
    connect(m_securityToolsWidget, &SecurityToolsWidget::notifyShowDataUsageWidget, this,
            &SecurityToolsModule::showDataUsageWidget);
    connect(m_securityToolsWidget, &SecurityToolsWidget::notifyShowUsbControlWidget, this,
            &SecurityToolsModule::showUsbControlWidget);
    // 连接通知打开登录安全页信号
    connect(m_securityToolsWidget, &SecurityToolsWidget::notifyShowLoginSafetyWidget, this,
            &SecurityToolsModule::showLoginSafetyWidget);

    m_frameProxy->pushWidget(this, m_securityToolsWidget);

    switch (index) {
    case DATA_USAGE_INDEX: {
        showDataUsageWidget();
        break;
    }
    case STARTUP_CONTROL_INDEX: {
        showStartupWidget();
        break;
    }
    case NET_CONTROL_INDEX: {
        showNetControlWidget();
        break;
    }
    case USB_CONN_INDEX: {
        showUsbControlWidget();
        break;
    }
    case LOGIN_SAFETY_INDEX:
        showLoginSafetyWidget();
        break;
    default:
        break;
    }
}

void SecurityToolsModule::deactive()
{
}

void SecurityToolsModule::showDataUsageWidget()
{
    m_securityToolsWidget->hide();
    m_dataUsageWidget = new DataUsageWidget(m_dataUsageModel);
    m_frameProxy->pushWidget(this, m_dataUsageWidget);

    // 当此页面被删除之后，让指针置空
    connect(m_dataUsageWidget, &DataUsageWidget::destroyed, this, [this] {
        m_dataUsageWidget = nullptr;
    });
}

void SecurityToolsModule::showStartupWidget()
{
    m_securityToolsWidget->hide();
    m_startupWidget = new StartupWidget(new InvokerFactory(this));
    m_frameProxy->pushWidget(this, m_startupWidget);

    // 当此页面被删除之后，让指针置空
    connect(m_startupWidget, &StartupWidget::destroyed, this, [this] {
        m_startupWidget = nullptr;
    });
}

void SecurityToolsModule::showNetControlWidget()
{
    m_securityToolsWidget->hide();
    m_netControlWidget = new NetControlWidget("", new InvokerFactory(this));
    m_frameProxy->pushWidget(this, m_netControlWidget);

    // 当此页面被删除之后，让指针置空
    connect(m_netControlWidget, &NetControlWidget::destroyed, this, [this] {
        m_netControlWidget = nullptr;
    });
}

void SecurityToolsModule::showUsbControlWidget()
{
    m_securityToolsWidget->hide();
    m_usbConnectionWidget = new UsbConnectionWidget(m_usbConnectionWork);
    m_frameProxy->pushWidget(this, m_usbConnectionWidget);

    // 当此页面被删除之后，让指针置空
    connect(m_usbConnectionWidget, &UsbConnectionWidget::destroyed, this, [this] {
        m_usbConnectionWidget = nullptr;
    });
}

void SecurityToolsModule::showLoginSafetyWidget()
{
    m_securityToolsWidget->hide();
    m_loginSafetyWidget = new LoginSafetyWidget(m_loginSafetyModel);
    m_frameProxy->pushWidget(this, m_loginSafetyWidget);

    // 当此页面被删除之后，让指针置空
    connect(m_loginSafetyWidget, &LoginSafetyWidget::destroyed, this, [this] {
        m_loginSafetyWidget = nullptr;
    });
}

// 定位联网管控包名
void SecurityToolsModule::setNetControlIndex(const QString &sPkgName)
{
    m_netControlWidget->setNetControlIndex(sPkgName);
}

// 删除安全工具子页面
void SecurityToolsModule::deleteSunWidget()
{
    if (m_startupWidget != nullptr) {
        m_startupWidget->deleteLater();
        m_startupWidget = nullptr;
    }
    if (m_netControlWidget != nullptr) {
        m_netControlWidget->deleteLater();
        m_netControlWidget = nullptr;
    }
    if (m_dataUsageWidget != nullptr) {
        m_dataUsageWidget->deleteLater();
        m_dataUsageWidget = nullptr;
    }
    if (m_usbConnectionWidget != nullptr) {
        m_usbConnectionWidget->deleteLater();
        m_usbConnectionWidget = nullptr;
    }
    if (m_loginSafetyWidget != nullptr) {
        m_loginSafetyWidget->deleteLater();
        m_loginSafetyWidget = nullptr;
    }
}
