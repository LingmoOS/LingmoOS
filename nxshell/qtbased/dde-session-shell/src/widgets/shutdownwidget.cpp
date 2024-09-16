// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shutdownwidget.h"
#include "multiuserswarningview.h"
#include "../global_util/gsettingwatcher.h"
#include "../global_util/public_func.h"

#include <DConfig>

#if 0 // storage i10n
QT_TRANSLATE_NOOP("ShutdownWidget", "Shut down"),
QT_TRANSLATE_NOOP("ShutdownWidget", "Reboot"),
QT_TRANSLATE_NOOP("ShutdownWidget", "Suspend"),
QT_TRANSLATE_NOOP("ShutdownWidget", "Hibernate")
#endif

DCORE_USE_NAMESPACE

ShutdownWidget::ShutdownWidget(QWidget *parent)
    : QFrame(parent)
    , m_index(-1)
    , m_model(nullptr)
    , m_systemMonitor(nullptr)
    , m_switchosInterface(new HuaWeiSwitchOSInterface("com.huawei", "/com/huawei/switchos", QDBusConnection::sessionBus(), this))
    , m_dconfig(DConfig::create(getDefaultConfigFileName(), getDefaultConfigFileName(), QString(), this))
{
    m_frameDataBind = FrameDataBind::Instance();

    initUI();
    initConnect();

    onEnable("systemShutdown", enableState(GSettingWatcher::instance()->getStatus("systemShutdown")));
    onEnable("systemSuspend", enableState(GSettingWatcher::instance()->getStatus("systemSuspend")));
    onEnable("systemHibernate", enableState(GSettingWatcher::instance()->getStatus("systemHibernate")));
    onEnable("systemLock", enableState(GSettingWatcher::instance()->getStatus("systemLock")));

    std::function<void (QVariant)> function = std::bind(&ShutdownWidget::onOtherPageChanged, this, std::placeholders::_1);
    int index = m_frameDataBind->registerFunction("ShutdownWidget", function);

    connect(this, &ShutdownWidget::destroyed, this, [this, index] {
        m_frameDataBind->unRegisterFunction("ShutdownWidget", index);
    });
    installEventFilter(this);
}

ShutdownWidget::~ShutdownWidget()
{
    GSettingWatcher::instance()->erase("systemSuspend");
    GSettingWatcher::instance()->erase("systemHibernate");
    GSettingWatcher::instance()->erase("systemShutdown");
}

void ShutdownWidget::initConnect()
{
    connect(m_requireRestartButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireRestartButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, false);
    });
    connect(m_requireShutdownButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireShutdownButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
    });
    connect(m_requireSuspendButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireSuspendButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSuspend, false);
    });
    connect(m_requireHibernateButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireHibernateButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireHibernate, false);
    });
    connect(m_requireLockButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireLockButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireLock, false);
    });
    connect(m_requireSwitchUserBtn, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireSwitchUserBtn;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSwitchUser, false);
    });
    if (m_requireSwitchSystemBtn) {
        connect(m_requireSwitchSystemBtn, &RoundItemButton::clicked, this, [ = ] {
            m_currentSelectedBtn = m_requireSwitchSystemBtn;
            onRequirePowerAction(SessionBaseModel::PowerAction::RequireSwitchSystem, false);
        });
    }
    connect(m_requireLogoutButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireLogoutButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireLogout, false);
    });

    connect(GSettingWatcher::instance(), &GSettingWatcher::enableChanged, this, &ShutdownWidget::onEnable);

    if (m_systemMonitor) {
        connect(m_systemMonitor, &SystemMonitor::requestShowSystemMonitor, this, &ShutdownWidget::runSystemMonitor);
    }

    connect(m_dconfig, &DConfig::valueChanged, this, [this] (const QString &key) {
        if (key == "hideLogoutButton") {
            if (m_requireLogoutButton && m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
                m_requireLogoutButton->setVisible(!m_dconfig->value("hideLogoutButton", false).toBool());
            }
        }
    });
}

void ShutdownWidget::updateTr(RoundItemButton *widget, const QString &tr)
{
    m_trList << std::pair<std::function<void (QString)>, QString>(std::bind(&RoundItemButton::setText, widget, std::placeholders::_1), tr);
}

bool ShutdownWidget::enableState(const QString &gsettingsValue)
{
    if ("Disabled" == gsettingsValue)
        return false;
    else
        return true;
}

