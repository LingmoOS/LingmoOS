// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "appproxymodule.h"

#include <widgets/buttontuple.h>
#include <widgets/comboxwidget.h>
#include <widgets/lineeditwidget.h>
#include <widgets/widgetmodule.h>
#include <widgets/settingsgroupmodule.h>

#include <QLineEdit>
#include <QPushButton>

#include <DTipLabel>
#include <DLineEdit>
#include <QHostAddress>

#include <networkcontroller.h>
#include <proxycontroller.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE

const QStringList c_ProxyTypeList = {
    QStringLiteral("http"), QStringLiteral("socks4"), QStringLiteral("socks5")
};

AppProxyModule::AppProxyModule(QObject *parent)
    : PageModule("applicationProxy", tr("Application Proxy"), tr("Application Proxy"), QIcon::fromTheme("dcc_app_proxy"), parent)
{
    deactive();
    SettingsGroupModule *appProxyModuleGroup = new SettingsGroupModule("", tr(""));
    appProxyModuleGroup->appendChild(new WidgetModule<ComboxWidget>("app_proxy_type", tr("Proxy Type"), [this](ComboxWidget *proxyType) {
        m_proxyType = proxyType;
        proxyType->addBackground();
        proxyType->setTitle(tr("Proxy Type"));
        proxyType->comboBox()->addItems(c_ProxyTypeList);
        connect(proxyType, &ComboxWidget::onIndexChanged, this, [=] {
            m_btns->setEnabled(true);
        });
        auto updateProxyType = [proxyType](const AppProxyType &type) {
            proxyType->comboBox()->blockSignals(true);
            proxyType->setCurrentIndex(static_cast<int>(type));
            proxyType->comboBox()->blockSignals(false);
        };
        updateProxyType(NetworkController::instance()->proxyController()->appProxy().type);
        connect(NetworkController::instance()->proxyController(), &ProxyController::appTypeChanged, proxyType, updateProxyType);
        connect(this, &AppProxyModule::resetData, proxyType, [updateProxyType]() {
            updateProxyType(NetworkController::instance()->proxyController()->appProxy().type);
        });
    }));
    appProxyModuleGroup->appendChild(new WidgetModule<LineEditWidget>("app_proxy_ip", tr("IP Address"), [this](LineEditWidget *addr) {
        m_addr = addr;
        m_addr->addBackground();
        m_addr->setTitle(tr("IP Address"));
        m_addr->setPlaceholderText(tr("Required"));
        m_addr->textEdit()->installEventFilter(this);
        auto updateAddr = [addr](const QString &ip) {
            addr->textEdit()->blockSignals(true);
            addr->setText(ip);
            addr->textEdit()->blockSignals(false);
        };
        updateAddr(NetworkController::instance()->proxyController()->appProxy().ip);
        connect(NetworkController::instance()->proxyController(), &ProxyController::appIPChanged, addr, updateAddr);
        connect(this, &AppProxyModule::resetData, addr, [updateAddr]() {
            updateAddr(NetworkController::instance()->proxyController()->appProxy().ip);
        });
    }));
    appProxyModuleGroup->appendChild(new WidgetModule<LineEditWidget>("app_proxy_port", tr("Port"), [this](LineEditWidget *portWidget) {
        m_port = portWidget;
        portWidget->addBackground();
        portWidget->setTitle(tr("Port"));
        portWidget->setPlaceholderText(tr("Required"));
        portWidget->textEdit()->installEventFilter(this);
        auto updatePort = [portWidget](const uint &port) {
            portWidget->textEdit()->blockSignals(true);
            portWidget->setText(QString::number(port));
            portWidget->textEdit()->blockSignals(false);
        };
        updatePort(NetworkController::instance()->proxyController()->appProxy().port);
        connect(NetworkController::instance()->proxyController(), &ProxyController::appPortChanged, portWidget, updatePort);
        connect(this, &AppProxyModule::resetData, portWidget, [updatePort]() {
            updatePort(NetworkController::instance()->proxyController()->appProxy().port);
        });
    }));
    appProxyModuleGroup->appendChild(new WidgetModule<LineEditWidget>("app_proxy_username", tr("Username"), [this](LineEditWidget *username) {
        m_username = username;
        username->addBackground();
        username->setTitle(tr("Username"));
        username->setPlaceholderText(tr("Optional"));
        username->textEdit()->installEventFilter(this);
        auto updateUsername = [username](const QString &user) {
            username->textEdit()->blockSignals(true);
            username->setText(user);
            username->textEdit()->blockSignals(false);
        };
        updateUsername(NetworkController::instance()->proxyController()->appProxy().username);
        connect(NetworkController::instance()->proxyController(), &ProxyController::appUsernameChanged, username, updateUsername);
        connect(this, &AppProxyModule::resetData, username, [updateUsername]() {
            updateUsername(NetworkController::instance()->proxyController()->appProxy().username);
        });
    }));
    appProxyModuleGroup->appendChild(new WidgetModule<LineEditWidget>("app_proxy_password", tr("Password"), [this](LineEditWidget *password) {
        m_password = password;
        password->addBackground();
        password->setTitle(tr("Password"));
        password->setPlaceholderText(tr("Optional"));
        password->textEdit()->setEchoMode(QLineEdit::Password);
        m_password->textEdit()->setValidator(new QRegExpValidator(QRegExp("^\\S+$"), this));
        password->textEdit()->installEventFilter(this);
        auto updatePassword = [password](const QString &user) {
            password->textEdit()->blockSignals(true);
            password->setText(user);
            password->textEdit()->blockSignals(false);
        };
        updatePassword(NetworkController::instance()->proxyController()->appProxy().password);
        connect(NetworkController::instance()->proxyController(), &ProxyController::appPasswordChanged, password, updatePassword);
        connect(this, &AppProxyModule::resetData, password, [updatePassword]() {
            updatePassword(NetworkController::instance()->proxyController()->appProxy().password);
        });
    }));
    appendChild(appProxyModuleGroup);
    appendChild(new WidgetModule<DTipLabel>("app_proxy_tip", tr("Check \"Use a proxy\" in application context menu in Launcher after configured"), [](DTipLabel *tip) {
        tip->setText(tr("Check \"Use a proxy\" in application context menu in Launcher after configured"));
        tip->setWordWrap(true);
        tip->setAlignment(Qt::AlignLeft);
        tip->setMargin(8);
    }));
    ModuleObject *extra = new WidgetModule<ButtonTuple>("save", tr("Save", "button"), [this](ButtonTuple *btns) {
        m_btns = btns;
        m_btns->setButtonType(ButtonTuple::Save);
        m_btns->leftButton()->setText(tr("Cancel", "button"));
        m_btns->rightButton()->setText(tr("Save", "button"));
        m_btns->setEnabled(false);
        connect(m_btns->leftButton(), &QPushButton::clicked, m_btns, [this]() {
            m_btns->setEnabled(false);
            emit resetData();
        });
        connect(m_btns->rightButton(), &QPushButton::clicked, this, &AppProxyModule::onCheckValue);
    });
    extra->setExtra();
    appendChild(extra);
}

