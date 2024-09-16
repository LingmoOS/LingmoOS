// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanbrowserinfofilter.h"
#include "../../deepin-pc-manager/src/window/modules/common/common.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

TrashCleanBrowserInfoFilter::TrashCleanBrowserInfoFilter(QObject *parent)
    : TrashCleanAppInfoFilter(parent)
{
}

// 重写父类
// 解析JSON并仅取出cookies内容
TrashCleanAppList TrashCleanBrowserInfoFilter::InitAppData(QString jsonPath)
{
    TrashCleanAppList appList;
    // 处理json信息
    // 需要从JSON中取出包名与配置
    QJsonDocument appInfoDoc;
    QJsonParseError err;
    // 读app.json获取名称和路径信息
    QFile appfile(jsonPath);
    if (!appfile.open(QFile::ReadOnly)) {
        qDebug() << "can not find app info file:" << jsonPath;
        return appList;
    }

    QJsonObject object = QJsonDocument::fromJson(appfile.readAll(), &err).object();
    appfile.close();

    if (QJsonParseError::NoError != err.error) {
        qDebug() << "incorrect json format:" << jsonPath;
        return appList;
    }
    QJsonArray apps = object.value(CLEANER_BROWSER_JSON_NAME).toArray();

    foreach (const auto &it, apps) {
        TrashCleanApp appData;
        auto appObj = it.toObject();
        // 取出ID 路径配置
        QString appID = appObj.value(CLEANER_APP_ID).toString();
        if (appID.isEmpty()) {
            continue;
        }

        appData.appPkgName = appID;

        // 浏览器cookies路径
        foreach (const auto &cookiesPath, appObj.value(CLEANER_BROWSER_COOKIES_PATH).toArray()) {
            appData.browserCookiesPaths.append(cookiesPath.toString());
        }

        appList.append(appData);
    }

    return appList;
}

// 重写父类
// 不需要处理卸载应用
void TrashCleanBrowserInfoFilter::MergeAppInfo()
{
    MergeInstalledAppInfo(GetInstalledApps());
}

bool TrashCleanBrowserInfoFilter::isValidConfig(const TrashCleanApp &app)
{
    if (!app.appDisplayName.isEmpty() && app.browserCookiesPaths.size()) {
        return true;
    }

    return false;
}