void ShutdownWidget::onEnable(const QString &gsettingsName, bool enable)
{
    if ("systemShutdown" == gsettingsName) {
        m_requireShutdownButton->setDisabled(!enable);
    } else if ("systemSuspend" == gsettingsName) {
        m_requireSuspendButton->setDisabled(!enable);
        m_requireSuspendButton->setCheckable(enable);
    } else if ("systemHibernate" == gsettingsName) {
        m_requireHibernateButton->setDisabled(!enable);
        m_requireHibernateButton->setCheckable(enable);
    } else if ("systemLock" == gsettingsName) {
        m_requireLockButton->setDisabled(!enable);
    }
}

void ShutdownWidget::onOtherPageChanged(const QVariant &value)
{
    m_index = value.toInt();

    for (auto it = m_btnList.constBegin(); it != m_btnList.constEnd(); ++it) {
        (*it)->updateState(RoundItemButton::Normal);
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);
}

void ShutdownWidget::hideToplevelWindow()
{
    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (widget->isVisible()) {
            widget->hide();
        }
    }
}

void ShutdownWidget::enterKeyPushed()
{
    if (m_systemMonitor && m_systemMonitor->state() == SystemMonitor::Enter) {
        runSystemMonitor();
        return;
    }

    if (!m_currentSelectedBtn->isEnabled())
        return;

    if (m_currentSelectedBtn == m_requireShutdownButton)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
    else if (m_currentSelectedBtn == m_requireRestartButton)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, false);
    else if (m_currentSelectedBtn == m_requireSuspendButton)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSuspend, false);
    else if (m_currentSelectedBtn == m_requireHibernateButton)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireHibernate, false);
    else if (m_currentSelectedBtn == m_requireLockButton)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireLock, false);
    else if (m_currentSelectedBtn == m_requireSwitchUserBtn)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSwitchUser, false);
    else if (m_currentSelectedBtn == m_requireSwitchSystemBtn)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSwitchSystem, false);
    else if (m_currentSelectedBtn == m_requireLogoutButton)
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireLogout, false);
}

void ShutdownWidget::enableHibernateBtn(bool enable)
{
    m_requireHibernateButton->setVisible(enable && (GSettingWatcher::instance()->getStatus("systemHibernate") != "Hiden"));
}

void ShutdownWidget::enableSleepBtn(bool enable)
{
    m_requireSuspendButton->setVisible(enable && (GSettingWatcher::instance()->getStatus("systemSuspend") != "Hiden"));
}

