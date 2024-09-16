// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMCONFIG_H
#define IMCONFIG_H
class QString;
class QStringList;

//默认输入法 切换方式 默认输入法快捷键
class IMConfig
{
public:
    static QString IMSwitchKey();
    static bool setIMSwitchKey(const QString &);
    static QString defaultIMKey();
    static bool setDefaultIMKey(const QString &);
    static QString IMPluginKey(const QString &);
    static QString IMPluginPar(const QString &);

    static bool checkShortKey(const QStringList &str, QString &configName);
    static bool checkShortKey(const QString &str, QString &configName);
    static bool setIMvalue(const QString &key, const QString &value);
private:
    static QString configFile(const QString &filePath, const QString &key);
    static QString configFile(const QString &filePath, const QString &group, const QString &key);
    static bool setConfigFile(const QString &filePath, const QString &key, const QString &value);
    static QString prefix;
};

#endif // IMCONFIG_H
