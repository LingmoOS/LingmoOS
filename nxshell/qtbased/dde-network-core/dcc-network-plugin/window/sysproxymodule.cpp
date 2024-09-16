// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sysproxymodule.h"

#include <widgets/buttontuple.h>
#include <widgets/comboxwidget.h>
#include <widgets/lineeditwidget.h>
#include <widgets/settingsgroup.h>
#include <widgets/switchwidget.h>
#include <widgets/widgetmodule.h>

#include <DFloatingButton>
#include <DFontSizeManager>
#include <DListView>
#include <DSwitchButton>
#include <DTextEdit>

#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

#include <networkcontroller.h>
#include <proxycontroller.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE
// 与m_ProxyMethodList对应
#define ProxyMethodIndex_Manual 0
#define ProxyMethodIndex_Auto 1

SysProxyModule::SysProxyModule(QObject *parent)
    : PageModule("systemProxy", tr("System Proxy"), tr("System Proxy"), QIcon::fromTheme("dcc_system_agent"), parent)
    , m_ProxyMethodList({ tr("Manual"), tr("Auto") })
    , m_uiMethod(dde::network::ProxyMethod::Init)
{
    deactive();
    m_modules.append(new WidgetModule<SwitchWidget>("system_proxy", tr("System Proxy"), [this](SwitchWidget *proxySwitch) {
        m_proxySwitch = proxySwitch;
        QLabel *lblTitle = new QLabel(tr("System Proxy"));
        DFontSizeManager::instance()->bind(lblTitle, DFontSizeManager::T5, QFont::DemiBold);
        proxySwitch->setLeftWidget(lblTitle);
        proxySwitch->switchButton()->setAccessibleName(lblTitle->text());

        auto updateSwitch = [proxySwitch]() {
            ProxyMethod method = NetworkController::instance()->proxyController()->proxyMethod();
            proxySwitch->blockSignals(true);
            proxySwitch->setChecked(method != ProxyMethod::None);
            proxySwitch->blockSignals(false);
        };
        updateSwitch();
        connect(NetworkController::instance()->proxyController(), &ProxyController::proxyMethodChanged, proxySwitch, updateSwitch);
        connect(proxySwitch, &SwitchWidget::checkedChanged, proxySwitch, [this](const bool checked) {
            m_buttonTuple->setEnabled(checked);
            if (checked) {
                uiMethodChanged(m_lastCombIndex == 0 ? ProxyMethod::Manual : ProxyMethod::Auto);
                m_proxyTypeBox->comboBox()->setCurrentIndex(m_lastCombIndex);
                checkConf();
            } else {
                // 关闭代理
                NetworkController::instance()->proxyController()->setProxyMethod(ProxyMethod::None);
                uiMethodChanged(ProxyMethod::None);
                m_buttonTuple->setVisible(false);
                m_buttonTuple->rightButton()->clicked();
            }
        });
    }));
    m_modules.append(new WidgetModule<ComboxWidget>("system_proxy_box", tr("System Proxy"), [this](ComboxWidget *proxyTypeBox) {
        m_proxyTypeBox = proxyTypeBox;
        proxyTypeBox->setTitle(tr("Proxy Type"));
        proxyTypeBox->addBackground();
        proxyTypeBox->comboBox()->addItems(m_ProxyMethodList);
        auto updateBox = [proxyTypeBox]() {
            ProxyMethod method = NetworkController::instance()->proxyController()->proxyMethod();
            proxyTypeBox->comboBox()->blockSignals(true);
            switch (method) {
            case ProxyMethod::Auto:
                proxyTypeBox->comboBox()->setCurrentIndex(ProxyMethodIndex_Auto);
                break;
            case ProxyMethod::Manual:
                proxyTypeBox->comboBox()->setCurrentIndex(ProxyMethodIndex_Manual);
                break;
            default:
                break;
            }
            proxyTypeBox->comboBox()->blockSignals(false);
        };
        updateBox();
        connect(NetworkController::instance()->proxyController(), &ProxyController::proxyMethodChanged, proxyTypeBox, updateBox);
        connect(proxyTypeBox->comboBox(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index) {
            switch (index) {
            case ProxyMethodIndex_Auto:
                uiMethodChanged(ProxyMethod::Auto);
                break;
            case ProxyMethodIndex_Manual:
                uiMethodChanged(ProxyMethod::Manual);
                break;
            }
            checkConf();
            m_buttonTuple->setEnabled(true);
        });
    }));
    m_modules.append(new WidgetModule<SettingsGroup>("system_proxy_auto_group", QString(), [this](SettingsGroup *autoGroup) {
        m_autoUrl = new LineEditWidget;
        m_autoUrl->setPlaceholderText(tr("Optional"));
        m_autoUrl->setTitle(tr("Configuration URL"));
        m_autoUrl->textEdit()->installEventFilter(this);
        ProxyController *proxyController = NetworkController::instance()->proxyController();
        connect(proxyController, &ProxyController::autoProxyChanged, m_autoUrl, &LineEditWidget::setText);
        connect(m_autoUrl->textEdit(), &QLineEdit::textChanged, this, &SysProxyModule::checkConf);
        autoGroup->appendItem(m_autoUrl);
        resetData(ProxyMethod::Auto);
    }));
    m_modules.append(new WidgetModule<QWidget>("system_proxy_manual_group", QString(), this, &SysProxyModule::initManualView));
    m_modules.append(new WidgetModule<QWidget>); // 加个空的保证下面有弹簧
    ModuleObject *extra = new WidgetModule<ButtonTuple>("save", tr("Save", "button"), [this](ButtonTuple *buttonTuple) {
        m_buttonTuple = buttonTuple;
        m_buttonTuple->setButtonType(ButtonTuple::Save);
        m_buttonTuple->leftButton()->setText(tr("Cancel", "button"));
        m_buttonTuple->rightButton()->setText(tr("Save", "button"));
        m_buttonTuple->setVisible(NetworkController::instance()->proxyController()->proxyMethod() != ProxyMethod::None);
        m_buttonTuple->setEnabled(false);
        checkConf();

        connect(m_buttonTuple->rightButton(), &QPushButton::clicked, this, [this] {
            m_lastCombIndex = m_proxyTypeBox->comboBox()->currentIndex();
            applySettings();
        });
        connect(m_buttonTuple->leftButton(), &QPushButton::clicked, this, [this]() {
            m_buttonTuple->setEnabled(false);
            if (m_proxyTypeBox->comboBox()->currentIndex() == ProxyMethodIndex_Auto)
                resetData(ProxyMethod::Auto);
            else
                resetData(ProxyMethod::Manual);
        });
    });
    extra->setExtra();
    m_modules.append(extra);
    NetworkController::instance()->proxyController()->querySysProxyData();
    uiMethodChanged(NetworkController::instance()->proxyController()->proxyMethod());
    connect(NetworkController::instance()->proxyController(), &ProxyController::proxyMethodChanged, this, &SysProxyModule::uiMethodChanged);
}