void ShutdownWidget::initUI()
{
    setFocusPolicy(Qt::StrongFocus);
    m_requireShutdownButton = new RoundItemButton(this);
    m_requireShutdownButton->setFocusPolicy(Qt::NoFocus);
    m_requireShutdownButton->setObjectName("RequireShutDownButton");
    m_requireShutdownButton->setAccessibleName("RequireShutDownButton");
    m_requireShutdownButton->setAutoExclusive(true);
    updateTr(m_requireShutdownButton, "Shut down");
    GSettingWatcher::instance()->bind("systemShutdown", m_requireShutdownButton);  // GSettings配置项

    m_requireRestartButton = new RoundItemButton(tr("Reboot"), this);
    m_requireRestartButton->setFocusPolicy(Qt::NoFocus);
    m_requireRestartButton->setObjectName("RequireRestartButton");
    m_requireRestartButton->setAccessibleName("RequireRestartButton");
    m_requireRestartButton->setAutoExclusive(true);
    updateTr(m_requireRestartButton, "Reboot");

    m_requireSuspendButton = new RoundItemButton(tr("Suspend"), this);
    m_requireSuspendButton->setFocusPolicy(Qt::NoFocus);
    m_requireSuspendButton->setObjectName("RequireSuspendButton");
    m_requireSuspendButton->setAccessibleName("RequireSuspendButton");
    m_requireSuspendButton->setAutoExclusive(true);
    updateTr(m_requireSuspendButton, "Suspend");
    GSettingWatcher::instance()->bind("systemSuspend", m_requireSuspendButton);  // GSettings配置项

    m_requireHibernateButton = new RoundItemButton(tr("Hibernate"), this);
    m_requireHibernateButton->setFocusPolicy(Qt::NoFocus);
    m_requireHibernateButton->setAccessibleName("RequireHibernateButton");
    m_requireHibernateButton->setObjectName("RequireHibernateButton");
    m_requireHibernateButton->setAutoExclusive(true);
    updateTr(m_requireHibernateButton, "Hibernate");
    GSettingWatcher::instance()->bind("systemHibernate", m_requireHibernateButton);  // GSettings配置项

    m_requireLockButton = new RoundItemButton(tr("Lock"));
    m_requireLockButton->setFocusPolicy(Qt::NoFocus);
    m_requireLockButton->setAccessibleName("RequireLockButton");
    m_requireLockButton->setObjectName("RequireLockButton");
    m_requireLockButton->setAutoExclusive(true);
    updateTr(m_requireLockButton, "Lock");
    GSettingWatcher::instance()->bind("systemLock", m_requireLockButton);  // GSettings配置项

    m_requireLogoutButton = new RoundItemButton(tr("Log out"));
    m_requireLogoutButton->setFocusPolicy(Qt::NoFocus);
    m_requireLogoutButton->setAccessibleName("RequireLogoutButton");
    m_requireLogoutButton->setObjectName("RequireLogoutButton");
    m_requireLogoutButton->setAutoExclusive(true);
    m_requireLogoutButton->setVisible(!m_dconfig->value("hideLogoutButton", false).toBool());
    updateTr(m_requireLogoutButton, "Log out");

    m_requireSwitchUserBtn = new RoundItemButton(tr("Switch user"));
    m_requireSwitchUserBtn->setFocusPolicy(Qt::NoFocus);
    m_requireSwitchUserBtn->setAccessibleName("RequireSwitchUserButton");
    m_requireSwitchUserBtn->setObjectName("RequireSwitchUserButton");
    m_requireSwitchUserBtn->setAutoExclusive(true);
    updateTr(m_requireSwitchUserBtn, "Switch user");
    m_requireSwitchUserBtn->setVisible(false);

    if (m_switchosInterface->isDualOsSwitchAvail()) {
        m_requireSwitchSystemBtn = new RoundItemButton(tr("Switch system"));
        m_requireSwitchSystemBtn->setAccessibleName("SwitchSystemButton");
        m_requireSwitchSystemBtn->setFocusPolicy(Qt::NoFocus);
        m_requireSwitchSystemBtn->setObjectName("RequireSwitchSystemButton");
        m_requireSwitchSystemBtn->setAutoExclusive(true);
        updateTr(m_requireSwitchSystemBtn, "Switch system");
    }

    m_btnList.append(m_requireShutdownButton);
    m_btnList.append(m_requireRestartButton);
    m_btnList.append(m_requireSuspendButton);
    m_btnList.append(m_requireHibernateButton);
    m_btnList.append(m_requireLockButton);
    m_btnList.append(m_requireSwitchUserBtn);
    if(m_requireSwitchSystemBtn) {
        m_btnList.append(m_requireSwitchSystemBtn);
    }
    m_btnList.append(m_requireLogoutButton);

    m_shutdownLayout = new QHBoxLayout;
    m_shutdownLayout->setMargin(0);
    m_shutdownLayout->setSpacing(10);
    m_shutdownLayout->addStretch();
    m_shutdownLayout->addWidget(m_requireShutdownButton);
    m_shutdownLayout->addWidget(m_requireRestartButton);
    m_shutdownLayout->addWidget(m_requireSuspendButton);
    m_shutdownLayout->addWidget(m_requireHibernateButton);
    m_shutdownLayout->addWidget(m_requireLockButton);
    m_shutdownLayout->addWidget(m_requireSwitchUserBtn);
    if(m_requireSwitchSystemBtn) {
        m_shutdownLayout->addWidget(m_requireSwitchSystemBtn);
    }
    m_shutdownLayout->addWidget(m_requireLogoutButton);
    m_shutdownLayout->addStretch(0);

    m_shutdownFrame = new QFrame;
    m_shutdownFrame->setAccessibleName("ShutdownFrame");
    m_shutdownFrame->setLayout(m_shutdownLayout);

    m_actionLayout = new QVBoxLayout;
    m_actionLayout->setMargin(0);
    m_actionLayout->setSpacing(10);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_shutdownFrame);
    m_actionLayout->setAlignment(m_shutdownFrame, Qt::AlignCenter);
    m_actionLayout->addStretch();

    if (findValueByQSettings<bool>(DDESESSIONCC::session_ui_configs, "Shutdown", "enableSystemMonitor", true)) {
        QFile file("/usr/bin/deepin-system-monitor");
        if (file.exists()) {
            m_systemMonitor = new SystemMonitor;
            m_systemMonitor->setAccessibleName("SystemMonitor");
            m_systemMonitor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            m_systemMonitor->setFocusPolicy(Qt::NoFocus);
            setFocusPolicy(Qt::StrongFocus);
            m_actionLayout->addWidget(m_systemMonitor);
            m_actionLayout->setAlignment(m_systemMonitor, Qt::AlignHCenter);
        }
    }

    m_actionFrame = new QFrame;
    m_actionFrame->setAccessibleName("ActionFrame");
    m_actionFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_actionFrame->setFocusPolicy(Qt::NoFocus);
    m_actionFrame->setLayout(m_actionLayout);

    m_mainLayout = new QStackedLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_actionFrame);
    m_mainLayout->setAlignment(m_actionFrame, Qt::AlignCenter);
    setLayout(m_mainLayout);

    updateStyle(":/skin/requireshutdown.qss", this);

    // refresh language
    for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
        it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
    }

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ShutdownWidget::leftKeySwitch()
{
    m_btnList.at(m_index)->updateState(RoundItemButton::Normal);
    if (m_index <= 0) {
        m_index = m_btnList.length() - 1;
    } else {
        m_index  -= 1;
    }

    for (int i = m_btnList.size(); i != 0; --i) {
        int index = (m_index + i) % m_btnList.size();

        if (m_btnList[index]->isVisible()) {
            m_index = index;
            break;
        }
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);

    m_frameDataBind->updateValue("ShutdownWidget", m_index);

    if (m_systemMonitor && m_systemMonitor->isVisible()) {
        m_systemMonitor->setState(SystemMonitor::Leave);
    }
}

