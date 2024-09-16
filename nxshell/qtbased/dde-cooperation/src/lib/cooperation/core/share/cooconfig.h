// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOCONFIG_H
#define COOCONFIG_H

#include <QObject>
#include <QString>

class QSettings;

class CooConfig : public QObject
{
    Q_OBJECT

public:
    CooConfig(QSettings *settings);
    ~CooConfig();

public:
    const QString &screenName() const;
    int port() const;
    QString serverIp() const;
    const QString &networkInterface() const;
    int logLevel() const;
    QString logLevelText() const;

    QString barriersName() const;
    QString barriercName() const;
    QString barrierProgramDir() const;
    QString configName() const;

    QString profileDir() const;
    void setProfileDir(const QString &s);

    void setCryptoEnabled(bool e);
    bool getCryptoEnabled() const;

    void saveSettings();
    void setPort(int i);
    void setServerIp(const QString &s);
    void setScreenName(const QString &s);

protected:
    QSettings &settings();

    void setNetworkInterface(const QString &s);
    void setLogLevel(int i);
    void loadSettings();

private:
    QSettings *m_pSettings;
    QString m_ScreenName;
    int m_Port;
    QString m_TargetServerIp;
    QString m_Interface;
    int m_LogLevel;
    QString m_ProfileDir;

    bool m_CryptoEnabled;

    static const char m_BarriersName[];
    static const char m_BarriercName[];
    static const char m_ConfigName[];
};

#endif
