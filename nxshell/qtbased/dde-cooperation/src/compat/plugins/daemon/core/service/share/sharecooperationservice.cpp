// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecooperationservice.h"
#include "service/comshare.h"
#include <utils/config.h>
#include <utils/cooconfig.h>

#include <QFile>
#include <QTimer>

#include <utils/utils.h>
#include <co/tasked.h>

ShareCooperationService::ShareCooperationService(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");

    _expectedRunning = false;
    _brrierType = BarrierType::Server; // default start as server.

    QSettings *settings = DaemonConfig::instance()->settings();
    _cooConfig = new CooConfig(settings);
}

ShareCooperationService::~ShareCooperationService()
{
    stopBarrier();
}

void ShareCooperationService::setBarrierType(BarrierType type)
{
    _brrierType = type;
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
    if (!config.server_screen.empty())
        _cooConfig->setScreenName(config.server_screen.c_str());
    if (BarrierType::Server != _brrierType) {
        ELOG << "not the brrier server !!!!!!!";
        return false;
    }
    auto path = checkParam(config);
    if (path.isEmpty())
        return false;
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
        ELOG << "the _cooConfig is null !!!!!" << " ip = " << ip.toStdString() << ":" << port;
        return false;
    }
    if (ip.isEmpty()) {
        ELOG << "error param !!!!!" << " ip = " << ip.toStdString() << ":" << port;
        return false;
    }
    if (!screen.isEmpty())
        _cooConfig->setScreenName(screen);
    _cooConfig->setServerIp(ip);
    _cooConfig->setPort(port == 0 ? UNI_SHARE_SERVER_PORT : port);
    return true;
}

