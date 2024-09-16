// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockcontent.h"

#include "base_module_interface.h"
#include "controlwidget.h"
#include "logowidget.h"
#include "mfa_widget.h"
#include "modules_loader.h"
#include "sessionbasemodel.h"
#include "sfa_widget.h"
#include "shutdownwidget.h"
#include "timewidget.h"
#include "userframelist.h"

#include <DDBusSender>

#include <QLocalSocket>
#include <QMouseEvent>

using namespace dss;
using namespace dss::module;

static const QString dsg_config = "org.desktopspec.ConfigManager";
static const QString localeName_key = "localeName";
static const QString longDateFormat_key = "longDateFormat";
static const QString shortTimeFormat_key = "shortTimeFormat";

LockContent::LockContent(SessionBaseModel *const model, QWidget *parent)
    : SessionBaseWindow(parent)
    , m_model(model)
    , m_controlWidget(new ControlWidget(m_model, this))
    , m_logoWidget(new LogoWidget(this))
    , m_timeWidget(new TimeWidget(this))
    , m_userListWidget(nullptr)
    , m_wmInter(new com::deepin::wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , m_sfaWidget(nullptr)
    , m_mfaWidget(nullptr)
    , m_authWidget(nullptr)
    , m_lockFailTimes(0)
    , m_localServer(new QLocalServer(this))
{
    if (m_model->appType() == AppType::Login) {
        // 管理员账户且密码过期的情况下，设置当前状态为ResetPasswdMode，从而方便直接跳转到重置密码界面
        bool showReset = m_model->currentUser()->expiredState() == User::ExpiredState::ExpiredAlready && m_model->currentUser()->accountType() == User::AccountType::Admin;
        m_model->setCurrentModeState(showReset ? SessionBaseModel::ModeStatus::ResetPasswdMode : SessionBaseModel::ModeStatus::PasswordMode);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    }

    initUI();
    initConnections();

    if (model->appType() == AppType::Lock) {
        setMPRISEnable(model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode);
    }

    QTimer::singleShot(0, this, [ = ] {
        onCurrentUserChanged(model->currentUser());
        onUserListChanged(model->isServerModel() ? model->loginedUserList() : model->userList());
    });

    m_localServer->setMaxPendingConnections(1);
    m_localServer->setSocketOptions(QLocalServer::WorldAccessOption);
    static bool once = false;
    if (!once) {
        // 将greeter和lock的服务名称分开
        // 如果服务相同，但是创建套接字文件的用户不一样，greeter和lock不能删除对方的套接字文件，造成锁屏无法监听服务。
        QString serverName = QString("GrabKeyboard_") + (m_model->appType() == Login ? "greeter" : ("lock_" + m_model->currentUser()->name()));
        // 将之前的server删除，如果是旧文件，即使监听成功，客户端也无法连接。
        QLocalServer::removeServer(serverName);
        if (!m_localServer->listen(serverName)) { // 监听特定的连接
            qWarning() << "listen failed!" << m_localServer->errorString();
        } else {
            qDebug() << "listen success!";
        }
    }
    once = true;

    QString localeName = regionValue(localeName_key);
    m_timeWidget->updateLocale(localeName);
}

void LockContent::initUI()
{
    m_timeWidget->setAccessibleName("TimeWidget");
    m_timeWidget->setVisible(false);            // 处理时间制跳转策略，获取到时间制再显示时间窗口
    setTopWidget(m_timeWidget);

    m_logoWidget->setAccessibleName("LogoWidget");
    setLeftBottomWidget(m_logoWidget);

    m_controlWidget->setAccessibleName("ControlWidget");
    setRightBottomWidget(m_controlWidget);

    if (m_model->getAuthProperty().MFAFlag) {
        initMFAWidget();
    } else {
        initSFAWidget();
    }
    m_authWidget->hide();

    initUserListWidget();
}

void LockContent::initConnections()
{
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &LockContent::onCurrentUserChanged);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, [ = ] (std::shared_ptr<User> user) {
        Q_EMIT requestEndAuthentication(m_model->currentUser()->name(), AT_All);
        Q_EMIT requestSwitchToUser(user);
    });
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, [ = ] {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
    });
    connect(m_controlWidget, &ControlWidget::requestTogglePopup, this, &SessionBaseWindow::togglePopup);
    connect(m_controlWidget, &ControlWidget::requestHidePopup, this, &SessionBaseWindow::hidePopup);

    //刷新背景单独与onStatusChanged信号连接，避免在showEvent事件时调用onStatusChanged而重复刷新背景，减少刷新次数
    connect(m_model, &SessionBaseModel::onStatusChanged, this, &LockContent::onStatusChanged);

    connect(m_model, &SessionBaseModel::userListChanged, this, &LockContent::onUserListChanged);
    connect(m_model, &SessionBaseModel::userListLoginedChanged, this, &LockContent::onUserListChanged);
    connect(m_model, &SessionBaseModel::authFinished, this, &LockContent::restoreMode);
    connect(m_model, &SessionBaseModel::MFAFlagChanged, this, [this](const bool isMFA) {
        isMFA ? initMFAWidget() : initSFAWidget();
        // 当前中间窗口为空或者中间窗口就是验证窗口的时候显示验证窗口
        if (!centerWidget() || centerWidget() == m_authWidget)
            setCenterContent(m_authWidget, Qt::AlignTop, m_authWidget->getTopSpacing());
    });

    connect(m_wmInter, &__wm::WorkspaceSwitched, this, &LockContent::currentWorkspaceChanged);
    connect(m_localServer, &QLocalServer::newConnection, this, &LockContent::onNewConnection);
}