void SysProxyModule::active()
{
    NetworkController::instance()->proxyController()->querySysProxyData();
    uiMethodChanged(NetworkController::instance()->proxyController()->proxyMethod());
}

void SysProxyModule::deactive()
{
    m_proxySwitch = nullptr;
    m_proxyTypeBox = nullptr;
    m_autoUrl = nullptr;
    m_httpAddr = nullptr;
    m_httpPort = nullptr;
    m_httpsAddr = nullptr;
    m_httpsPort = nullptr;
    m_ftpAddr = nullptr;
    m_ftpPort = nullptr;
    m_socksAddr = nullptr;
    m_socksPort = nullptr;
    m_ignoreList = nullptr;
    m_buttonTuple = nullptr;
    QTimer::singleShot(1, this, [this]() {
        uiMethodChanged(NetworkController::instance()->proxyController()->proxyMethod());
    });
}

void SysProxyModule::initManualView(QWidget *w)
{
    // 手动代理编辑界面处理逻辑提取
    auto initProxyGroup = [this](QPointer<LineEditWidget> &proxyEdit, QPointer<LineEditWidget> &portEdit, const QString &proxyTitle, SettingsGroup *&group) {
        proxyEdit = new LineEditWidget(group);
        proxyEdit->setPlaceholderText(tr("Optional"));
        proxyEdit->setTitle(proxyTitle);
        proxyEdit->textEdit()->installEventFilter(this);
        portEdit = new LineEditWidget;
        portEdit->setPlaceholderText(tr("Optional"));
        portEdit->setTitle(tr("Port"));
        portEdit->textEdit()->installEventFilter(this);
        group->appendItem(proxyEdit);
        group->appendItem(portEdit);
        connect(proxyEdit->textEdit(), &QLineEdit::textChanged, this, [this] (const QString &text) {
            if (!text.isEmpty() && m_buttonTuple) {
                m_buttonTuple->setEnabled(true);
            }
        });
        connect(portEdit->textEdit(), &QLineEdit::textChanged, this, [portEdit](const QString &str) {
            if (str.toInt() < 0) {
                portEdit->setText("0");
            } else if (str.toInt() > 65535) {
                portEdit->setText("65535");
            }
        });
        connect(proxyEdit->textEdit(), &QLineEdit::textChanged, this, &SysProxyModule::checkConf);
    };

    //  手动代理编辑界面控件初始化
    SettingsGroup *httpGroup = new SettingsGroup();
    initProxyGroup(m_httpAddr, m_httpPort, tr("HTTP Proxy"), httpGroup);

    SettingsGroup *httpsGroup = new SettingsGroup();
    initProxyGroup(m_httpsAddr, m_httpsPort, tr("HTTPS Proxy"), httpsGroup);

    SettingsGroup *ftpGroup = new SettingsGroup();
    initProxyGroup(m_ftpAddr, m_ftpPort, tr("FTP Proxy"), ftpGroup);

    SettingsGroup *socksGroup = new SettingsGroup();
    initProxyGroup(m_socksAddr, m_socksPort, tr("SOCKS Proxy"), socksGroup);

    // 手动代理界面忽略主机编辑框初始化
    m_ignoreList = new DTextEdit(w);
    m_ignoreList->setAccessibleName("ProxyPage_ignoreList");
    m_ignoreList->installEventFilter(this);
    QLabel *ignoreTips = new QLabel(w);
    ignoreTips->setWordWrap(true);
    ignoreTips->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ignoreTips->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ignoreTips->setText(tr("Ignore the proxy configurations for the above hosts and domains"));

    // 手动代理界面布局
    QVBoxLayout *manualLayout = new QVBoxLayout(w);
    manualLayout->addWidget(httpGroup);
    manualLayout->addWidget(httpsGroup);
    manualLayout->addWidget(ftpGroup);
    manualLayout->addWidget(socksGroup);
    manualLayout->addWidget(m_ignoreList);
    manualLayout->addWidget(ignoreTips);
    manualLayout->setMargin(0);
    manualLayout->setSpacing(10);

    resetData(ProxyMethod::Manual);
    ProxyController *proxyController = NetworkController::instance()->proxyController();
    connect(m_ignoreList, &DTextEdit::textChanged, this, [this] {
        if (!m_ignoreList->toPlainText().isEmpty() && m_buttonTuple) {
            m_buttonTuple->setEnabled(true);
        }
    });
    connect(proxyController, &ProxyController::proxyIgnoreHostsChanged, m_ignoreList, [this](const QString &hosts) {
        const QTextCursor cursor = m_ignoreList->textCursor();
        m_ignoreList->blockSignals(true);
        m_ignoreList->setPlainText(hosts);
        m_ignoreList->setTextCursor(cursor);
        m_ignoreList->blockSignals(false);
    });

    connect(proxyController, &ProxyController::proxyChanged, m_httpAddr, [this]() {
        resetData(ProxyMethod::Manual);
    });
}