bool ShareCooperationService::startBarrier()
{
    LOG << "starting process";
    _expectedRunning = true;

    QString app;
    QStringList args;

    args << "-f" << "--no-tray" << "--debug" << cooConfig().logLevelText();


    args << "--name" << getScreenName();

#ifndef Q_OS_LINUX

//    if (m_ServerConfig.enableDragAndDrop()) {
//        args << "--enable-drag-drop";
//    }
#endif

//    if (!m_cooConfig->getCryptoEnabled()) {
        args << "--disable-crypto";
//    }

#if defined(Q_OS_WIN)
    // on windows, the profile directory changes depending on the user that
    // launched the process (e.g. when launched with elevation). setting the
    // profile dir on launch ensures it uses the same profile dir is used
    // no matter how its relaunched.
//    args << "--profile-dir" << QString::fromStdString("\"" + barrier::DataDirectories::profile().u8string() + "\"");
#endif

    if ((barrierType() == BarrierType::Client && !clientArgs(args, app))
        || (barrierType() == BarrierType::Server && !serverArgs(args, app)))
    {
        stopBarrier();
        return false;
    }

    setBarrierProcess(new QProcess());
    connect(barrierProcess(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(barrierFinished(int, QProcess::ExitStatus)));
    connect(barrierProcess(), SIGNAL(readyReadStandardOutput()), this, SLOT(logOutput()));
    connect(barrierProcess(), SIGNAL(readyReadStandardError()), this, SLOT(logError()));

    LOG << "starting " << QString(barrierType() == BarrierType::Server ? "server" : "client").toStdString();
    LOG << QString("command: %1 %2").arg(app, args.join(" ")).toStdString();

    LOG << "config file: "  << this->configFilename().toStdString();
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
    _expectedRunning = false;

//    QMutexLocker locker(&m_StopDesktopMutex);
    if (!barrierProcess()) {
        return;
    }

    if (barrierProcess()->isOpen()) {
        // try to shutdown child gracefully
        barrierProcess()->write(&ShutdownCh, 1);
        barrierProcess()->waitForFinished(5000);
        barrierProcess()->close();
    }

    delete barrierProcess();
    setBarrierProcess(nullptr);
}

bool ShareCooperationService::clientArgs(QStringList& args, QString& app)
{
    app = appPath(cooConfig().barriercName());

    if (!QFile::exists(app))
    {
        WLOG << "Barrier client not found";
        return false;
    }

#if defined(Q_OS_WIN)
    // wrap in quotes so a malicious user can't start \Program.exe as admin.
    app = QString("\"%1\"").arg(app);
#endif

//    if (cooConfig().logToFile())
//    {
//        cooConfig().persistLogDir();
//        args << "--log" << cooConfig().logFilenameCmd();
//    }

    args << "[" + cooConfig().serverIp() + "]:" + QString::number(cooConfig().port());

    return true;
}

bool ShareCooperationService::serverArgs(QStringList& args, QString& app)
{
    app = appPath(cooConfig().barriersName());

    if (!QFile::exists(app))
    {
        WLOG << "Barrier server not found";
        return false;
    }

#if defined(Q_OS_WIN)
    // wrap in quotes so a malicious user can't start \Program.exe as admin.
    app = QString("\"%1\"").arg(app);
#endif

//    if (cooConfig().logToFile())
//    {
//        cooConfig().persistLogDir();

//        args << "--log" << cooConfig().logFilenameCmd();
//    }

//        args << "--disable-client-cert-checking";

    QString configFilename = this->configFilename();
#if defined(Q_OS_WIN)
    // wrap in quotes in case username contains spaces.
    configFilename = QString("\"%1\"").arg(configFilename);
#endif
    args << "-c" << configFilename << "--address" << address();

    return true;
}

QString ShareCooperationService::checkParam(const ShareServerConfig &config)
{
    auto path =  configFilename();
    if (config.screen_left.empty()) {
        ELOG << " config's screen left  empty ===== " << config.as_json();
        return "";
    }

    if (config.screen_right.empty()) {
        ELOG << " config's screen right  empty ===== " << config.as_json();
        return "";
    }

    if (path.isEmpty()) {
        ELOG << " config path is empty ===== js = " << config.as_json() << "\n path = " << path.toStdString();
        return "";
    }

    return path;
}

void ShareCooperationService::setScreen(const ShareServerConfig &config, QTextStream *stream)
{
    // 设置screen
    *stream << "section: screens" << endl;
    *stream << "\t" << config.screen_left.c_str() << ":" << endl;
    // 设置左边
    *stream << "\t\t" << "halfDuplexCapsLock = "
            << (config.left_halfDuplexCapsLock ? "true" : "false") << endl;
    *stream << "\t\t" << "halfDuplexNumLock = "
            << (config.left_halfDuplexNumLock ? "true" : "false") << endl;
    *stream << "\t\t" << "halfDuplexScrollLock = "
            << (config.left_halfDuplexScrollLock ? "true" : "false") << endl;
    *stream << "\t\t" << "xtestIsXineramaUnaware = "
            << (config.left_xtestIsXineramaUnaware ? "true" : "false") << endl;
    *stream << "\t\t" << "preserveFocus = "
            << (config.left_preserveFocus ? "true" : "false") << endl;
    *stream << "\t\t" << "switchCorners = "
            << (config.left_switchCorners.empty() ? "none" : config.left_switchCorners.c_str()) << endl;
    *stream << "\t\t" << "switchCornerSize = " << config.left_switchCornerSize << endl;
    // 设置右边
    *stream << "\t" << config.screen_right.c_str() << ":" << endl;
    *stream << "\t\t" << "halfDuplexCapsLock = "
            << (config.right_halfDuplexCapsLock ? "true" : "false") << endl;
    *stream << "\t\t" << "halfDuplexNumLock = "
            << (config.right_halfDuplexNumLock ? "true" : "false") << endl;
    *stream << "\t\t" << "halfDuplexScrollLock = "
            << (config.right_halfDuplexScrollLock ? "true" : "false") << endl;
    *stream << "\t\t" << "xtestIsXineramaUnaware = "
            << (config.right_xtestIsXineramaUnaware ? "true" : "false") << endl;
    *stream << "\t\t" << "preserveFocus = "
            << (config.right_preserveFocus ? "true" : "false") << endl;
    *stream << "\t\t" << "switchCorners = "
            << (config.right_switchCorners.empty() ? "none" : config.right_switchCorners.c_str()) << endl;
    *stream << "\t\t" << "switchCornerSize = " << config.right_switchCornerSize << endl;
    *stream << "end" << endl << endl;

    *stream << "section: aliases" << endl;
    *stream << "end" << endl << endl;

}

void ShareCooperationService::setScreenLink(const ShareServerConfig &config, QTextStream *stream)
{
    *stream << "section: links" << endl;
    *stream << "\t" << config.screen_left.c_str() << ":" << endl;
    *stream << "\t\t" << "right = " << config.screen_right.c_str() << endl;
    *stream << "\t" << config.screen_right.c_str() << ":" << endl;
    *stream << "\t\t" << "left = " << config.screen_left.c_str() << endl;
    *stream << "end" << endl << endl;
}

void ShareCooperationService::setScreenOptions(const ShareServerConfig &config, QTextStream *stream)
{
    *stream  << "section: options" << endl;
    *stream << "\t" << "relativeMouseMoves = "
            << (config.relativeMouseMoves ? "true" : "false") << endl;
    *stream << "\t" << "screenSaverSync = "
            << (config.screenSaverSync ? "true" : "false") << endl;
    *stream << "\t" << "win32KeepForeground = "
            << (config.win32KeepForeground ? "true" : "false") << endl;
    *stream << "\t" << "clipboardSharing = "
            << (config.clipboardSharing ? "true" : "false") << endl;
    *stream << "\t" << "switchCorners = "
            << (config.switchCorners.empty() ? "none" : config.switchCorners.c_str()) << endl;
    *stream << "\t" << "switchCornerSize = " << config.switchCornerSize << endl;
    *stream << "end" << endl << endl;
}

QString ShareCooperationService::configFilename()
{
    return Util::barrierConfig();
}

QString ShareCooperationService::getScreenName()
{
    if (cooConfig().screenName() == "") {
        return QHostInfo::localHostName();
    }
    else {
        return cooConfig().screenName();
    }
}

QString ShareCooperationService::address()
{
    QString address = cooConfig().networkInterface();
    if (!address.isEmpty())
        address = "[" + address + "]";
    return address + ":" + QString::number(cooConfig().port());
}

QString ShareCooperationService::appPath(const QString& name)
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
        co::Tasked s;
        s.run_in([this]() {
            restartBarrier();
        }, 1);
#else
        QTimer::singleShot(1000, this, SLOT(restartBarrier()));
#endif
        LOG << "detected process not running, auto restarting";
    }
}

void ShareCooperationService::appendLogRaw(const QString& text, bool error)
{
    for (QString line : text.split(QRegExp("\r|\n|\r\n"))) {
        if (!line.isEmpty()) {
            if (error) {
                ELOG << line.toStdString();
            } else {
                LOG << line.toStdString();
            }
        }
    }
}

void ShareCooperationService::logOutput()
{
    if (_pBarrier)
    {
        appendLogRaw(_pBarrier->readAllStandardOutput(), false);
    }
}

void ShareCooperationService::logError()
{
    if (_pBarrier)
    {
        appendLogRaw(_pBarrier->readAllStandardError(), true);
    }
}