/**
 * @brief 初始化多因认证界面
 */
void LockContent::initMFAWidget()
{
    qDebug() << "LockContent::initMFAWidget:" << m_sfaWidget << m_mfaWidget;
    if (m_sfaWidget) {
        m_sfaWidget->hide();
        delete m_sfaWidget;
        m_sfaWidget = nullptr;
    }
    if (m_mfaWidget) {
        m_authWidget = m_mfaWidget;
        return;
    }
    m_mfaWidget = new MFAWidget(this);
    m_mfaWidget->setModel(m_model);
    m_authWidget = m_mfaWidget;

    connect(m_mfaWidget, &MFAWidget::requestStartAuthentication, this, &LockContent::requestStartAuthentication);
    connect(m_mfaWidget, &MFAWidget::sendTokenToAuth, this, &LockContent::sendTokenToAuth);
    connect(m_mfaWidget, &MFAWidget::requestEndAuthentication, this, &LockContent::requestEndAuthentication);
    connect(m_mfaWidget, &MFAWidget::requestCheckAccount, this, &LockContent::requestCheckAccount);
}

/**
 * @brief 初始化单因认证界面
 */
void LockContent::initSFAWidget()
{
    qDebug() << "LockContent::initSFAWidget:" << m_sfaWidget << m_mfaWidget;
    if (m_mfaWidget) {
        m_mfaWidget->hide();
        delete m_mfaWidget;
        m_mfaWidget = nullptr;
    }
    if (m_sfaWidget) {
        m_authWidget = m_sfaWidget;
        return;
    }
    m_sfaWidget = new SFAWidget(this);
    m_sfaWidget->setModel(m_model);
    m_authWidget = m_sfaWidget;

    connect(m_sfaWidget, &SFAWidget::requestStartAuthentication, this, &LockContent::requestStartAuthentication);
    connect(m_sfaWidget, &SFAWidget::sendTokenToAuth, this, &LockContent::sendTokenToAuth);
    connect(m_sfaWidget, &SFAWidget::requestEndAuthentication, this, &LockContent::requestEndAuthentication);
    connect(m_sfaWidget, &SFAWidget::requestCheckAccount, this, &LockContent::requestCheckAccount);
    connect(m_sfaWidget, &SFAWidget::authFinished, this, &LockContent::authFinished);
    connect(m_sfaWidget, &SFAWidget::updateParentLayout, this, [this] {
        if (centerWidget() == m_sfaWidget)
            changeCenterSpaceSize(0, m_sfaWidget->getTopSpacing());
    });
}

