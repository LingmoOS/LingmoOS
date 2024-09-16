// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonutils.h"
#include "qtcompat.h"

#include <QNetworkInterface>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTranslator>
#include <QDir>
#include <QCommandLineParser>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QProcess>
#include <QRandomGenerator>
#include <QTcpSocket>

using namespace deepin_cross;

//random: 13628~23628
#define WEB_MIN_PORT 13628
#define WEB_MAX_PORT 23628

std::string CommonUitls::getFirstIp()
{
    QString ip;
    // QNetworkInterface 类提供了一个主机 IP 地址和网络接口的列表
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces()) {
        if (!netInterface.flags().testFlag(QNetworkInterface::IsRunning)
            || (netInterface.type() != QNetworkInterface::Ethernet
                && netInterface.type() != QNetworkInterface::Wifi)) {
            // 跳过非运行时, 非有线，非WiFi接口
            continue;
        }

        if (netInterface.name().startsWith("virbr") || netInterface.name().startsWith("vmnet")
            || netInterface.name().startsWith("docker")) {
            // 跳过桥接，虚拟机和docker的网络接口
            qInfo() << "netInterface name:" << netInterface.name();
            continue;
        }

        // 每个网络接口包含 0 个或多个 IP 地址
        QList<QNetworkAddressEntry> entryList = netInterface.addressEntries();
        // 遍历每一个 IP 地址
        foreach (QNetworkAddressEntry entry, entryList) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && entry.ip() != QHostAddress::LocalHost) {
                //IP地址
                ip = QString(entry.ip().toString());
                return ip.toStdString();
            }
        }
    }
    return ip.toStdString();
}

void CommonUitls::loadTranslator()
{
    QStringList translateDirs;
#ifdef _WIN32
    translateDirs << QDir::currentPath() + QDir::separator() + "translations";
#endif

    auto dataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (auto &dir : dataDirs) {
        translateDirs << dir + QDir::separator() + qApp->applicationName() + QDir::separator() + "translations";
    }

    auto locale = QLocale::system();
    QStringList missingQmfiles;
    QStringList translateFilenames { QString("%1_%2").arg(qApp->applicationName()).arg(QLocale::system().name()) };
    const QStringList parseLocalNameList = locale.name().split("_", SKIP_EMPTY_PARTS);
    if (parseLocalNameList.length() > 0)
        translateFilenames << QString("%1_%2").arg(qApp->applicationName()).arg(parseLocalNameList.at(0));

    for (const auto &translateFilename : translateFilenames) {
        for (const auto &dir : translateDirs) {
            QString translatePath = dir + QDir::separator() + translateFilename;
            if (QFile::exists(translatePath + ".qm")) {
                qDebug() << "load translate" << translatePath;
                auto translator = new QTranslator(qApp);
                translator->load(translatePath);
                qApp->installTranslator(translator);
                qApp->setProperty("dapp_locale", locale.name());
                return;
            }
        }

        if (locale.language() != QLocale::English)
            missingQmfiles << translateFilename + ".qm";
    }

    if (missingQmfiles.size() > 0) {
        qWarning() << qApp->applicationName() << "can not find qm files" << missingQmfiles;
    }
}

void CommonUitls::initLog()
{
    deepin_cross::Logger::GetInstance().init(logDir().toStdString(), qApp->applicationName().toStdString());
#ifdef QT_DEBUG
    deepin_cross::g_logLevel = deepin_cross::debug;
    LOG << "Debug build, set LogLevel " << deepin_cross::g_logLevel;
#endif

#ifdef linux
    QString logConfPath = QString("/usr/share/%1/")
                                  .arg(qApp->applicationName());   //  /usr/share/xx
#else
    QString logConfPath = logDir();
#endif
    QString configFile = logConfPath + "config.conf";   //日志级别配置
    QFile file(configFile);
    QSettings settings(configFile, QSettings::IniFormat);
    if (!file.exists()) {
        settings.setValue("g_minLogLevel", 2);
        settings.sync();
    }

#ifndef QT_DEBUG
    int level = settings.value("g_minLogLevel", 2).toInt();
    LOG << "Release build, set LogLevel " << level;
    deepin_cross::g_logLevel = static_cast<deepin_cross::LogLevel>(level);

    QTimer *timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [configFile] {
        QSettings settings(configFile, QSettings::IniFormat);
        int level = settings.value("g_minLogLevel", 2).toInt();
        auto logLevel = static_cast<deepin_cross::LogLevel>(level);
        if (deepin_cross::g_logLevel != logLevel) {
            deepin_cross::g_logLevel = logLevel;
            LOG << "Release build, update LogLevel " << level;
        }
    });
    timer->start(2000);
