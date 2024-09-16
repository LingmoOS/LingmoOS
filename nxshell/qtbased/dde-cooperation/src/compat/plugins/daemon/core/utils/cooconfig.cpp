// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooconfig.h"
#include "common/constant.h"

#include <QtCore>
#include <QtNetwork>

#if defined(Q_OS_WIN)
const char CooConfig::m_BarriersName[] = "barriers.exe";
const char CooConfig::m_BarriercName[] = "barrierc.exe";
//const char CooConfig::m_BarrierLogDir[] = "log/";
#else
const char CooConfig::m_BarriersName[] = "barriers";
const char CooConfig::m_BarriercName[] = "barrierc";
//const char CooConfig::m_BarrierLogDir[] = "/var/log/";
#endif

static const char* logLevelNames[] =
{
    "ERROR",
    "WARNING",
    "NOTE",
    "INFO",
    "DEBUG",
    "DEBUG1",
    "DEBUG2"
};

CooConfig::CooConfig(QSettings* settings) :
    m_pSettings(settings),
    m_ScreenName(),
    m_Port(UNI_SHARE_SERVER_PORT),
    m_TargetServerIp(),
    m_Interface(),
    m_LogLevel(0),
    m_CryptoEnabled(false)
{
    Q_ASSERT(m_pSettings);

    loadSettings();
}

CooConfig::~CooConfig()
{
    saveSettings();
}

const QString &CooConfig::screenName() const { return m_ScreenName; }

int CooConfig::port() const { return m_Port; }

QString CooConfig::serverIp() const { return m_TargetServerIp; }

const QString &CooConfig::networkInterface() const { return m_Interface; }

int CooConfig::logLevel() const { return m_LogLevel; }

//bool CooConfig::logToFile() const { return m_LogToFile; }

//const QString &CooConfig::logFilename() const { return m_LogFilename; }

//QString CooConfig::barrierLogDir() const
//{
//#if defined(Q_OS_WIN)
//    // on windows, we want to log to program files
//    return barrierProgramDir() + "log/";
//#else
//    // on unix, we'll log to the standard log dir
//    return "/var/log/";
//#endif
//}

QString CooConfig::barrierProgramDir() const
{
    // barrier binaries should be in the same dir.
    return QCoreApplication::applicationDirPath() + "/";
}

//void CooConfig::persistLogDir()
//{
//    QDir dir = barrierLogDir();

//    // persist the log directory
//    if (!dir.exists())
//    {
//        dir.mkpath(dir.path());
//    }
//}

//const QString CooConfig::logFilenameCmd() const
//{
//    QString filename = m_LogFilename;
//#if defined(Q_OS_WIN)
//    // wrap in quotes in case username contains spaces.
//    filename = QString("\"%1\"").arg(filename);
//#endif
//    return filename;
//}

QString CooConfig::logLevelText() const
{
    return logLevelNames[logLevel()];
}

void CooConfig::loadSettings()
{
    QString groupname("cooperation_settings");
    settings().beginGroup(groupname);

    m_ScreenName = settings().value("screenName", QHostInfo::localHostName()).toString();
    m_Port = settings().value("port", UNI_SHARE_SERVER_PORT).toInt();
    m_TargetServerIp = settings().value("serverIp").toString();
    m_Interface = settings().value("interface").toString();
    m_LogLevel = settings().value("logLevel", 3).toInt(); // level 3: INFO
//    m_LogToFile = settings().value("logToFile", false).toBool();
//    m_LogFilename = settings().value("logFilename", barrierLogDir() + "barrier.log").toString();
    m_CryptoEnabled = settings().value("cryptoEnabled", true).toBool();

    settings().endGroup();
}

void CooConfig::saveSettings()
{
    QString groupname("cooperation_settings");
    settings().beginGroup(groupname);

    settings().setValue("screenName", m_ScreenName);
    settings().setValue("port", m_Port);
    settings().setValue("serverIp", m_TargetServerIp);
    settings().setValue("interface", m_Interface);
    settings().setValue("logLevel", m_LogLevel);
//    settings().setValue("logToFile", m_LogToFile);
//    settings().setValue("logFilename", m_LogFilename);
    settings().setValue("cryptoEnabled", m_CryptoEnabled);

    settings().endGroup();
    settings().sync();
}

QSettings &CooConfig::settings() { return *m_pSettings; }

void CooConfig::setScreenName(const QString &s) { m_ScreenName = s; }

void CooConfig::setPort(int i) { m_Port = i; }

void CooConfig::setServerIp(const QString& s) { m_TargetServerIp = s; }

void CooConfig::setNetworkInterface(const QString &s) { m_Interface = s; }

void CooConfig::setLogLevel(int i) { m_LogLevel = i; }

//void CooConfig::setLogToFile(bool b) { m_LogToFile = b; }

//void CooConfig::setLogFilename(const QString &s) { m_LogFilename = s; }

QString CooConfig::barriersName() const { return m_BarriersName; }

QString CooConfig::barriercName() const { return m_BarriercName; }

void CooConfig::setCryptoEnabled(bool e) { m_CryptoEnabled = e; }

bool CooConfig::getCryptoEnabled() const { return m_CryptoEnabled; }
