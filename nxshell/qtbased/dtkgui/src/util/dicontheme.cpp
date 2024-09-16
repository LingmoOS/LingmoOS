// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dicontheme.h"
#include "private/dbuiltiniconengine_p.h"
#include "private/dciiconengine_p.h"
#include "private/diconproxyengine_p.h"
#ifndef DTK_DISABLE_LIBXDG
#include "private/xdgiconproxyengine_p.h"
#else
#include <qpa/qplatformtheme.h>
#include <private/qguiapplication_p.h>
#include <private/qiconloader_p.h>
#endif

#include <DStandardPaths>

#include <QCache>
#include <QSet>
#include <QGuiApplication>
#include <QFileInfo>
#include <QDir>

DGUI_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

static inline QString joinPath(const QString &basePath, const QString &path)
{
    if (path.isEmpty())
        return basePath;

    return basePath + QDir::separator() + path;
}

static inline QStringList systemDciThemePaths()
{
    QStringList paths;
    const auto dataPaths = DStandardPaths::paths(DStandardPaths::DSG::DataDir);
    paths.reserve(dataPaths.size());

    for (const auto &dataPath : dataPaths) {
        paths.push_back(joinPath(dataPath, QLatin1String("icons")));
    }

    return paths;
}

static inline QString applicationDciThemePath()
{
    return QLatin1String(":/dsg/icons");
}

static inline QString applicationBuiltInIconPath()
{
    return QLatin1String(":/dsg/built-in-icons");
}

static QString findDciIconFromPath(const QString &iconName, const QString &themeName, const QString path)
{
    if (path.isEmpty() || iconName.isEmpty())
        return nullptr;

    QString themePath = joinPath(path, themeName);
    QFileInfo themeInfo(themePath);
    if (!themeInfo.exists() || !themeInfo.isDir())
        return nullptr;

    /*
     *  iconName has two types, like as:
     *  1. example.dci
     *  2. org.deepin.app/example.dci
     *
     *  We do not need to consider the icon name,
     *  just whether the file exists.
     */
    QString iconNameWithSuffix(iconName);
    // Focus that the icon name can not contain the dci suffix.
    iconNameWithSuffix += QLatin1String(".dci");
    QString iconPath = joinPath(themePath, iconNameWithSuffix);
    if (!QDir::cleanPath(iconPath).startsWith(QDir::cleanPath(themePath)))  // Wrongful
        return nullptr;

    QFileInfo iconInfo(iconPath);
    if (iconInfo.exists() && iconInfo.isFile())
        return iconPath;

    return nullptr;
}

QIconEngine *DIconTheme::createIconEngine(const QString &iconName, Options options)
{
    return new DIconProxyEngine(iconName, options);
}

QIcon DIconTheme::findQIcon(const QString &iconName, Options options)
{
    if (QDir::isAbsolutePath(iconName)) {
        return QIcon(iconName);
    }
    auto engine = createIconEngine(iconName, options);
    // fallback to QIcon::fromTheme
    if (!options.testFlag(DontFallbackToQIconFromTheme) && !engine)
        return QIcon::fromTheme(iconName);

    return QIcon(engine);
}

QIcon DIconTheme::findQIcon(const QString &iconName, const QIcon &fallback, Options options)
{
    QIcon icon = findQIcon(iconName, options);
    return !icon.isNull() ? icon : fallback;
}

bool DIconTheme::isBuiltinIcon(const QIcon &icon)
{
    if (icon.isNull())
        return false;
    QIconEngine *engine = const_cast<QIcon &>(icon).data_ptr()->engine;
    if (auto proxyEngine = dynamic_cast<DIconProxyEngine *>(engine))
        return !proxyEngine->proxyKey().compare("DBuiltinIconEngine");

    return dynamic_cast<DBuiltinIconEngine *>(engine);
}

bool DIconTheme::isXdgIcon(const QIcon &icon)
{
#ifdef DTK_DISABLE_LIBXDG
    return false;
#else
    if (icon.isNull())
        return false;

    QIconEngine *engine = const_cast<QIcon &>(icon).data_ptr()->engine;
    if (auto proxyEngine = dynamic_cast<DIconProxyEngine *>(engine))
        return !proxyEngine->proxyKey().compare("XdgIconProxyEngine");

    return dynamic_cast<XdgIconProxyEngine *>(engine);
#endif
}

class DIconTheme::CachedData
{
public:
    QCache<QString, QIcon> cache;
    QCache<QString, QString> dciIconPathCache;
};

DIconTheme::Cached::Cached()
    : data(new CachedData())
{

}

DIconTheme::Cached::~Cached()
{
    delete data;
}

int DIconTheme::Cached::maxCost() const
{
    return data->cache.maxCost();
}

void DIconTheme::Cached::setMaxCost(int cost)
{
    data->cache.setMaxCost(cost);
    data->dciIconPathCache.setMaxCost(cost);
}

