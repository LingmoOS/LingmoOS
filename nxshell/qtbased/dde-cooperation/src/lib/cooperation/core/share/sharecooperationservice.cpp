// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooconfig.h"
#include "sharecooperationservice.h"
#include "common/log.h"
#include "common/qtcompat.h"
#include "discover/deviceinfo.h"

#include <common/constant.h>
#include <common/commonutils.h>
#include <QDir>
#include <QFile>
#include <QHostInfo>
#include <QStandardPaths>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#endif

ShareCooperationService::ShareCooperationService(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    _expectedRunning = false;
    _brrierType = BarrierType::Server;   // default start as server.

    // init some config for barrier
    QSettings *settings = new QSettings();
    _cooConfig = new CooConfig(settings);
    QString ipName = QString::fromStdString(deepin_cross::CommonUitls::getFirstIp());
    _cooConfig->setScreenName(ipName);
}

ShareCooperationService::~ShareCooperationService()
{
    stopBarrier();
}

void ShareCooperationService::setBarrierType(BarrierType type)
{
    _brrierType = type;
}

void ShareCooperationService::setServerConfig(const DeviceInfoPointer selfDevice, const DeviceInfoPointer targetDevice)
{
    ShareServerConfig config;
    config.server_screen = selfDevice->ipAddress();
    config.client_screen = targetDevice->ipAddress();

    bool link = selfDevice->linkMode() == cooperation_core::DeviceInfo::LinkMode::RightMode;
    config.screen_left = link ? selfDevice->ipAddress() : targetDevice->ipAddress();
    config.screen_right = link ? targetDevice->ipAddress() : selfDevice->ipAddress();

    config.clipboardSharing = selfDevice->clipboardShared();
    setServerConfig(config);
}

BarrierType ShareCooperationService::barrierType() const
{
    return _brrierType;
}

bool ShareCooperationService::restartBarrier()
{
    stopBarrier();
    return startBarrier();
}

bool ShareCooperationService::setServerConfig(const ShareServerConfig &config)
{
    if (BarrierType::Server != _brrierType) {
        ELOG << "not the brrier server !!!!!!!";
        return false;
    }

    if (!checkParam(config))
        return false;
    auto path = configFilename();
    QFile file(path);
    if (!file.open(QFileDevice::OpenModeFlag::Truncate | QFileDevice::OpenModeFlag::WriteOnly)) {
        ELOG << "open server config error, path = " << path.toStdString() << ", case : "
             << file.errorString().toStdString();
        return false;
    }

    QTextStream outStream(&file);

    // 设置screen
    setScreen(config, &outStream);
    // 设置link
    setScreenLink(config, &outStream);
    // 设置options
    setScreenOptions(config, &outStream);
    outStream.flush();
    file.flush();
    file.close();

    return true;
}

bool ShareCooperationService::setClientTargetIp(const QString &screen, const QString &ip, const int &port)
{
    if (BarrierType::Server == _brrierType) {
        ELOG << "not the brrier client !!!!!!!";
        return false;
    }
    if (!_cooConfig) {
        ELOG << "the _cooConfig is null !!!!!"
             << " ip = " << ip.toStdString() << ":" << port;
        return false;
    }
    if (ip.isEmpty()) {
        ELOG << "error param !!!!!"
             << " ip = " << ip.toStdString() << ":" << port;
        return false;
    }

    _cooConfig->setServerIp(ip);
    _cooConfig->setPort(port == 0 ? UNI_SHARE_SERVER_PORT : port);
    return true;
}

bool ShareCooperationService::setClientTargetIp(const QString &ip)
{
    if (BarrierType::Server == _brrierType) {
        ELOG << "not the brrier client !!!!!!!";
        return false;
    }
    if (!_cooConfig) {
        ELOG << "the _cooConfig is null !!!!!"
             << " ip = " << ip.toStdString() << ":" << UNI_SHARE_SERVER_PORT;
        return false;
    }
    if (ip.isEmpty()) {
        ELOG << "error param !!!!!"
             << " ip = " << ip.toStdString() << ":" << UNI_SHARE_SERVER_PORT;
        return false;
    }

    _cooConfig->setServerIp(ip);
    _cooConfig->setPort(UNI_SHARE_SERVER_PORT);
    return true;
}

void ShareCooperationService::setEnableCrypto(bool enable)
{
    cooConfig().setCryptoEnabled(enable);
}

void ShareCooperationService::setBarrierProfile(const QString &dir)
{
    // check and create
    QDir pdir(dir);
    if (!pdir.exists()) {
        pdir.mkpath(pdir.absolutePath());
    }

    _cooConfig->setProfileDir(dir);
}