void ShutdownWidget::rightKeySwitch()
{
    m_btnList.at(m_index)->updateState(RoundItemButton::Normal);

    if (m_index == m_btnList.size() - 1) {
        m_index = 0;
    } else {
        ++m_index;
    }

    for (int i = 0; i < m_btnList.size(); ++i) {
        int index = (m_index + i) % m_btnList.size();

        if (m_btnList[index]->isVisible()) {
            m_index = index;
            break;
        }
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);

    m_frameDataBind->updateValue("ShutdownWidget", m_index);

    if (m_systemMonitor && m_systemMonitor->isVisible()) {
        m_systemMonitor->setState(SystemMonitor::Leave);
    }
}

void ShutdownWidget::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    RoundItemButton *roundItemButton;
    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
        m_requireLockButton->setVisible(GSettingWatcher::instance()->getStatus("systemLock") != "Hiden");
        m_requireSwitchUserBtn->setVisible(m_switchUserEnable);
        if (m_requireSwitchSystemBtn) {
            m_requireSwitchSystemBtn->setVisible(true);
        }
        m_requireLogoutButton->setVisible(!m_dconfig->value("hideLogoutButton", false).toBool());
        roundItemButton = m_requireLockButton;
    } else {
        m_requireLockButton->setVisible(false);
        m_requireSwitchUserBtn->setVisible(false);
        if (m_requireSwitchSystemBtn) {
            m_requireSwitchSystemBtn->setVisible(false);
        }
        m_requireLogoutButton->setVisible(false);
        roundItemButton = m_requireShutdownButton;
    }

    int index = m_btnList.indexOf(roundItemButton);
    roundItemButton->updateState(RoundItemButton::Checked);
    m_frameDataBind->updateValue("ShutdownWidget", index);

    if (m_systemMonitor) {
        m_systemMonitor->setVisible(status == SessionBaseModel::ModeStatus::ShutDownMode);
    }
}

void ShutdownWidget::runSystemMonitor()
{
    auto launchProcessByAM = []() {
        QDBusMessage message = QDBusMessage::createMethodCall(
                "org.desktopspec.ApplicationManager1",
                "/org/desktopspec/ApplicationManager1/deepin_2dsystem_2dmonitor",
                "org.desktopspec.ApplicationManager1.Application",
                "Launch"
        );

        message << QString("") << QStringList() << QVariantMap();

        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() == QDBusMessage::ReplyMessage) {
            qDebug() << "Launch deepin-system-monitor successful!";
        } else {
            qWarning() << "Launch deepin-system-monitor main process error:" << reply.errorMessage();
        }
    };

    launchProcessByAM();

    if (m_systemMonitor) {
        m_systemMonitor->clearFocus();
        m_systemMonitor->setState(SystemMonitor::Leave);
    }

    hideToplevelWindow();
}