void SysProxyModule::applySettings()
{
    ProxyController *proxyController = NetworkController::instance()->proxyController();
    m_buttonTuple->setEnabled(false);
    if (!m_proxySwitch->checked()) {
        proxyController->setProxyMethod(ProxyMethod::None);
    } else if (m_proxyTypeBox->comboBox()->currentIndex() == ProxyMethodIndex_Manual) {
        proxyController->setProxy(SysProxyType::Http, m_httpAddr->text(), m_httpPort->text());
        proxyController->setProxy(SysProxyType::Https, m_httpsAddr->text(), m_httpsPort->text());
        proxyController->setProxy(SysProxyType::Ftp, m_ftpAddr->text(), m_ftpPort->text());
        proxyController->setProxy(SysProxyType::Socks, m_socksAddr->text(), m_socksPort->text());
        proxyController->setProxyIgnoreHosts(m_ignoreList->toPlainText());
        proxyController->setProxyMethod(ProxyMethod::Manual);
    } else if (m_proxyTypeBox->comboBox()->currentIndex() == ProxyMethodIndex_Auto) {
        proxyController->setAutoProxy(m_autoUrl->text());
        proxyController->setProxyMethod(ProxyMethod::Auto);
    }
}

void SysProxyModule::uiMethodChanged(ProxyMethod uiMethod)
{
    int i = 0;
    for (ModuleObject *module : m_modules) {
        if (module->name() == "system_proxy_box") {
            if (uiMethod != ProxyMethod::None)
                insertChild(i++, module);
            else
                removeChild(module);
        } else if (module->name() == "system_proxy_auto_group") {
            if (uiMethod == ProxyMethod::Auto)
                insertChild(i++, module);
            else
                removeChild(module);
        } else if (module->name() == "system_proxy_manual_group") {
            if (uiMethod == ProxyMethod::Manual)
                insertChild(i++, module);
            else
                removeChild(module);
        } else {
            insertChild(i++, module);
        }
    }
    if (m_buttonTuple)
        m_buttonTuple->setVisible(uiMethod != ProxyMethod::None);
}

