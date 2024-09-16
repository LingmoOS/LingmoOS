// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmanager.h"
#include "inhibitor.h"
#include "inhibitor1adaptor.h"
#include "utils/dconf.h"
#include "utils/utils.h"
#include "org_deepin_dde_Audio1_Sink.h"
#include "org_deepin_dde_SoundThemePlayer1.h"

#include <QCoreApplication>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QGSettings>
#include <QStandardPaths>
#include <QJsonDocument>

#include <unistd.h>
#include <signal.h>

#define MASK_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->MaskUnitFiles(QStringList() << service, true, true);\
    if (reply.isError()) {\
    qWarning() << "failed to mask service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define UNMASK_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->UnmaskUnitFiles(QStringList() << service, true);\
    if (reply.isError()) {\
    qWarning() << "failed to unmask service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define START_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->StartUnit(service, "replace");\
    if (reply.isError()) {\
    qWarning() << "failed to start service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define STOP_SERVICE(service) \
{\
    auto reply = m_systemd1ManagerInter->StopUnit(service, "replace");\
    if (reply.isError()) {\
    qWarning() << "failed to stop service: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    }

#define VIEW_SERVICE(service) \
{\
    QDBusPendingReply<QString> reply = m_systemd1ManagerInter->GetUnitFileState(service);\
    if (reply.isError()) {\
    qWarning() << "failed to get service state: " << service << ", error: " << reply.error().name();\
    return;\
    }\
    qDebug() << service << " state: " << reply.value();\
    }\

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_locked(false)
    , m_currentUid(QString::number(getuid()))
    , m_stage(0)
    , m_currentSessionPath("/")
    , m_powerInter(new org::deepin::dde::PowerManager1("org.deepin.dde.PowerManager1", "/org/deepin/dde/PowerManager1", QDBusConnection::systemBus(), this))
    , m_audioInter(new org::deepin::dde::Audio1("org.deepin.dde.Audio1", "/org/deepin/dde/Audio1", QDBusConnection::sessionBus(), this))
    , m_login1ManagerInter(new org::freedesktop::login1::Manager("org.freedesktop.login1", "/org/freedesktop/login1", QDBusConnection::systemBus(), this))
    , m_login1UserInter(new org::freedesktop::login1::User("org.freedesktop.login1", m_login1ManagerInter->GetUser(getuid()).value().path(), QDBusConnection::systemBus(), this))
    , m_login1SessionInter(new org::freedesktop::login1::Session("org.freedesktop.login1", m_login1UserInter->display().path.path(), QDBusConnection::systemBus(), this))
    , m_systemd1ManagerInter(new org::freedesktop::systemd1::Manager("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus(), this))
    , m_DBusInter(new org::freedesktop::DBus("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus(), this))
{
    initConnections();

    // 处理异常退出的情况
    handleOSSignal();

    // 播放登录提示音
    if (canPlayEvent("desktop-login")) {
        playLoginSound();
    }
}

SessionManager::~SessionManager()
{
    for (auto cookie : m_inhibitorMap.keys()) {
        auto ih = m_inhibitorMap.value(cookie);
        QDBusConnection::sessionBus().unregisterObject(ih->GetClientId().path());
        ih->deleteLater();
        ih = nullptr;
        m_inhibitorMap.remove(cookie);
    }
}

void SessionManager::initConnections()
{
    connect(m_login1SessionInter, &org::freedesktop::login1::Session::ActiveChanged, [=](bool active) {
        qDebug() << "session active status changed to:" << active;
        if (active) {
            // 变活跃
        } else {
            // 变不活跃
            bool isPreparingForSleep = m_login1ManagerInter->preparingForSleep();
            if (!isPreparingForSleep) {
                qDebug() << "call objLoginSessionSelf.Lock";
                m_login1SessionInter->Lock();
            }
        }
    });

    if (Dconf::GetValue("com.deepin.dde.startdde", "", "swap-sched-enabled", QVariant(false)).toBool()) {
        initSwapSched();
    }

    connect(m_DBusInter, &org::freedesktop::DBus::NameOwnerChanged,
            [ = ](const QString &name, const QString &oldOwner, const QString &newOwner) {
        if (newOwner == "" && oldOwner != "" && name == oldOwner && name.startsWith(":")) {
            // uniq name lost
        }
    });

    // signal和method同名，可以采用直接关联的方式完成目的
    QDBusConnection::systemBus().connect("org.freedesktop.login1", m_login1UserInter->display().path.path(), "org.freedesktop.login1.Session"
                                         , "Lock", this, SLOT(handleLoginSessionLocked()));
    QDBusConnection::systemBus().connect("org.freedesktop.login1", m_login1UserInter->display().path.path(), "org.freedesktop.login1.Session"
                                         , "Unlock", this, SLOT(handleLoginSessionUnlocked()));
}

void SessionManager::initSwapSched()
{
    qDebug() << "init swap sched";
    // TODO
}

void SessionManager::prepareLogout(bool force)
{
    if (!force)
        launchAutostopScripts();

    stopSogouIme();
    stopLangSelector();
    stopAtSpiService();
    stopBAMFDaemon();
    stopRedshift();
    stopObexService();
    stopDock();
    stopDesktop();

    // 防止注销时，蓝牙音频设置没有断开连接
    disconnectAudioDevices();

    // 强制注销时不播放音乐
    if (!force && canPlayEvent("desktop-logout")) {
        playLogoutSound();
    } else {
        stopPulseAudioService();
    }
}

void SessionManager::doLogout()
{
#ifndef QT_DEBUG
    QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");
    qInfo() << systemd.call("StartUnit", "dde-session-shutdown.target", "replace-irreversibly");
#else
    qInfo() << "logout completed, bug not now";
#endif
}

SessionManager *SessionManager::instance()
{
    static SessionManager instance;
    return &instance;
}

bool SessionManager::locked()
{
    return m_locked;
}

QString SessionManager::currentUid()
{
    return m_currentUid;
}

void SessionManager::setCurrentUid(QString uid)
{
    m_currentUid = uid;
}

Q_DECL_DEPRECATED int SessionManager::stage()
{
    return m_stage;
}

Q_DECL_DEPRECATED void SessionManager::setStage(int value)
{
    // TODO 待确定此属性的用途
    m_stage = value;
}

QDBusObjectPath SessionManager::currentSessionPath()
{
    m_currentSessionPath = m_login1UserInter->display().path;
    if (m_currentSessionPath.path().isEmpty()) {
        // 要保证 CurrentSessionPath 属性值的合法性
        m_currentSessionPath.setPath("/");
    }

    return m_currentSessionPath;
}

void SessionManager::setCurrentSessionPath(QDBusObjectPath value)
{
    m_currentSessionPath = value;
}

bool SessionManager::AllowSessionDaemonRun()
{
    // TODO: run org.deepin.dde.Daemon1.StartPart2
    return true;
}

bool SessionManager::CanHibernate()
{
    if (QString(getenv("POWER_CAN_SLEEP")) == "0") {
        return false;
    }

    // 是否支持休眠
    if (!m_powerInter->CanHibernate()) {
        return false;
    }

    // 当前能否休眠
    QString canHibernate = m_login1ManagerInter->CanHibernate();
    return canHibernate == "yes";
}

bool SessionManager::CanLogout()
{
    return true;
}

bool SessionManager::CanReboot()
{
    QString canReboot = m_login1ManagerInter->CanReboot();
    return canReboot == "yes";
}

bool SessionManager::CanShutdown()
{
    QString canPowerOff = m_login1ManagerInter->CanPowerOff();
    return canPowerOff == "yes";
}

bool SessionManager::CanSuspend()
{
    if (QString(getenv("POWER_CAN_SLEEP")) == "0") {
        return false;
    }

    // 是否支持待机
    if (!m_powerInter->CanSuspend()) {
        return false;
    }

    // 当前能否待机
    QString canSuspend = m_login1ManagerInter->CanSuspend();
    return canSuspend == "yes";
}

void SessionManager::ForceLogout()
{
    qDebug() << "force logout";

    prepareLogout(true);
    clearCurrentTty();
    doLogout();
}

void SessionManager::ForceReboot()
{
    reboot(true);
}

void SessionManager::ForceShutdown()
{
    shutdown(true);
}
/**
 * @brief SessionManager::GetInhibitors 遍历得到 Inhibitor 的路径列表，Inhibitor 是操作拦截器的意思，可以阻止一些由 flags 指定的操作。
 * @return ArrayofObjectPath: 返回一个包含 Inhibitors 路径名的数组
 */
QList<QDBusObjectPath> SessionManager::GetInhibitors()
{
    QList<QDBusObjectPath> list;
    for (auto ih : m_inhibitorMap.values()) {
        list << ih->GetClientId();
    }
    return list;
}

/**
 * @brief SessionManager::Inhibit 添加一个Inhibitor
 * @param appId: 发起操作的应用名
 * @param topLevelXid: 应用的上层ID
 * @param reason: 为什么添加Inhibit
 * @param flags: 标识符, 1 logout, 2 用户切换, 4 会话或系统休眠, 8 会话闲置
 * @return cookie: 返回inhibitor 的ID
 */
uint SessionManager::Inhibit(const QString &appId, uint topLevelXid, const QString &reason, uint flags)
{
    //  The flags parameter must include at least one of the following:
    //
    //    1: Inhibit logging out
    //    2: Inhibit user switching
    //    4: Inhibit suspending the session or computer
    //    8: Inhibit the session being marked as idle
    if (!(flags & 1 << 0)
            && !(flags & 1 << 1)
            && !(flags & 1 << 2)
            && !(flags & 1 << 3)){
        if (calledFromDBus())
            sendErrorReply(QDBusError::NotSupported, "flags not supported");
        return 0;
    }

    static uint index = 0;
    uint cookie = index;
    Inhibitor *ih = new Inhibitor(appId, topLevelXid, reason, flags, index++);
    new Inhibitor1Adaptor(ih);

    bool registered = QDBusConnection::sessionBus().registerObject(ih->GetClientId().path(), ih);
    if (!registered) {
        if (calledFromDBus())
            sendErrorReply(QDBusError::InvalidArgs, "export inhibit object failed");
        return cookie;
    }

    m_inhibitorMap.insert(cookie, ih);
    Q_EMIT InhibitorAdded(ih->GetClientId());
    return cookie;
}

/**
 * @brief SessionManager::IsInhibited 获取此 flag 代表的操作是否被拦截了。
 * @param flags: 传入的标识符
 * @return: flag 与Inhibitor中的交集存在true则返回,否则为false
 */
bool SessionManager::IsInhibited(uint flags)
{
    for (auto ih : m_inhibitorMap.values()) {
        if (ih->GetFlags() == flags)
            return true;
    }

    return false;
}

/**
 * @brief SessionManager::Uninhibit 根据id删除 Inhibitor,并取消在Dbus Session 上的注册,向Session Manager 发送信号
 * @param inhibitCookie: 待删除Inhibitor的id
 */
void SessionManager::Uninhibit(uint inhibitCookie)
{
    for (auto cookie : m_inhibitorMap.keys()) {
        if (cookie == inhibitCookie) {
            auto path = m_inhibitorMap.value(cookie)->GetClientId();
            QDBusConnection::sessionBus().unregisterObject(path.path());
            m_inhibitorMap.value(cookie)->deleteLater();
            m_inhibitorMap.remove(cookie);
            Q_EMIT InhibitorRemoved(path);
        }
    }
}

Q_DECL_DEPRECATED void SessionManager::Logout()
{
    Q_UNUSED(void());
}

Q_DECL_DEPRECATED void SessionManager::PowerOffChoose()
{
    Q_UNUSED(void());
}

Q_DECL_DEPRECATED void SessionManager::Reboot()
{
    Q_UNUSED(void())
}

bool SessionManager::Register(const QString &id)
{
    Q_UNUSED(id)
    return false;
}

void SessionManager::RequestHibernate()
{
    QDBusPendingReply<> reply = m_login1ManagerInter->Hibernate(false);
    if (reply.isError()) {
        qWarning() << "failed to hibernate, error: " << reply.error().name();
    }

    // NOTE: do we need it anymore?
    if (Dconf::SetValue("com.deepin.dde.startdde", "", "quick-black-screen", false)) {
        setDPMSMode(false);
    }
}

void SessionManager::RequestLock()
{
    QDBusInterface inter("org.deepin.dde.LockFront1", "/org/deepin/dde/LockFront1", "org.deepin.dde.LockFront1", QDBusConnection::sessionBus(), this);
    const QDBusMessage &msg = inter.call("Show");
    if (!msg.errorName().isEmpty()) {
        qWarning() << "failed to lock, error: " << msg.errorMessage();
    }
}

void SessionManager::RequestLogout()
{
    qDebug() << "request logout";

    prepareLogout(false);
    clearCurrentTty();
    doLogout();
}

void SessionManager::RequestReboot()
{
    reboot(false);
}

void SessionManager::RequestShutdown()
{
    shutdown(false);
}

void SessionManager::RequestSuspend()
{
    if (QFile("/etc/deepin/no_suspend").exists()) {
        QThread::sleep(1);
        setDPMSMode(false);
        return;
    }

    // 使用窗管接口进行黑屏处理
    if (Dconf::SetValue("com.deepin.dde.startdde", "", "quick-black-screen", QVariant(false))) {
        QDBusInterface inter("org.kde.KWin", "/BlackScreen", "org.kde.kwin.BlackScreen", QDBusConnection::sessionBus(), this);
        const QDBusMessage &msg = inter.call("setActive", true);
        if (!msg.errorName().isEmpty())
            qWarning() << "failed to create blackscreen, error: " << msg.errorName();
    }

    QDBusPendingReply<> reply = m_login1ManagerInter->Suspend(false);
    if (reply.isError()) {
        qWarning() << "failed to suspend, error:" << reply.error().name();
    }
}

void SessionManager::SetLocked(bool lock)
{
    // 仅允许dde-lock进程调用
    QString cmdLine = QString("/proc/%1/cmdline").arg(connection().interface()->servicePid(message().service()));
    QFile file(cmdLine);

    // NOTE: 如果以deepin-turbo进行加速启动，这里是不准确的，可能需要判断desktop文件的全路径，不过deepin-turbo后续应该会放弃支持
    if (!file.open(QIODevice::ReadOnly) || !file.readAll().startsWith("/usr/bin/dde-lock")) {
        qWarning() << "failed to get caller infomation or caller is illegal.";
        return;
    }

    m_locked = lock;
    emitLockChanged(m_locked);
}

Q_DECL_DEPRECATED void SessionManager::Shutdown()
{
    Q_UNUSED(void())
}

/**
 * @brief SessionManager::ToggleDebug
 * @note 允许在运行时打开debug日志输出，使得不用重启即可获取部分重要日志
 */
void SessionManager::ToggleDebug()
{
    QLoggingCategory::installFilter([] (QLoggingCategory *category) {
        category->setEnabled(QtDebugMsg, true);
    });
}

void SessionManager::prepareShutdown(bool force)
{
    stopSogouIme();
    stopBAMFDaemon();

    if (!force)
        preparePlayShutdownSound();

    stopPulseAudioService();
}

void SessionManager::clearCurrentTty()
{
    QDBusInterface inter("org.deepin.dde.Daemon1", "/org/deepin/dde/Daemon1", "org.deepin.dde.Daemon1", QDBusConnection::systemBus(), this);
    const QDBusMessage &msg = inter.call("ClearTty", m_login1SessionInter->vTNr());
    if (!msg.errorMessage().isEmpty())
        qWarning() << "failed to clear tty, error: " << msg.errorMessage();
}

void SessionManager::init()
{
    qInfo() << "session manager init";
    // TODO recover

    startAtSpiService();
    startObexService();

    qInfo() << "session manager init finished";
}

void SessionManager::stopSogouIme()
{
    // TODO 使用kill函数杀死进程，前提是进程存在
    EXEC_COMMAND("pkill"
                 , QStringList()
                 << "-ef"
                 << "sogouImeService");
}

void SessionManager::stopLangSelector()
{
    // TODO 待优化，可以跟随当前用户一同退出
    EXEC_COMMAND("pkill"
                 , QStringList()
                 << "-ef"
                 << "-u"
                 << QString::number(getuid()) << "/usr/lib/deepin-daemon/langselector"
                 );
}


void SessionManager::launchAutostopScripts()
{
    QStringList paths;
    paths.append(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()+ QDir::separator() + "autostop");
    paths.append("/etc/xdg/autostop");

    for (auto path : paths) {
        QDir scriptDir(path);
        const QFileInfoList &fileInfos = scriptDir.entryInfoList(QDir::Files);
        for (auto info : fileInfos) {
            qDebug() << "[autostop] script will be launched: " << info.absoluteFilePath();
            EXEC_COMMAND("/bin/bash", QStringList() << "-c" << info.absoluteFilePath());
        }
    }
}

void SessionManager::startAtSpiService()
{
    UNMASK_SERVICE(AT_SPI_SERVICE);
    START_SERVICE(AT_SPI_SERVICE);
    VIEW_SERVICE(AT_SPI_SERVICE);
}

void SessionManager::stopAtSpiService()
{
    MASK_SERVICE(AT_SPI_SERVICE);
    VIEW_SERVICE(AT_SPI_SERVICE);
}

void SessionManager::startObexService()
{
    UNMASK_SERVICE(OBEX_SERVICE);
    START_SERVICE(OBEX_SERVICE);
    VIEW_SERVICE(OBEX_SERVICE);
}

QString SessionManager::getAudioServerBackend()
{
    QStringList audioServers = {PULSEAUDIO_SERVICE, PIPEWIRE_SOCKET};
    QString currentAudioServer;

    for (auto server : audioServers) {
        QDBusPendingReply<QString> reply = m_systemd1ManagerInter->GetUnitFileState(server);
        if (!reply.isError() && (reply.value() == QStringLiteral("enabled"))) {
            currentAudioServer = server;
            break;
        }
    }

    return currentAudioServer;
}

void SessionManager::stopObexService()
{
    MASK_SERVICE(OBEX_SERVICE);
    VIEW_SERVICE(OBEX_SERVICE);
}

void SessionManager::stopPulseAudioService()
{
    QDBusInterface interface("org.deepin.dde.Audio1"
                              , "/org/deepin/dde/Audio1"
                              , "org.deepin.dde.Audio1"
                              , QDBusConnection::sessionBus(), this);
    interface.setTimeout(1000);
    auto msg = interface.call("NoRestartPulseAudio");
    if (!msg.errorMessage().isEmpty())
        qWarning() << "error: " << msg.errorMessage();

    const QString audioServer = getAudioServerBackend();

    STOP_SERVICE(audioServer);
    // pipewire also need to stop pipwire-pulse socket
    if (audioServer == PIPEWIRE_SOCKET) {
        STOP_SERVICE(PIPEWIRE_PULSE_SOCKET);
    }
    VIEW_SERVICE(audioServer);
}

void SessionManager::stopBAMFDaemon()
{
    STOP_SERVICE(BAMFDAEMON_SERVICE);
    VIEW_SERVICE(BAMFDAEMON_SERVICE);
}

void SessionManager::stopRedshift()
{
    STOP_SERVICE(REDSHIFT_SERVICE);
    VIEW_SERVICE(REDSHIFT_SERVICE);
}

void SessionManager::stopDock()
{
    STOP_SERVICE(DDE_DOCK_SERVICE);
    VIEW_SERVICE(DDE_DOCK_SERVICE);
}

void SessionManager::stopDesktop()
{
    STOP_SERVICE(DDE_DESKTOP_SERVICE);
    VIEW_SERVICE(DDE_DESKTOP_SERVICE);
}

void SessionManager::disconnectAudioDevices()
{
    auto msg = QDBusInterface("org.deepin.dde.Bluetooth1"
                              , "/org/deepin/dde/Bluetooth1"
                              , "org.deepin.dde.Bluetooth1"
                              , QDBusConnection::systemBus(), this)
            .call("DisconnectAudioDevices");
    if (!msg.errorMessage().isEmpty()) {
        qWarning() << "error: " << msg.errorMessage();
    } else {
        qDebug() << "success to disconnect audio devices";
    }
}

// 准备好关机时的音频文件
void SessionManager::preparePlayShutdownSound()
{
    const QString &soundFile = "/tmp/deepin-shutdown-sound.json";

    const QString &soundTheme = Utils::SettingValue(APPEARANCE_SCHEMA, QByteArray(), APPEARANCE_SOUND_THEME_KEY, QString()).toString();
    const QString &event = "system-shutdown";
    bool canPlay = canPlayEvent(event);
    QString device; // TODO

    QJsonObject obj;
    obj["Event"] = event;
    obj["CanPlay"] = canPlay;
    obj["Theme"] = soundTheme;
    obj["Device"] = device;

    QFile file(soundFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        file.close();
    } else {
        qWarning() << "failed to save shutdown sound config";
    }
}

bool SessionManager::canPlayEvent(const QString &event)
{
    if (event == SOUND_EFFECT_ENABLED_KEY || event == SOUND_EFFECT_PLAYER_KEY) {
        return false;
    }

    if (!Utils::SettingValue(SOUND_EFFECT_SCHEMA, QByteArray(), SOUND_EFFECT_ENABLED_KEY, false).toBool()) {
        return false;
    }

    if (!Utils::SettingValue(SOUND_EFFECT_SCHEMA, QByteArray(), event, false).toBool()) {
        return false;
    }

    return true;
}

void SessionManager::playLoginSound()
{
    const QString audioServer = getAudioServerBackend();
    UNMASK_SERVICE(audioServer);
    START_SERVICE(audioServer);

    const QString &tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty())
        return;

    // 存在此文件说明已经播放过
    const QString &markFile = tempDir + QDir::separator() + "startdde-login-sound-mark";
    if (QFile(markFile).exists())
        return;

    // 获取自动登录的账户
    auto getLightdmAutoLoginUser = [ = ] {
        QSettings settings("/etc/lightdm/lightdm.conf", QSettings::IniFormat);
        settings.beginGroup("Seat:*");
        if (!settings.allKeys().contains("autologin-user"))
            return QString();

        return settings.value("autologin-user").toString();
    };

    // 当前用户不是自动登录用户时，不需要播放
    const QString &autoLoginUser = getLightdmAutoLoginUser();
    if (autoLoginUser.isEmpty() || m_login1UserInter->name() != autoLoginUser)
        return;

    qInfo() << "play system sound: desktop-login";
    auto soundTheme = Utils::SettingValue(APPEARANCE_SCHEMA, QByteArray(), APPEARANCE_SOUND_THEME_KEY, QString()).toString();
    org::deepin::dde::SoundThemePlayer1 inter("org.deepin.dde.SoundThemePlayer1", "/org/deepin/dde/SoundThemePlayer1", QDBusConnection::systemBus(), this);
    QDBusPendingReply<> reply = inter.Play(soundTheme, "desktop-login", QString());
    if (reply.isError()) {
        qWarning() << "failed to play login sound, error: " << reply.error().name();
    } else {
        qDebug() << "success to play system sound: desktop-login";
    }

    // 播放后创建文件
    QFile file(markFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "failed to create mark file";
        return;
    }
    file.close();
}

void SessionManager::playLogoutSound()
{
    auto sink = m_audioInter->defaultSink();
    if (sink.path().isEmpty()) {
        qDebug() << "no default sink";
        stopPulseAudioService();
        return;
    }

    auto sinkInter = new org::deepin::dde::audio1::Sink(
                "org.deepin.dde.Audio1"
                , sink.path()
                , QDBusConnection::sessionBus()
                , this);
    if (sinkInter->mute()) {
        qDebug() << "default sink is mute";
        stopPulseAudioService();
        return;
    }

    // 始终在注销前退出pulseaudio服务
    stopPulseAudioService();

    auto soundTheme = Utils::SettingValue(APPEARANCE_SCHEMA, QByteArray(), APPEARANCE_SOUND_THEME_KEY, QString()).toString();
    org::deepin::dde::SoundThemePlayer1 inter("org.deepin.dde.SoundThemePlayer1", "/org/deepin/dde/SoundThemePlayer1", QDBusConnection::systemBus(), this);
    // TODO device?
    QDBusReply<void> reply = inter.Play(soundTheme, "desktop-logout", "");
    if (!reply.isValid()) {
        qWarning() << "failed to play logout sound, error: " << reply.error().name();
    }
}

void SessionManager::setDPMSMode(bool on)
{
    if (Utils::IS_WAYLAND_DISPLAY) {
        EXEC_COMMAND("dde_wldpms", QStringList() << "-s" << (on ? "On" : "Off"));
    } else {
        // TODO 可通过Xlib发送对应请求实现此功能
        EXEC_COMMAND("xset", QStringList() << "dpms" << "force" << (on ? "on" : "off"));
    }
}

/**
 * @brief sig_crash 意外退出时处理一些事情
 * @param sig 收到的异常退出信号类型
 */
[[noreturn]] void sig_crash(int sig) {
    Q_UNUSED(sig);
    SessionManager::instance()->doLogout();
    exit(-1);
};

void SessionManager::handleOSSignal()
{
    signal(SIGINT,  sig_crash);
    signal(SIGABRT, sig_crash);
    signal(SIGTERM, sig_crash);
    signal(SIGSEGV, sig_crash);
}

void SessionManager::shutdown(bool force)
{
    prepareShutdown(force);
    clearCurrentTty();

    QDBusPendingReply<> reply = m_login1ManagerInter->PowerOff(false);
    if (reply.isError()) {
        qWarning() << "failed to power off, error: " << reply.error().name();
    }

    //NOTE: do we need it anymore?
    if (Dconf::SetValue("com.deepin.dde.startdde", "", "quick-black-screen", false)) {
        setDPMSMode(false);
    }

    qApp->quit();
}

void SessionManager::reboot(bool force)
{
    prepareShutdown(force);
    clearCurrentTty();

    QDBusPendingReply<> reply = m_login1ManagerInter->Reboot(false);
    if (reply.isError()) {
        qWarning() << "failed to reboot, error: " << reply.error().name();
    }

    // NOTE: do we need it anymore?
    if (Dconf::SetValue("com.deepin.dde.startdde", "", "quick-black-screen", QVariant(false))) {
        setDPMSMode(false);
    }

    qApp->quit();
}

void SessionManager::emitLockChanged(bool locked)
{
   QDBusMessage msg = QDBusMessage::createSignal("/org/deepin/dde/SessionManager1", "org.freedesktop.DBus.Properties", "PropertiesChanged");
   QList<QVariant> arguments;
   arguments.push_back("org.deepin.dde.SessionManager1");
   QVariantMap changedProps;
   changedProps.insert("Locked", locked);
   arguments.push_back(changedProps);
   arguments.push_back(QStringList());
   msg.setArguments(arguments);
   QDBusConnection::sessionBus().send(msg);
}

void SessionManager::emitStageChanged(int state)
{
    Q_UNUSED(state);
    // TODO 可能这个属性已经没用了，确认一下就可以删了
}

void SessionManager::emitCurrentSessionPathChanged(QDBusObjectPath path)
{
    Q_UNUSED(path);
    // 这个属性不会变化
}

void SessionManager::emitCurrentUidChanged(QString uid)
{
    Q_UNUSED(uid);
     // 这个属性不会变化
}

void SessionManager::handleLoginSessionLocked()
{
    qDebug() << "login session locked." << locked();
    // 在特殊情况下，比如用 dde-switchtogreeter 命令切换到 greeter, 即切换到其他 tty
    // 前端(登录界面和锁屏界面)已绑定锁定信号进行了处理，此处只需更新Locked属性。

    // 如果已经锁定，则立即返回
    if (locked()) {
        qDebug() << "already locked";
        return;
    }

    m_locked = true;
    emitLockChanged(true);
}

void SessionManager::handleLoginSessionUnlocked()
{
    qDebug() << "login session unlocked.";

    bool sessionActive = m_login1SessionInter->active();
    if (sessionActive) {
        Q_EMIT Unlock();
    } else {
        // 图形界面 session 不活跃，此时采用 kill 掉锁屏前端的特殊操作，
        // 如果不 kill 掉它，则一旦 session 变活跃，锁屏前端会在很近的时间内执行锁屏和解锁，
        // 会使用系统通知报告锁屏失败。 而在此种特殊情况下 kill 掉它，并不会造成明显问题。
        do {
            org::freedesktop::DBus dbusInter("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus());
            QDBusPendingReply<QString> ownerReply = dbusInter.GetNameOwner("org.deepin.dde.LockFront1");
            if (ownerReply.isError()) {
                qWarning() << "failed to get lockFront service owner";
                return;
            }
            const QString &owner = ownerReply.value();

            QDBusPendingReply<uint> pidReply = dbusInter.GetConnectionUnixProcessID(owner);
            if (pidReply.isError()) {
                qWarning() << "failed to get lockFront service pid";
                return;
            }
            uint pid = pidReply.value();

            // 默认发送 SIGTERM, 礼貌的退出信号
            EXEC_COMMAND("kill", QStringList() << QString::number(pid));

            Q_EMIT Unlock();
        } while (false);

        m_locked = false;
        emitLockChanged(false);
    }
}