/**
 * @brief 初始化用户列表界面
 */
void LockContent::initUserListWidget()
{
    if (m_userListWidget) {
        return;
    }
    m_userListWidget = new UserFrameList(this);
    m_userListWidget->setModel(m_model);
    m_userListWidget->setVisible(false);

    connect(m_userListWidget, &UserFrameList::clicked, this, &LockContent::restoreMode);
    connect(m_userListWidget, &UserFrameList::requestSwitchUser, this, &LockContent::requestSwitchToUser);
}

void LockContent::onValueChanged(const QDBusMessage &dbusMessage)
{
    QList<QVariant> arguments = dbusMessage.arguments();
    if (1 != arguments.count())
        return;

    QString interfaceName = dbusMessage.arguments().at(0).toString();
    if (interfaceName == localeName_key) {
        m_localeName = regionValue(localeName_key);
    } else if (interfaceName == shortTimeFormat_key) {
        m_shortTimeFormat = regionValue(shortTimeFormat_key);
    } else if (interfaceName == longDateFormat_key) {
        m_longDateFormat = regionValue(longDateFormat_key);
    }
    m_timeWidget->updateLocale(m_localeName, m_shortTimeFormat, m_longDateFormat);
}

QString LockContent::configPath(std::shared_ptr<User> user) const
{
    if (!user)
        return QString();

    QDBusInterface configInter(dsg_config, "/", "org.desktopspec.ConfigManager", QDBusConnection::systemBus());
    if (!configInter.isValid()) {
        qWarning("Can't acquire config manager. error:\"%s\"",
                 qPrintable(QDBusConnection::systemBus().lastError().message()));
        return QString();
    }
    QDBusReply<QDBusObjectPath> dbusReply = configInter.call("acquireManagerV2",
                                                             (uint)user->uid(),
                                                             QString(QCoreApplication::applicationName()),
                                                             QString("org.deepin.region-format"),
                                                             QString(""));
    if (configInter.lastError().isValid() ) {
        qWarning("Call failed: %s", qPrintable(configInter.lastError().message()));
        return QString();
    }
    return dbusReply.value().path();
}

QString LockContent::regionValue(const QString &key) const
{
    QString dbusPath = configPath(m_user);
    if (dbusPath.isEmpty())
        return QString();
    QDBusInterface managerInter(dsg_config, dbusPath, "org.desktopspec.ConfigManager.Manager", QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = managerInter.call("value", key);
    if (managerInter.lastError().isValid() ) {
        qWarning("Call failed: %s", qPrintable(managerInter.lastError().message()));
        return QString();
    }
    return reply.value().toString();
}

void LockContent::buildConnect()
{
    QString dbusPath = configPath(m_user);
    if (dbusPath.isEmpty())
        return;
    QDBusConnection::systemBus().connect(dsg_config, dbusPath, "org.desktopspec.ConfigManager.Manager",
                                         "valueChanged", "s", this,
                                         SLOT(onValueChanged(const QDBusMessage&)));
}

void LockContent::disconnect(std::shared_ptr<User> user)
{
    if (!user)
        return;

    QString dbusPath = configPath(user);
    if (dbusPath.isEmpty())
        return;
    QDBusConnection::systemBus().disconnect(dsg_config, dbusPath, "org.desktopspec.ConfigManager.Manager",
                                            "valueChanged", "s", this,
                                            SLOT(onValueChanged(const QDBusMessage&)));
}

void LockContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    if (!user)
        return;

    for (auto connect : m_currentUserConnects) {
        m_user.get()->disconnect(connect);
    }
    disconnect(m_user);
    m_currentUserConnects.clear();

    m_user = user;

    m_localeName = regionValue(localeName_key);
    QLocale locale = m_localeName.isEmpty()? user->locale() : QLocale(m_localeName);
    m_shortTimeFormat = regionValue(shortTimeFormat_key);
    m_longDateFormat = regionValue(longDateFormat_key);
    buildConnect();

    auto logoLocale = qApp->applicationName() == "dde-lock" ? QLocale::system().name() : user->locale();
    m_logoWidget->updateLocale(logoLocale);
    m_timeWidget->updateLocale(locale.name(), m_shortTimeFormat, m_longDateFormat);

    m_timeWidget->set24HourFormat(user->isUse24HourFormat());
    m_timeWidget->setWeekdayFormatType(user->weekdayFormat());
    m_timeWidget->setShortDateFormat(user->shortDateFormat());
    m_timeWidget->setShortTimeFormat(user->shortTimeFormat());

    m_currentUserConnects << connect(user.get(), &User::greeterBackgroundChanged, this, &LockContent::updateGreeterBackgroundPath, Qt::UniqueConnection)
                          << connect(user.get(), &User::use24HourFormatChanged, this, &LockContent::updateTimeFormat, Qt::UniqueConnection)
                          << connect(user.get(), &User::weekdayFormatChanged, m_timeWidget, &TimeWidget::setWeekdayFormatType)
                          << connect(user.get(), &User::shortDateFormatChanged, m_timeWidget, &TimeWidget::setShortDateFormat)
                          << connect(user.get(), &User::shortTimeFormatChanged, m_timeWidget, &TimeWidget::setShortTimeFormat);

    //TODO: refresh blur image
    QMetaObject::invokeMethod(this, [ = ] {
        updateTimeFormat(user->isUse24HourFormat());
    }, Qt::QueuedConnection);

    m_logoWidget->updateLocale(logoLocale);
}