void AppProxyModule::deactive()
{
    m_proxyType = nullptr;
    m_addr = nullptr;
    m_port = nullptr;
    m_username = nullptr;
    m_password = nullptr;
    m_btns = nullptr;
}

void AppProxyModule::onCheckValue()
{
    m_addr->setIsErr(false);
    m_port->setIsErr(false);
    m_username->setIsErr(false);
    m_password->setIsErr(false);

    ProxyController *proxyController = NetworkController::instance()->proxyController();

    bool ok = true;
    const uint port = m_port->text().toUInt(&ok);
    // 如果地址和端口为0，删除配置文件
    if (m_addr->text().isEmpty() && ok && port == 0) {
        AppProxyConfig config;
        config.port = 0;
        config.ip.clear();

        m_btns->setEnabled(false);

        proxyController->setAppProxy(config);
        return;
    }

    const QString &addr = m_addr->text();
    if (addr.isEmpty() || !isIPV4(addr)) {
        m_addr->setIsErr(true);
        m_addr->dTextEdit()->showAlertMessage(tr("Invalid IP address"), m_addr, 2000);
        return;
    }

    if (!ok || port > 65535) {
        m_port->setIsErr(true);
        m_port->dTextEdit()->showAlertMessage(tr("Invalid port"), m_port, 2000);
        return;
    }

    const QString &username = m_username->text();
    const QString &password = m_password->text();

    AppProxyConfig config;
    config.type = static_cast<AppProxyType>(m_proxyType->comboBox()->currentIndex());
    config.ip = addr;
    config.port = port;
    config.username = username;
    config.password = password;

    m_btns->setEnabled(false);

    proxyController->setAppProxy(config);
}

bool AppProxyModule::eventFilter(QObject *watched, QEvent *event)
{
    // 实现鼠标点击编辑框，确定按钮激活，统一网络模块处理，捕捉FocusIn消息
    if (event->type() == QEvent::FocusIn) {
        if (dynamic_cast<QLineEdit *>(watched)) {
            m_btns->setEnabled(true);
        }
    }

    return ModuleObject::eventFilter(watched, event);
}

bool AppProxyModule::isIPV4(const QString &ipv4)
{
    QHostAddress ipAddr(ipv4);
    if (ipAddr == QHostAddress(QHostAddress::Null) || ipAddr == QHostAddress(QHostAddress::AnyIPv4)
        || ipAddr.protocol() != QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
        return false;
    }

    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    return regExpIP.exactMatch(ipv4);
}
