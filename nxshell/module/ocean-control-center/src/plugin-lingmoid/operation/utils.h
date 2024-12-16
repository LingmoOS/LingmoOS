// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <LSysInfo>

DCORE_USE_NAMESPACE

#define LINGMOID_SERVICE   QStringLiteral("com.lingmo.lingmoid")
#define LINGMOID_PATH      QStringLiteral("/com/lingmo/lingmoid")
#define LINGMOID_INTERFACE QStringLiteral("com.lingmo.lingmoid")

#define SYNC_SERVICE   QStringLiteral("com.lingmo.sync.Daemon")
#define SYNC_INTERFACE QStringLiteral("com.lingmo.sync.Daemon")
#define SYNC_PATH      QStringLiteral("/com/lingmo/sync/Daemon")

#define UTCLOUD_PATH      QStringLiteral("/com/lingmo/utcloud/Daemon")
#define UTCLOUD_INTERFACE QStringLiteral("com.lingmo.utcloud.Daemon")

#define LICENSE_SERVICE   QStringLiteral("com.lingmo.license")
#define LICENSE_PATH      QStringLiteral("/com/lingmo/license/Info")
#define LICENSE_INTERFACE QStringLiteral("com.lingmo.license.Info")


const LSysInfo::UosType UosType = LSysInfo::uosType();
const LSysInfo::UosEdition UosEdition = LSysInfo::uosEditionType();
const bool IsServerSystem = (LSysInfo::UosServer == UosType);          // 是否是服务器版
const bool IsCommunitySystem = (LSysInfo::UosCommunity == UosEdition); // 是否是社区版
const bool IsProfessionalSystem = (LSysInfo::UosProfessional == UosEdition); // 是否是专业版
const bool IsHomeSystem = (LSysInfo::UosHome == UosEdition);                 // 是否是个人版
const bool IsLingmoDesktop = (LSysInfo::LingmoDesktop == LSysInfo::lingmoType()); // 是否是Lingmo桌面


#endif // UTILS_H