void LockContent::pushPasswordFrame()
{
    setCenterContent(m_authWidget, Qt::AlignTop | Qt::AlignHCenter, m_authWidget->getTopSpacing());

    m_authWidget->syncResetPasswordUI();
}

void LockContent::pushUserFrame()
{
    if(m_model->isServerModel())
        m_controlWidget->setUserSwitchEnable(false);

    m_userListWidget->updateLayout();
    setCenterContent(m_userListWidget);
}

void LockContent::pushConfirmFrame()
{
    setCenterContent(m_authWidget, Qt::AlignTop | Qt::AlignHCenter, m_authWidget->getTopSpacing());
}

void LockContent::pushShutdownFrame()
{
    //设置关机选项界面大小为中间区域的大小,并移动到左上角，避免显示后出现移动现象
    m_shutdownFrame.reset(new ShutdownWidget(this));
    m_shutdownFrame->setAccessibleName("ShutdownFrame");
    m_shutdownFrame->setModel(m_model);
    m_shutdownFrame->move(0, 0);
    m_shutdownFrame->onStatusChanged(m_model->currentModeState());
    onUserListChanged(m_model->isServerModel() ? m_model->loginedUserList() : m_model->userList());
    setCenterContent(m_shutdownFrame.get());
}

void LockContent::setMPRISEnable(const bool state)
{
    if (!m_mediaWidget) {
        m_mediaWidget.reset(new MediaWidget(this));
        m_mediaWidget->setAccessibleName("MediaWidget");
        m_mediaWidget->initMediaPlayer();
    }

    m_mediaWidget->setVisible(state);
    setCenterBottomWidget(m_mediaWidget.get());
}

void LockContent::onNewConnection()
{
    if (!m_localServer->hasPendingConnections())
        return;

    // 重置密码程序启动连接成功锁屏界面才释放键盘，避免点击重置密码过程中使用快捷键切走锁屏
    if (window()->windowHandle() && window()->windowHandle()->setKeyboardGrabEnabled(false)) {
        qDebug() << "setKeyboardGrabEnabled(false) success！";
    }

    QLocalSocket *socket = m_localServer->nextPendingConnection();
    connect(socket, &QLocalSocket::disconnected, this, &LockContent::onDisConnect);
    connect(socket, &QLocalSocket::readyRead, this, [socket, this] {
        auto content = socket->readAll();
        if (content == "close") {
            m_sfaWidget->syncPasswordResetPasswordVisibleChanged(QVariant::fromValue(true));
            m_sfaWidget->syncResetPasswordUI();
        }
    });
}

void LockContent::onDisConnect()
{
    tryGrabKeyboard();
}

