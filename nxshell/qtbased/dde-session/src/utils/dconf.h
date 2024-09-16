// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONF_H
#define DCONF_H

#include <DConfig>

#include <QObject>
#include <QString>

DCORE_USE_NAMESPACE

// Dconfig 配置类
class Dconf
{
public:
    Dconf();
    ~Dconf();

public:
    static const QVariant GetValue(const QString &name, const QString &subPath = QString(), const QString &key = QString(), const QVariant &fallback = QVariant());
    static bool SetValue(const QString &name, const QString &subPath, const QString &key, const QVariant &value);

private:
    static DConfig *connectDconf(const QString &name, const QString &subpath = QString(), QObject *parent = nullptr);
};

#endif // DCONF_H