void DIconTheme::Cached::clear()
{
    data->cache.clear();
    data->dciIconPathCache.clear();
}

QIcon DIconTheme::Cached::findQIcon(const QString &iconName, Options options, const QIcon &fallback)
{
    const QString themeName = QIcon::themeName();
    const QString cacheKey = themeName + QChar('/') + iconName + QChar('/') + QString::number(static_cast<int>(options));
    if (data->cache.contains(cacheKey)) {
        auto cacheIcon = data->cache.object(cacheKey);
        if (cacheIcon->isNull())
            return fallback;
        return *cacheIcon;
    }

    auto newIcon = new QIcon(DIconTheme::findQIcon(iconName, options));

    if (newIcon->isNull()) {
        return fallback;
    } else {
        data->cache.insert(cacheKey, newIcon);
    }

    return *newIcon;
}

QString DIconTheme::Cached::findDciIconFile(const QString &iconName, const QString &themeName, const QString &fallback)
{
    const QString cacheKey = themeName + QLatin1Char('/') + iconName;
    if (data->dciIconPathCache.contains(cacheKey)) {
        const QString *cachePath = data->dciIconPathCache.object(cacheKey);
        if (cachePath->isEmpty())
            return fallback;
        return *cachePath;
    }

    QString *path = new QString(DIconTheme::findDciIconFile(iconName, themeName));
    data->dciIconPathCache.insert(cacheKey, path);

    if (path->isEmpty())
        return fallback;

    return *path;
}

static inline QStringList getDciThemePaths() {
    QStringList paths = systemDciThemePaths();
    paths.push_back(applicationDciThemePath());
    return paths;
}

Q_GLOBAL_STATIC(DIconTheme::Cached, _globalCache)
Q_GLOBAL_STATIC_WITH_ARGS(QStringList, _dciThemePath, (getDciThemePaths()))

static void cleanGlobalCache() {
    if (_globalCache.exists())
        _globalCache->clear();
}

DIconTheme::Cached *DIconTheme::cached()
{
    if (Q_UNLIKELY(!_globalCache.exists() && !_globalCache.isDestroyed())) {
        qAddPostRoutine(cleanGlobalCache);
    }
    return _globalCache();
}

QString DIconTheme::findDciIconFile(const QString &iconName, const QString &themeName)
{
    if (iconName.isEmpty())
        return nullptr;

    const QString cleanIconName = QDir::cleanPath(iconName);
    if (iconName.startsWith('/') || iconName.endsWith('/')
            || cleanIconName.length() != iconName.length()
            || cleanIconName.startsWith("../"))  // Wrongful
        return nullptr;

    const int splitCharPos = iconName.lastIndexOf('/');
    QString effectiveIconName = iconName;
    // If the icon name syntax is "AAA/BBB", then the "AAA" is the icon group name, the "BBB" is the actual icon
    // name. eg: "org.deepin.app/accounts", hypothesis the icon search path is "/usr/share/dsg/icons" and
    // icon theme name is "theme_name", then the icon files list for find is:
    // "/usr/share/dsg/icons/theme_name/org.deepin.app/accounts.dci"
    // "qrc:/dsg/icons/theme_name/org.deepin.app/accounts.dci" // fallback to the qrc directory

    // "/usr/share/dsg/icons/theme_name/accounts.dci" // ignore the icon group name
    // "qrc:/dsg/icons/theme_name/accounts.dci" // ignore the icon group name

    // "/usr/share/dsg/icons/accounts.dci" // ignore the icon theme
    // "qrc:/dsg/icons/accounts.dci" // ignore the icon theme

    // "qrc:/dsg/built-in-icons/accounts.dci" // fallback to built-in icons

    const auto &searchPaths = DIconTheme::dciThemeSearchPaths();
    for (const QString &themePath : searchPaths) {
        QString iconPath = findDciIconFromPath(effectiveIconName, themeName, themePath);
        if (!iconPath.isEmpty())
            return iconPath;
    }

    if (splitCharPos > 0) {
        effectiveIconName = iconName.mid(splitCharPos + 1);
        Q_ASSERT(!effectiveIconName.isEmpty());
        for (const QString &themePath : searchPaths) {
            QString iconPath = findDciIconFromPath(effectiveIconName, themeName, themePath);
            if (!iconPath.isEmpty())
                return iconPath;
        }
    }

    // fallback to without theme directory
    for (const QString &themePath : searchPaths) {
        QString iconPath = findDciIconFromPath(effectiveIconName, nullptr, themePath);
        if (!iconPath.isEmpty())
            return iconPath;
    }

    return findDciIconFromPath(effectiveIconName, nullptr, applicationBuiltInIconPath());
}

QStringList DIconTheme::dciThemeSearchPaths()
{
    return *_dciThemePath;
}

void DIconTheme::setDciThemeSearchPaths(const QStringList &path)
{
    *_dciThemePath = path;
}

DGUI_END_NAMESPACE