void LockContent::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    refreshLayout(status);

    switch (status) {
    case SessionBaseModel::ModeStatus::PasswordMode:
        pushPasswordFrame();
        break;
    case SessionBaseModel::ModeStatus::ConfirmPasswordMode:
        pushConfirmFrame();
        break;
    case SessionBaseModel::ModeStatus::UserMode:
        pushUserFrame();
        break;
    case SessionBaseModel::ModeStatus::PowerMode:
    case SessionBaseModel::ModeStatus::ShutDownMode:
        pushShutdownFrame();
        break;
    default:
        break;
    }

    m_model->setAbortConfirm(status == SessionBaseModel::ModeStatus::ConfirmPasswordMode);
    if (status != SessionBaseModel::ModeStatus::ConfirmPasswordMode)
        m_model->setPowerAction(SessionBaseModel::PowerAction::None);
}

void LockContent::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果是重置密码界面，不做处理
    if (m_model->currentModeState() == SessionBaseModel::ResetPasswdMode)
        return SessionBaseWindow::mouseReleaseEvent(event);

    //在关机界面没有点击按钮直接点击界面时，直接隐藏关机界面
    if (m_model->currentModeState() == SessionBaseModel::ShutDownMode) {
        hideToplevelWindow();
    }

    if (m_model->currentModeState() == SessionBaseModel::UserMode
            || m_model->currentModeState() == SessionBaseModel::PowerMode) {
        // 触屏点击空白处不退出用户列表界面
        if (event->source() == Qt::MouseEventSynthesizedByQt)
            return SessionBaseWindow::mouseReleaseEvent(event);

        // 点击空白处的时候切换到当前用户，恢复对应状态（如密码过期）并开启验证。
        restoreMode();
        emit requestSwitchToUser(m_model->currentUser());
    }

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    return SessionBaseWindow::mouseReleaseEvent(event);
}

void LockContent::showEvent(QShowEvent *event)
{
    SessionBaseWindow::showEvent(event);

    onStatusChanged(m_model->currentModeState());
    tryGrabKeyboard();
}

void LockContent::hideEvent(QHideEvent *event)
{
    if (!m_shutdownFrame.isNull())
        m_shutdownFrame->recoveryLayout();
    // explicitly hide popup in case fake window layer is shown next time
    hidePopup();
    return SessionBaseWindow::hideEvent(event);
}

void LockContent::resizeEvent(QResizeEvent *event)
{
    if (SessionBaseModel::PasswordMode == m_model->currentModeState() || (SessionBaseModel::ConfirmPasswordMode == m_model->currentModeState())) {
        changeCenterSpaceSize(0, m_authWidget->getTopSpacing());
    }

    SessionBaseWindow::resizeEvent(event);
    m_userListWidget->setMaximumSize(getCenterContentSize());
}

void LockContent::restoreMode()
{
    if (m_model->appType() == AppType::Login) {
        // 管理员账户且密码过期的情况下，设置当前状态为ResetPasswdMode，从而方便直接跳转到重置密码界面
        bool showReset = m_model->currentUser()->expiredState() == User::ExpiredState::ExpiredAlready && m_model->currentUser()->accountType() == User::AccountType::Admin;
        m_model->setCurrentModeState(showReset ? SessionBaseModel::ModeStatus::ResetPasswdMode : SessionBaseModel::ModeStatus::PasswordMode);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
    }
}

void LockContent::updateGreeterBackgroundPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    if (m_model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode) {
        emit requestBackground(path);
    }
}

void LockContent::updateDesktopBackgroundPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
        emit requestBackground(path);
    }
}

void LockContent::updateTimeFormat(bool use24)
{
    if (m_user != nullptr) {
        m_timeWidget->set24HourFormat(use24);
        m_timeWidget->setVisible(true);
    }
}

void LockContent::toggleModule(const QString &name)
{
    BaseModuleInterface *module = ModulesLoader::instance().findModuleByName(name);
    if (!module) {
        return;
    }

    switch (module->type()) {
    case BaseModuleInterface::LoginType:
        setCenterContent(module->content());
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        break;
    case BaseModuleInterface::TrayType:
        return;
    }
}