void SysProxyModule::resetData(ProxyMethod uiMethod)
{
    ProxyController *proxyController = NetworkController::instance()->proxyController();
    switch (uiMethod) {
    case ProxyMethod::None:
        break;
    case ProxyMethod::Auto:
        m_autoUrl->setText(proxyController->autoProxy());
        break;
    case ProxyMethod::Manual: {
        auto onProxyChanged = [this](const SysProxyConfig &config) {
            switch (config.type) {
            case SysProxyType::Http:
                m_httpAddr->setText(config.url);
                m_httpPort->setText(QString::number(config.port));
                break;
            case SysProxyType::Https:
                m_httpsAddr->setText(config.url);
                m_httpsPort->setText(QString::number(config.port));
                break;
            case SysProxyType::Ftp:
                m_ftpAddr->setText(config.url);
                m_ftpPort->setText(QString::number(config.port));
                break;
            case SysProxyType::Socks:
                m_socksAddr->setText(config.url);
                m_socksPort->setText(QString::number(config.port));
                break;
            }
        };
        m_ignoreList->setPlainText(proxyController->proxyIgnoreHosts());
        static QList<SysProxyType> types = { SysProxyType::Ftp, SysProxyType::Http, SysProxyType::Https, SysProxyType::Socks };
        for (const SysProxyType &type : types) {
            SysProxyConfig config = proxyController->proxy(type);
            config.type = type;
            onProxyChanged(config);
        }
    } break;
    default:
        break;
    }
}

void SysProxyModule::checkConf()
{
    if (!m_proxyTypeBox || !m_buttonTuple)
        return;
    if (m_proxyTypeBox->comboBox()->currentIndex() == ProxyMethodIndex_Manual && m_httpAddr && m_httpsAddr && m_ftpAddr && m_socksAddr) {
        m_buttonTuple->rightButton()->setEnabled(!m_httpAddr->text().isEmpty() || !m_httpsAddr->text().isEmpty() || !m_ftpAddr->text().isEmpty() || !m_socksAddr->text().isEmpty());
    } else if (m_proxyTypeBox->comboBox()->currentIndex() == ProxyMethodIndex_Auto && m_autoUrl) {
        m_buttonTuple->rightButton()->setEnabled(!m_autoUrl->text().isEmpty());
    }
}

bool SysProxyModule::eventFilter(QObject *watched, QEvent *event)
{
    // 实现鼠标点击编辑框，确定按钮激活，捕捉FocusIn消息，DTextEdit没有鼠标点击消息
    if (event->type() == QEvent::FocusIn) {
        if (m_buttonTuple && ((dynamic_cast<QLineEdit *>(watched) || dynamic_cast<DTextEdit *>(watched))))
            m_buttonTuple->setEnabled(true);
    }

    return ModuleObject::eventFilter(watched, event);
}
