// Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DSysInfo>

#include <QMargins>
#include <QMetaType>

DCORE_USE_NAMESPACE

const DSysInfo::UosType UosType = DSysInfo::uosType();
const DSysInfo::UosEdition UosEdition = DSysInfo::uosEditionType();
const bool IsServerSystem = (DSysInfo::UosServer == UosType);          // 是否是服务器版
const bool IsCommunitySystem = (DSysInfo::UosCommunity == UosEdition); // 是否是社区版
const bool IsProfessionalSystem = (DSysInfo::UosProfessional == UosEdition); // 是否是专业版
const bool IsHomeSystem = (DSysInfo::UosHome == UosEdition);                 // 是否是个人版
const bool IsDeepinDesktop = (DSysInfo::DeepinDesktop == DSysInfo::deepinType()); // 是否是Deepin桌面

enum SyncType : int {
    Network,
    Sound,
    Mouse,
    Update,
    Dock,
    Launcher,
    Wallpaper,
    Theme,
    Power,
};

enum SyncState : int {
    Succeed,
    Syncing,
    Failed,
};

enum DaemonError : int {
    ErrorBind = 7519,
};

namespace utils {

QString forgetPwdURL();

QString wechatURL();

QString getRemainPasswdMsg(int remain);

void sendSysNotify(const QString &strReason);

bool isContainDigitAndChar(const QString &strdata);

QString getThemeName();

QString getActiveColor();

QString getStandardFont();

QString getDeviceKernel();

QString getOsVersion();

QString getDeviceCode();

QString getLang(const QString &region);

bool isTablet();

QString getDeviceType();

QStringList getDeviceInfo();

} // namespace utils