void LockContent::onUserListChanged(QList<std::shared_ptr<User> > list)
{
    const bool allowShowUserSwitchButton = m_model->allowShowUserSwitchButton();
    const bool alwaysShowUserSwitchButton = m_model->alwaysShowUserSwitchButton();
    bool haveLogindUser = true;

    if (m_model->isServerModel() && m_model->appType() == Login) {
        haveLogindUser = !m_model->loginedUserList().isEmpty();
    }

    bool enable = (alwaysShowUserSwitchButton ||
                   (allowShowUserSwitchButton &&
                    (list.size() > (m_model->isServerModel() ? 0 : 1)))) &&
            haveLogindUser;

    m_controlWidget->setUserSwitchEnable(enable);

    if (m_shutdownFrame)
        m_shutdownFrame->setUserSwitchEnable(enable);
}

void LockContent::tryGrabKeyboard()
{
#ifndef QT_DEBUG
    if (m_model->isUseWayland()) {
        return;
    }

    if (window()->windowHandle() && window()->windowHandle()->setKeyboardGrabEnabled(true)) {
        m_lockFailTimes = 0;
        return;
    }

    m_lockFailTimes++;

    if (m_lockFailTimes == 15) {
        qWarning() << "Trying grabkeyboard has exceeded the upper limit. dde-lock will quit.";

        m_lockFailTimes = 0;

        DDBusSender()
                .service("org.freedesktop.Notifications")
                .path("/org/freedesktop/Notifications")
                .interface("org.freedesktop.Notifications")
                .method(QString("Notify"))
                .arg(tr("Lock Screen"))
                .arg(static_cast<uint>(0))
                .arg(QString(""))
                .arg(QString(""))
                .arg(tr("Failed to lock screen"))
                .arg(QStringList())
                .arg(QVariantMap())
                .arg(5000)
                .call();

        emit requestLockFrameHide();
        return;
    }

    QTimer::singleShot(100, this, &LockContent::tryGrabKeyboard);
#endif
}

void LockContent::hideToplevelWindow()
{
    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (widget->isVisible()) {
            widget->hide();
        }
    }
}

void LockContent::currentWorkspaceChanged()
{
    QDBusPendingCall call = m_wmInter->GetCurrentWorkspaceBackgroundForMonitor(QGuiApplication::primaryScreen()->name());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [ = ] {
        if (!call.isError()) {
            QDBusReply<QString> reply = call.reply();
            updateWallpaper(reply.value());
        } else {
            qWarning() << "get current workspace background error: " << call.error().message();
            updateWallpaper("/usr/share/backgrounds/deepin/desktop.jpg");
        }

        watcher->deleteLater();
    });
}

void LockContent::updateWallpaper(const QString &path)
{
    const QUrl url(path);
    QString wallpaper = path;
    if (url.isLocalFile()) {
        wallpaper = url.path();
    }

    updateDesktopBackgroundPath(wallpaper);
}

void LockContent::refreshBackground(SessionBaseModel::ModeStatus status)
{
    Q_UNUSED(status)

    // 根据当前状态刷新不同的背景
    auto user = m_model->currentUser();
    if (user != nullptr) {
        emit requestBackground(user->greeterBackground());
    }
}

void LockContent::refreshLayout(SessionBaseModel::ModeStatus status)
{
    setTopFrameVisible(status != SessionBaseModel::ModeStatus::ShutDownMode);
    setBottomFrameVisible(status != SessionBaseModel::ModeStatus::ShutDownMode);
}

void LockContent::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_mfaWidget) {
            m_mfaWidget->autoUnlock();
        }
        break;
    case Qt::Key_Escape:
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ConfirmPasswordMode) {
            m_model->setAbortConfirm(false);
            m_model->setPowerAction(SessionBaseModel::PowerAction::None);
        } else if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
            hideToplevelWindow();
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        } else if (m_model->currentModeState() == SessionBaseModel::ModeStatus::PowerMode) {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        }
        break;
    }
}