bool ShareCooperationService::startBarrier()
{
    LOG << "starting process";
    _expectedRunning = true;

    QString app;
    QStringList args;

    args << "-f"
         << "--no-tray"
         << "--debug" << cooConfig().logLevelText();

    args << "--name" << cooConfig().screenName();

    // set customize profile dir, which will include config and ssl ...
    QString profileDir = cooConfig().profileDir();
#if defined(Q_OS_WIN)
    // wrap in quotes in case username contains spaces.
    profileDir = QString("\"%1\"").arg(profileDir);
#endif
    args << "--profile-dir" << profileDir;

    if (!cooConfig().getCryptoEnabled()) {
        // default: false
        args << "--disable-crypto";
    }

    if ((barrierType() == BarrierType::Client && !clientArgs(args, app))
        || (barrierType() == BarrierType::Server && !serverArgs(args, app))) {
        stopBarrier();
        return false;
    }

    setBarrierProcess(new QProcess());
    connect(barrierProcess(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(barrierFinished(int, QProcess::ExitStatus)));
    connect(barrierProcess(), SIGNAL(readyReadStandardOutput()), this, SLOT(logOutput()));
    connect(barrierProcess(), SIGNAL(readyReadStandardError()), this, SLOT(logError()));

    LOG << "starting " << QString(barrierType() == BarrierType::Server ? "server" : "client").toStdString();
    LOG << QString("command: %1 %2").arg(app, args.join(" ")).toStdString();

    LOG << "log level: " << cooConfig().logLevelText().toStdString();
#if defined(Q_OS_WIN)
    QString winarg = args.join(" ");
    barrierProcess()->setNativeArguments(winarg);
    barrierProcess()->start(app);
#else
    barrierProcess()->start(app, args);
#endif

    if (!barrierProcess()->waitForStarted()) {
        ELOG << "Program can not be started: " << app.toStdString();
        return false;
    }

    return true;
}

const char ShutdownCh = 'S';
void ShareCooperationService::stopBarrier()
{
    // This may cause abort while app exit.
    // LOG << "stopping process";
    _expectedRunning = false;

    if (!barrierProcess()) {
        return;
    }

    if (barrierProcess()->isOpen()) {
        // try to shutdown child gracefully
        barrierProcess()->write(&ShutdownCh, 1);
#if defined(Q_OS_WIN)
        // it will freeze UI if aync wait on windows
        barrierProcess()->waitForFinished(100);
#else
        barrierProcess()->waitForFinished(5000);
#endif
        barrierProcess()->close();
    }

    delete barrierProcess();
    setBarrierProcess(nullptr);
}

bool ShareCooperationService::clientArgs(QStringList &args, QString &app)
{
    app = appPath(cooConfig().barriercName());

    if (!QFile::exists(app)) {
        WLOG << "Barrier client not found:" << app.toStdString();
        return false;
    }

#if defined(Q_OS_WIN)
    // wrap in quotes so a malicious user can't start \Program.exe as admin.
    app = QString("\"%1\"").arg(app);
#endif

    args << "[" + cooConfig().serverIp() + "]:" + QString::number(cooConfig().port());

    return true;
}

bool ShareCooperationService::serverArgs(QStringList &args, QString &app)
{
    app = appPath(cooConfig().barriersName());

    if (!QFile::exists(app)) {
        WLOG << "Barrier server not found:" << app.toStdString();
        return false;
    }

#if defined(Q_OS_WIN)
    // wrap in quotes so a malicious user can't start \Program.exe as admin.
    app = QString("\"%1\"").arg(app);
#endif

    args << "--address" << address();

    return true;
}

bool ShareCooperationService::checkParam(const ShareServerConfig &config)
{

    if (config.screen_left.isEmpty() || config.screen_right.isEmpty()) {
        WLOG << " config's screen left|right  empty!";
        return false;
    }

    return true;
}

void ShareCooperationService::setScreen(const ShareServerConfig &config, QTextStream *stream)
{
    // 设置screen
    *stream << "section: screens" << QT_ENDL;
    *stream << "\t" << config.screen_left << ":" << QT_ENDL;
    // 设置左边
    *stream << "\t\t"
            << "halfDuplexCapsLock = "
            << (config.left_halfDuplexCapsLock ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "halfDuplexNumLock = "
            << (config.left_halfDuplexNumLock ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "halfDuplexScrollLock = "
            << (config.left_halfDuplexScrollLock ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "xtestIsXineramaUnaware = "
            << (config.left_xtestIsXineramaUnaware ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "preserveFocus = "
            << (config.left_preserveFocus ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "switchCorners = "
            << (config.left_switchCorners.isEmpty() ? "none" : config.left_switchCorners) << QT_ENDL;
    *stream << "\t\t"
            << "switchCornerSize = " << config.left_switchCornerSize << QT_ENDL;
    // 设置右边
    *stream << "\t" << config.screen_right << ":" << QT_ENDL;
    *stream << "\t\t"
            << "halfDuplexCapsLock = "
            << (config.right_halfDuplexCapsLock ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "halfDuplexNumLock = "
            << (config.right_halfDuplexNumLock ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "halfDuplexScrollLock = "
            << (config.right_halfDuplexScrollLock ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "xtestIsXineramaUnaware = "
            << (config.right_xtestIsXineramaUnaware ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "preserveFocus = "
            << (config.right_preserveFocus ? "true" : "false") << QT_ENDL;
    *stream << "\t\t"
            << "switchCorners = "
            << (config.right_switchCorners.isEmpty() ? "none" : config.right_switchCorners) << QT_ENDL;
    *stream << "\t\t"
            << "switchCornerSize = " << config.right_switchCornerSize << QT_ENDL;
    *stream << "end" << QT_ENDL
            << QT_ENDL;

    *stream << "section: aliases" << QT_ENDL;
    *stream << "end" << QT_ENDL
            << QT_ENDL;
}

void ShareCooperationService::setScreenLink(const ShareServerConfig &config, QTextStream *stream)
{
    *stream << "section: links" << QT_ENDL;
    *stream << "\t" << config.screen_left << ":" << QT_ENDL;
    *stream << "\t\t"
            << "right = " << config.screen_right << QT_ENDL;
    *stream << "\t" << config.screen_right << ":" << QT_ENDL;
    *stream << "\t\t"
            << "left = " << config.screen_left << QT_ENDL;
    *stream << "end" << QT_ENDL
            << QT_ENDL;
}

void ShareCooperationService::setScreenOptions(const ShareServerConfig &config, QTextStream *stream)
{
    *stream << "section: options" << QT_ENDL;
    *stream << "\t"
            << "relativeMouseMoves = "
            << (config.relativeMouseMoves ? "true" : "false") << QT_ENDL;
    *stream << "\t"
            << "screenSaverSync = "
            << (config.screenSaverSync ? "true" : "false") << QT_ENDL;
    *stream << "\t"
            << "win32KeepForeground = "
            << (config.win32KeepForeground ? "true" : "false") << QT_ENDL;
    *stream << "\t"
            << "clipboardSharing = "
            << (config.clipboardSharing ? "true" : "false") << QT_ENDL;
    *stream << "\t"
            << "switchCorners = "
            << (config.switchCorners.isEmpty() ? "none" : config.switchCorners) << QT_ENDL;
    *stream << "\t"
            << "switchCornerSize = " << config.switchCornerSize << QT_ENDL;
    *stream << "end" << QT_ENDL
            << QT_ENDL;
}

QString ShareCooperationService::configFilename()
{
    QString configPath = cooConfig().profileDir() + "/" + cooConfig().configName();
    return configPath;
}

QString ShareCooperationService::address()
{
    QString address = cooConfig().networkInterface();
    if (!address.isEmpty())
        address = "[" + address + "]";
    return address + ":" + QString::number(cooConfig().port());
}

QString ShareCooperationService::appPath(const QString &name)
{
    return cooConfig().barrierProgramDir() + name;
}

void ShareCooperationService::barrierFinished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == 0) {
        LOG << "process exited normally";
        return;
    } else {
        ELOG << "process exited with error code: " << exitCode;
    }

    // auto restart if expect keep running
    if (_expectedRunning) {
#if defined(Q_OS_WIN)
        restartBarrier();
#else
        QTimer::singleShot(1000, this, SLOT(restartBarrier()));
#endif
        LOG << "detected process not running, auto restarting";
    }
}

void ShareCooperationService::appendLogRaw(const QString &text, bool error)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression regExp("\r|\n|\r\n");
#else
    QRegExp regExp("\r|\n|\r\n");
#endif
    for (QString line : text.split(regExp)) {
        if (!line.isEmpty()) {
            if (error) {
                ELOG << line.toStdString();
            } else {
                DLOG << line.toStdString();
            }
        }
    }
}

void ShareCooperationService::logOutput()
{
    if (_pBarrier) {
        appendLogRaw(_pBarrier->readAllStandardOutput(), false);
    }
}

void ShareCooperationService::logError()
{
    if (_pBarrier) {
        appendLogRaw(_pBarrier->readAllStandardError(), true);
    }
}
