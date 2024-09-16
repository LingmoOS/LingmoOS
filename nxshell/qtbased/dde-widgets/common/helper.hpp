// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <widgetsinterface.h>
#include <QTranslator>
#include <DStandardPaths>
#include <QCoreApplication>
#include <dsysinfo.h>

DCORE_USE_NAMESPACE
WIDGETS_BEGIN_NAMESPACE
class BuildinWidgetsHelper {
public:
    inline static BuildinWidgetsHelper *instance()
    {
        static BuildinWidgetsHelper *g_helper = nullptr;
        if (!g_helper)
            g_helper = new BuildinWidgetsHelper();
        return g_helper;
    }

    inline bool loadTranslator(const QString &fileNamePrefix);

    inline QString contributor() const
    {
        return DSysInfo::isCommunityEdition() ? QLatin1String("deepin")
                                              : QLatin1String("UOS");
    }
};

bool BuildinWidgetsHelper::loadTranslator(const QString &fileNamePrefix)
{
    const auto &locale = IWidget::userInterfaceLanguage();
    if (!locale.isEmpty()) {
        const QString fileName(fileNamePrefix + locale);
        // translations dirs.
        QStringList dirs { "./translations/" };
        DCORE_USE_NAMESPACE;
        const auto &genDatas = DStandardPaths::standardLocations(
                    QStandardPaths::GenericDataLocation);
        for (const auto & path : qAsConst(genDatas)) {
            dirs << path + "/dde-widgets/translations/";
        }
        auto qtl = new QTranslator();
        for (auto dir : dirs) {
            if (qtl->load(fileName, dir)) {
                qApp->installTranslator(qtl);
                qInfo(dwLog()) << QString("load translation [%1] successful.").arg(fileName);
                return true;
            }
        }
        if (qtl->isEmpty()) {
            qWarning(dwLog()) << QString("load translation [%1] error from those dirs.").arg(fileName)
                              << dirs;
            qtl->deleteLater();
        }
    }
    return false;
}

WIDGETS_END_NAMESPACE
