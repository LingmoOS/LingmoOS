// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog/dfmstandardpaths.h"
#include "openwithdialog/durl.h"

#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMap>

QString DFMStandardPaths::location(DFMStandardPaths::StandardLocation type)
{
    switch (type) {
    case TrashPath:
        return QDir::homePath() + "/.local/share/Trash";
    case TrashFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case TrashInfosPath:
        return QDir::homePath() + "/.local/share/Trash/info";
    case ApplicationConfigPath:
        return QDir::homePath() + "/.config";
#ifdef APPSHAREDIR
    case TranslationPath: {
        QString path = APPSHAREDIR "/translations";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/translations";
        }
        return path;
    }
    case TemplatesPath: {
        QString path = APPSHAREDIR "/templates";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/templates";
        }
        return path;
    }
    case MimeTypePath: {
        QString path = APPSHAREDIR "/mimetypes";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/mimetypes";
        }
        return path;
    }
    case ExtensionsPath: {
        QString path = APPSHAREDIR "/extensions";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/extensions";
        }
        return path;
    }
    case ExtensionsAppEntryPath: {
        QString path = APPSHAREDIR "/extensions/appEntry";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/extensions/appEntry";
        }
        return path;
    }
#endif
#ifdef PLUGINDIR
    case PluginsPath: {
        QString path = PLUGINDIR;
        if (!QDir(path).exists()) {
            path = QString::fromLocal8Bit(PLUGINDIR).split(':').last();
        }
        return path;
    }
#endif
#ifdef QMAKE_TARGET
    case ApplicationConfigPath:
        return getConfigPath();
#endif
    case ThumbnailPath:
        return QDir::homePath() + "/.cache/thumbnails";
    case ThumbnailFailPath:
        return location(ThumbnailPath) + "/fail";
    case ThumbnailLargePath:
        return location(ThumbnailPath) + "/large";
    case ThumbnailNormalPath:
        return location(ThumbnailPath) + "/normal";
    case ThumbnailSmallPath:
        return location(ThumbnailPath) + "/small";
#ifdef APPSHAREDIR
    case kApplicationSharePath:
        return APPSHAREDIR;
#endif
    case RecentPath:
        return "recent:///";
    case HomePath:
        return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    case DesktopPath:
        return QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    case VideosPath:
        return QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    case MusicPath:
        return QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    case PicturesPath:
        return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    case DocumentsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    case DownloadsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first();
    case CachePath:
        return getCachePath();
    case DiskPath:
        return QDir::rootPath();
#ifdef NETWORK_ROOT
    case NetworkRootPath:
        return NETWORK_ROOT;
#endif
#ifdef USERSHARE_ROOT
    case UserShareRootPath:
        return USERSHARE_ROOT;
#endif
#ifdef COMPUTER_ROOT
    case ComputerRootPath:
        return COMPUTER_ROOT;
#endif
    case Root:
        return "/";
    case Vault:
        return "dfmvault:///";   // 根据需求确定使用哪种类型
    default:
        return QStringLiteral("bug://dde-file-manager-lib/interface/dfmstandardpaths.cpp#") + QT_STRINGIFY(type);
    }
}


#ifdef QMAKE_TARGET
QString DFMStandardPaths::getConfigPath()
{
    QString projectName = QMAKE_TARGET;
    QDir::home().mkpath(".config");
    QDir::home().mkpath(QString("%1/deepin/%2/").arg(".config", projectName));
    QString defaultPath = QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".config", projectName);
    return defaultPath;
}
#endif

QString DFMStandardPaths::getCachePath()
{
    QString projectName = qApp->applicationName();

    const QString &cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    QDir::home().mkpath(cachePath);

    const QString &projectPath = QString("%1/%2/%3/").arg(cachePath, qApp->organizationName(), projectName);
    QDir::home().mkpath(projectPath);

    return projectPath;
}

DFMStandardPaths::DFMStandardPaths()
{
}
