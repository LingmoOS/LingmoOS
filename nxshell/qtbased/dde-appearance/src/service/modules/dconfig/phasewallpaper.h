// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHASEWALLPAPERDCONFIG
#define PHASEWALLPAPERDCONFIG

#include "dconfigsettings.h"

#include <DConfig>

#include <QJsonArray>
#include <QObject>
#include <QString>

#include <optional>

DCORE_USE_NAMESPACE

inline QString generateWpIndexKey(const QString &arg1, const QString &arg2)
{
    return arg1 + "+" + arg2;
}

// Dconfig 配置类
class PhaseWallPaper : public DconfigSettings
{
public:
    explicit PhaseWallPaper();
    ~PhaseWallPaper();

public:
    static std::optional<QJsonArray> setWallpaperUri(const QString &index, const QString &strMonitorName, const QString &uri);
    static QString getWallpaperUri(const QString &index, const QString &strMonitorName);
    static void resizeWorkspaceCount(int size);
};

#endif // PHASEWALLPAPERDCONFIG
