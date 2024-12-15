// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitemmodel.h"
#include <QStandardItem>

namespace apps {
class AppItem : public QStandardItem
{
public:
    AppItem(const QString &appid, AppItemModel::AppTypes appType);

    // action
    virtual void launch(const QString &action = {}, const QStringList &fields = {}, const QVariantMap &options = {});

    // desktop file static data
    QString appId() const;
    void setAppId(const QString &appid);

    AppItemModel::AppTypes appType() const;
    void setAppType(AppItemModel::AppTypes appType);

    QString appName() const;
    void setAppName(const QString &name);

    QString appIconName() const;
    void setAppIconName(const QString &appIconName);

    QString startupWMClass() const;
    void setStartupWMclass(const QString &wmClass);

    bool noDisplay() const;
    void setNoDisPlay(bool noDisplay);

    AppItemModel::OCEANCategories oceanCategories() const;
    void setOCEANCategories(const AppItemModel::OCEANCategories &categories);

    QString actions() const;
    void setActions(const QString &actions);

    // desktop file peripheral data
    virtual quint64 lastLaunchedTime() const;
    virtual void setLastLaunchedTime(const quint64 &time);

    virtual quint64 installedTime() const;
    virtual void setInstalledTime(const quint64 &time);

    virtual quint64 launchedTimes() const;
    virtual void setLaunchedTimes(const quint64 &times);

    virtual bool docked() const;
    virtual void setDocked(bool docked);

    virtual bool onDesktop() const;
    virtual void setOnDesktop(bool onDesktop);

    virtual bool autoStart() const;
    virtual void setAutoStart(bool autoStart);
};
}
