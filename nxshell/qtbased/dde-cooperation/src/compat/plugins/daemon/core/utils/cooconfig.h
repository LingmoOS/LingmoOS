// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#ifndef COOCONFIG_H
#define COOCONFIG_H

#include <QObject>
#include <QString>

class QSettings;

class CooConfig: public QObject
{
    Q_OBJECT

    public:
        CooConfig(QSettings* settings);
        ~CooConfig();

    public:
        const QString& screenName() const;
        int port() const;
        QString serverIp() const;
        const QString& networkInterface() const;
        int logLevel() const;
//        bool logToFile() const;
//        const QString& logFilename() const;
//        const QString logFilenameCmd() const;
        QString logLevelText() const;

        QString barriersName() const;
        QString barriercName() const;
        QString barrierProgramDir() const;
//        QString barrierLogDir() const;

//        void persistLogDir();

        void setCryptoEnabled(bool e);
        bool getCryptoEnabled() const;

        void setAutoConnect(bool b);
        bool getAutoConnect();

        void saveSettings();
        void setPort(int i);
        void setServerIp(const QString& s);
        void setScreenName(const QString& s);

protected:
        QSettings& settings();

        void setNetworkInterface(const QString& s);
        void setLogLevel(int i);
        void setLogToFile(bool b);
        void setLogFilename(const QString& s);
        void loadSettings();

    private:
        QSettings* m_pSettings;
        QString m_ScreenName;
        int m_Port;
        QString m_TargetServerIp;
        QString m_Interface;
        int m_LogLevel;
//        bool m_LogToFile;
//        QString m_LogFilename;
        bool m_CryptoEnabled;

        static const char m_BarriersName[];
        static const char m_BarriercName[];
//        static const char m_BarrierLogDir[];
};

#endif