#endif
    if (detailLog()) {
        deepin_cross::g_logLevel = deepin_cross::debug; //详细日志输出
    }
}

QString CommonUitls::elidedText(const QString &text, Qt::TextElideMode mode, int maxLength)
{
    if (text.length() <= maxLength)
        return text;

    QString tmpText(text);
    switch (mode) {
    case Qt::ElideLeft:
        tmpText = tmpText.right(maxLength);
        tmpText.insert(0, "...");
        break;
    case Qt::ElideMiddle: {
        int charsToRemove = tmpText.length() - maxLength + 3;   // 3 represents the length of "..."
        int startRemoveIndex = (tmpText.length() - charsToRemove) / 2;
        tmpText.remove(startRemoveIndex, charsToRemove);
        tmpText.insert(startRemoveIndex, "...");
    } break;
    case Qt::ElideRight:
        tmpText = tmpText.left(maxLength) + "...";
        break;
    default:
        break;
    }

    return tmpText;
}

QString CommonUitls::tipConfPath()
{
    return logDir() + "tip.flag";
}

QString CommonUitls::logDir()
{
    QString logPath = QString("%1/%2/%3/")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation))
                              .arg(qApp->organizationName())
                              .arg(qApp->applicationName());   //~/.cache/deepin/xx

    QDir logDir(logPath);
    if (!logDir.exists())
        QDir().mkpath(logPath);

    return logPath;
}

bool CommonUitls::detailLog()
{
    QCommandLineParser parser;
    // 添加自定义选项和参数"-d"
    QCommandLineOption option("d", "Enable detail log");
    parser.addOption(option);

    // 解析命令行参数
    const auto &args = qApp->arguments();
    if (args.size() != 2 || !args.contains("-d"))
        return false;

    parser.process(args);

    // 判断选项是否存在
    bool detailMode = parser.isSet(option);
    return detailMode;
}

bool CommonUitls::isProcessRunning(const QString &processName)
{
    QProcess ps;
    ps.start("pidof", QStringList() << processName);
    ps.waitForFinished();
    return ps.exitCode() == 0;
}

bool CommonUitls::isFirstStart()
{
    QString flagPath = QString("%1/%2/%3/first_run.flag")
                               .arg(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation))
                               .arg(qApp->organizationName())
                               .arg(qApp->applicationName());   //~/.cache/deepin/xx

    QFile flag(flagPath);
    if (flag.exists())
        return false;

    if (flag.open(QIODevice::WriteOnly)) {
        LOG << "FirstStart";
        flag.close();
    } else {
        WLOG << "FirstStart Failed to create file: " << flagPath.toStdString();
    }
    return true;
}

QString CommonUitls::generateRandomPassword()
{
    QString password;
    for (int i = 0; i < 6; ++i) {
        int digit = QRandomGenerator::global()->bounded(10); // 生成0到9之间的随机数字
        password.append(QString::number(digit));
    }
    return password;
}

// 获取一个未被占用的随机端口
int CommonUitls::getAvailablePort()
{
    QRandomGenerator *generator = QRandomGenerator::global();
    while (true) {
        int port = generator->bounded(WEB_MIN_PORT, WEB_MAX_PORT);
        if (!isPortInUse(port)) {
            return port;
        }
    }
}

// 检查端口是否被占用
bool CommonUitls::isPortInUse(int port)
{
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", port);
    if (socket.waitForConnected(500)) {
        socket.disconnectFromHost();
        return true;
    }
    return false;
}
