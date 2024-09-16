// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSCUSTOMCONFIG_H
#define DBUSCUSTOMCONFIG_H

#include <QObject>

class ScreenSaverSettingDialog;
class CustomConfig : public QObject
{
    Q_OBJECT
public:
    explicit CustomConfig(QObject *parent = nullptr);
    ~CustomConfig();

    bool startCustomConfig(const QString &name);
private:
    ScreenSaverSettingDialog *m_settingDialiog { nullptr };
    QString m_lastConfigName;
};

#endif // DBUSCUSTOMCONFIG_H
