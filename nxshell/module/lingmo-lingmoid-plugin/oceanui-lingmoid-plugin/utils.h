// Copyright (C) 2011 ~ 2019 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LSysInfo>

#include <QMargins>
#include <QMetaType>

DCORE_USE_NAMESPACE

const LSysInfo::UosType UosType = LSysInfo::uosType();
const LSysInfo::UosEdition UosEdition = LSysInfo::uosEditionType();
const bool IsServerSystem = (LSysInfo::UosServer == UosType);          // 是否是服务器版
const bool IsCommunitySystem = (LSysInfo::UosCommunity == UosEdition); // 是否是社区版
const bool IsProfessionalSystem = (LSysInfo::UosProfessional == UosEdition); // 是否是专业版
const bool IsHomeSystem = (LSysInfo::UosHome == UosEdition);                 // 是否是个人版
const bool IsLingmoDesktop = (LSysInfo::LingmoDesktop == LSysInfo::lingmoType()); // 是否是Lingmo桌面

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