void ShutdownWidget::recoveryLayout()
{
    //关机或重启确认前会隐藏所有按钮,取消重启或关机后隐藏界面时重置按钮可见状态
    //同时需要判断切换用户按钮是否允许可见
    m_requireShutdownButton->setVisible(true && (GSettingWatcher::instance()->getStatus("systemShutdown") != "Hiden"));
    m_requireRestartButton->setVisible(true);
    enableHibernateBtn(m_model->hasSwap());
    enableSleepBtn(m_model->canSleep());
    m_requireLockButton->setVisible(true && (GSettingWatcher::instance()->getStatus("systemLock") != "Hiden"));
    m_requireSwitchUserBtn->setVisible(m_switchUserEnable);

    if (m_systemMonitor) {
        m_systemMonitor->setVisible(false);
    }

    m_mainLayout->setCurrentWidget(m_actionFrame);
    setFocusPolicy(Qt::StrongFocus);
}

void ShutdownWidget::onRequirePowerAction(SessionBaseModel::PowerAction powerAction, bool needConfirm)
{
    //锁屏或关机模式时，需要确认是否关机或检查是否有阻止关机
    if (m_model->appType() == Lock) {
        switch (powerAction) {
        case SessionBaseModel::PowerAction::RequireShutdown:
        case SessionBaseModel::PowerAction::RequireRestart:
        case SessionBaseModel::PowerAction::RequireSwitchSystem:
        case SessionBaseModel::PowerAction::RequireLogout:
        case SessionBaseModel::PowerAction::RequireSuspend:
        case SessionBaseModel::PowerAction::RequireHibernate:
            m_model->setIsCheckedInhibit(false);
            emit m_model->shutdownInhibit(powerAction, needConfirm);
            break;
        default:
            m_model->setPowerAction(powerAction);
            break;
        }
    } else {
        //登录模式直接操作
        m_model->setPowerAction(powerAction);
    }
}

void ShutdownWidget::setUserSwitchEnable(bool enable)
{
    //接收到用户列表变更信号号,记录切换用户是否允许可见,再根据当前是锁屏还是关机设置切换按钮可见状态
    if (m_switchUserEnable == enable) {
        return;
    }

    m_switchUserEnable = enable;
    m_requireSwitchUserBtn->setVisible(m_switchUserEnable && m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode);
}

bool ShutdownWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            if (m_systemMonitor && m_systemMonitor->isVisible() && m_currentSelectedBtn && m_currentSelectedBtn->isVisible()) {
                if (m_currentSelectedBtn->isChecked()) {
                    m_currentSelectedBtn->setChecked(false);
                    m_systemMonitor->setState(SystemMonitor::Enter);
                } else {
                    m_currentSelectedBtn->setChecked(true);
                    m_systemMonitor->setState(SystemMonitor::Leave);
                }
            }
            return true;
        }
    }
    return false;
}

void ShutdownWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        leftKeySwitch();
        break;
    case Qt::Key_Right:
        rightKeySwitch();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        enterKeyPushed();
        break;
    default:
        break;
    }

    QFrame::keyPressEvent(event);
}

bool ShutdownWidget::event(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
            it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
        }
    }

    if (e->type() == QEvent::FocusIn) {
        if (m_index < 0) {
            m_index = 0;
        }
        m_frameDataBind->updateValue("ShutdownWidget", m_index);
        m_btnList.at(m_index)->updateState(RoundItemButton::Checked);
    } else if (e->type() == QEvent::FocusOut) {
        m_btnList.at(m_index)->updateState(RoundItemButton::Normal);
    }

    return QFrame::event(e);
}

void ShutdownWidget::showEvent(QShowEvent *event)
{
    setFocus();
    QFrame::showEvent(event);
}

void ShutdownWidget::updateLocale(std::shared_ptr<User> user)
{
    Q_UNUSED(user)
    //只有登陆界面才需要根据系统切换语言
    if(qApp->applicationName() == "dde-lock") return;

    // refresh language
    for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
        it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
    }
}

void ShutdownWidget::setModel(SessionBaseModel *const model)
{
    m_model = model;

    connect(model, &SessionBaseModel::onRequirePowerAction, this, &ShutdownWidget::onRequirePowerAction);

    connect(model, &SessionBaseModel::onHasSwapChanged, this, &ShutdownWidget::enableHibernateBtn);
    enableHibernateBtn(model->hasSwap());

    connect(model, &SessionBaseModel::canSleepChanged, this, &ShutdownWidget::enableSleepBtn);
    connect(model, &SessionBaseModel::currentUserChanged, this, &ShutdownWidget::updateLocale);

    enableSleepBtn(model->canSleep());
}
