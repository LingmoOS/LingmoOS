/*

    kicontheme.cpp: Lowlevel icon theme handling.

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kicontheme.h"

#include "debug.h"

#include <KConfigGroup>
#include <KLocalizedString> // KLocalizedString::localizedFilePath. Need such functionality in, hmm, QLocale? QStandardPaths?
#include <KSharedConfig>

#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QResource>
#include <QSet>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

#include <qplatformdefs.h>

#include <array>
#include <cmath>

Q_GLOBAL_STATIC(QString, _themeOverride)

#ifdef Q_OS_ANDROID

// on Android we are marked as porting AID, don't mess with theming
void KIconTheme::initTheme()
{
}

#else

#include <BreezeIcons>

// do init only once and avoid later helpers to mess with it again
static bool initThemeUsed = false;

// startup function to set theme once the app got constructed
static void initThemeHelper()
{
    // Makes sure the icon theme fallback is set to breeze or one of its
    // variants. Most of our apps use "lots" of icons that most of the times
    // are only available with breeze, we still honour the user icon theme
    // but if the icon is not found there, we go to breeze since it's almost
    // sure it'll be there
    BreezeIcons::initIcons();

    // ensure lib call above did the job
    Q_ASSERT(!QIcon::fallbackThemeName().isEmpty());

    // only do further stuff if we requested it
    if (!initThemeUsed) {
        return;
    }

    // do nothing if we have the proper platform theme already
    if (QGuiApplicationPrivate::platformTheme() && QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("kde")) {
        return;
    }

    // get config, with fallback to kdeglobals
    const auto config = KSharedConfig::openConfig();

    // TODO: add Qt API to really fully override the engine
    // https://codereview.qt-project.org/c/qt/qtbase/+/559108

    // enforce the theme configured by the user, with kdeglobals fallback
    // if not set, use Breeze
    QString themeToUse = KConfigGroup(config, "Icons").readEntry("Theme", QStringLiteral("breeze"));
    QIcon::setThemeName(themeToUse);
    // Tell KIconTheme about the theme, in case KIconLoader is used directly
    *_themeOverride() = themeToUse;
    qCDebug(KICONTHEMES) << "KIconTheme::initTheme() enforces the icon theme:" << themeToUse;
}
Q_COREAPP_STARTUP_FUNCTION(initThemeHelper)

void KIconTheme::initTheme()
{
    // inject paths only once
    if (!initThemeUsed) {
        // inject our icon engine in the search path
        // it will be used as the first found engine for a suffix will be taken
        // this must be done before the QCoreApplication is constructed
        const auto paths = QCoreApplication::libraryPaths();
        for (const auto &path : paths) {
            if (const QString ourPath = path + QStringLiteral("/kiconthemes6"); QFile::exists(ourPath)) {
                QCoreApplication::addLibraryPath(ourPath);
            }
        }
    }

    // initThemeHelper will do the remaining work via Q_COREAPP_STARTUP_FUNCTION(initThemeHelper) above
    initThemeUsed = true;
}

#endif

class KIconThemeDir;
class KIconThemePrivate
{
public:
    QString example, screenshot;
    bool hidden;
    KSharedConfig::Ptr sharedConfig;

    struct GroupInfo {
        KIconLoader::Group type;
        const char *name;
        int defaultSize;
        QList<int> availableSizes{};
    };
    std::array<GroupInfo, KIconLoader::LastGroup> m_iconGroups = {{
        {KIconLoader::Desktop, "Desktop", 32},
        {KIconLoader::Toolbar, "Toolbar", 22},
        {KIconLoader::MainToolbar, "MainToolbar", 22},
        {KIconLoader::Small, "Small", 16},
        {KIconLoader::Panel, "Panel", 48},
        {KIconLoader::Dialog, "Dialog", 32},
    }};

    int mDepth;
    QString mDir, mName, mInternalName, mDesc;
    QStringList mInherits;
    QStringList mExtensions;
    QList<KIconThemeDir *> mDirs;
    QList<KIconThemeDir *> mScaledDirs;
    bool followsColorScheme : 1;

    /// Searches the given dirs vector for a matching icon
    QString iconPath(const QList<KIconThemeDir *> &dirs, const QString &name, int size, qreal scale, KIconLoader::MatchType match) const;
};
Q_GLOBAL_STATIC(QString, _theme)
Q_GLOBAL_STATIC(QStringList, _theme_list)

/**
 * A subdirectory in an icon theme.
 */
class KIconThemeDir
{
public:
    KIconThemeDir(const QString &basedir, const QString &themedir, const KConfigGroup &config);

    bool isValid() const
    {
        return mbValid;
    }
    QString iconPath(const QString &name) const;
    QStringList iconList() const;
    QString constructFileName(const QString &file) const
    {
        return mBaseDir + mThemeDir + QLatin1Char('/') + file;
    }

    KIconLoader::Context context() const
    {
        return mContext;
    }
    KIconLoader::Type type() const
    {
        return mType;
    }
    int size() const
    {
        return mSize;
    }
    int scale() const
    {
        return mScale;
    }
    int minSize() const
    {
        return mMinSize;
    }
    int maxSize() const
    {
        return mMaxSize;
    }
    int threshold() const
    {
        return mThreshold;
    }

private:
    bool mbValid = false;
    KIconLoader::Type mType = KIconLoader::Fixed;
    KIconLoader::Context mContext;
    int mSize = 0;
    int mScale = 1;
    int mMinSize = 1;
    int mMaxSize = 50;
    int mThreshold = 2;

    const QString mBaseDir;
    const QString mThemeDir;
};

QString KIconThemePrivate::iconPath(const QList<KIconThemeDir *> &dirs, const QString &name, int size, qreal scale, KIconLoader::MatchType match) const
{
    QString path;
    QString tempPath; // used to cache icon path if it exists

    int delta = -INT_MAX; // current icon size delta of 'icon'
    int dw = INT_MAX; // icon size delta of current directory

    // Rather downsample than upsample
    int integerScale = std::ceil(scale);

    // Search the directory that contains the icon which matches best to the requested
    // size. If there is no directory which matches exactly to the requested size, the
    // following criteria get applied:
    // - Take a directory having icons with a minimum difference to the requested size.
    // - Prefer directories that allow a downscaling even if the difference to
    //   the requested size is bigger than a directory where an upscaling is required.
    for (KIconThemeDir *dir : dirs) {
        if (dir->scale() != integerScale) {
            continue;
        }

        if (match == KIconLoader::MatchExact) {
            if ((dir->type() == KIconLoader::Fixed) && (dir->size() != size)) {
                continue;
            }
            if ((dir->type() == KIconLoader::Scalable) //
                && ((size < dir->minSize()) || (size > dir->maxSize()))) {
                continue;
            }
            if ((dir->type() == KIconLoader::Threshold) //
                && (abs(dir->size() - size) > dir->threshold())) {
                continue;
            }
        } else {
            // dw < 0 means need to scale up to get an icon of the requested size.
            // Upscaling should only be done if no larger icon is available.
            if (dir->type() == KIconLoader::Fixed) {
                dw = dir->size() - size;
            } else if (dir->type() == KIconLoader::Scalable) {
                if (size < dir->minSize()) {
                    dw = dir->minSize() - size;
                } else if (size > dir->maxSize()) {
                    dw = dir->maxSize() - size;
                } else {
                    dw = 0;
                }
            } else if (dir->type() == KIconLoader::Threshold) {
                if (size < dir->size() - dir->threshold()) {
                    dw = dir->size() - dir->threshold() - size;
                } else if (size > dir->size() + dir->threshold()) {
                    dw = dir->size() + dir->threshold() - size;
                } else {
                    dw = 0;
                }
            }
            // Usually if the delta (= 'dw') of the current directory is
            // not smaller than the delta (= 'delta') of the currently best
            // matching icon, this candidate can be skipped. But skipping
            // the candidate may only be done, if this does not imply
            // in an upscaling of the icon (it is OK to use a directory with
            // smaller icons that what we've already found, however).
            if ((abs(dw) >= abs(delta)) && ((dw < 0) || (delta > 0))) {
                continue;
            }

            if (match == KIconLoader::MatchBestOrGreaterSize && dw < 0) {
                continue;
            }
        }

        // cache the result of iconPath() call which checks if file exists
        tempPath = dir->iconPath(name);

        if (tempPath.isEmpty()) {
            continue;
        }

        path = tempPath;

        // if we got in MatchExact that far, we find no better
        if (match == KIconLoader::MatchExact) {
            return path;
        }
        delta = dw;
        if (delta == 0) {
            return path; // We won't find a better match anyway
        }
    }
    return path;
}

KIconTheme::KIconTheme(const QString &name, const QString &appName, const QString &basePathHint)
    : d(new KIconThemePrivate)
{
    d->mInternalName = name;

    QStringList themeDirs;

    // Applications can have local additions to the global "locolor" and
    // "hicolor" icon themes. For these, the _global_ theme description
    // files are used..

    /* clang-format off */
    if (!appName.isEmpty()
        && (name == defaultThemeName()
            || name == QLatin1String("hicolor")
            || name == QLatin1String("locolor"))) { /* clang-format on */
        const QString suffix = QLatin1Char('/') + appName + QLatin1String("/icons/") + name + QLatin1Char('/');
        QStringList dataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (auto &cDir : dataDirs) {
            cDir += suffix;
            if (QFileInfo::exists(cDir)) {
                themeDirs += cDir;
            }
        }

        if (!basePathHint.isEmpty()) {
            // Checks for dir existing are done below
            themeDirs += basePathHint + QLatin1Char('/') + name + QLatin1Char('/');
        }
    }

    // Find the theme description file. These are either locally in the :/icons resource path or global.
    QStringList icnlibs;

    // local embedded icons have preference
    icnlibs << QStringLiteral(":/icons");

    // global icons
    icnlibs += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);

    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    icnlibs += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("pixmaps"), QStandardPaths::LocateDirectory);

    QString fileName;
    QString mainSection;
    const QString pathSuffix = QLatin1Char('/') + name + QLatin1Char('/');
    const QLatin1String indexTheme("index.theme");
    const QLatin1String indexDesktop("theme.desktop");
    for (auto &iconDir : icnlibs) {
        iconDir += pathSuffix;
        const QFileInfo fi(iconDir);
        if (!fi.exists() || !fi.isDir()) {
            continue;
        }
        themeDirs.append(iconDir);

        if (d->mDir.isEmpty()) {
            QString possiblePath;
            if (possiblePath = iconDir + indexTheme; QFileInfo::exists(possiblePath)) {
                d->mDir = iconDir;
                fileName = possiblePath;
                mainSection = QStringLiteral("Icon Theme");
            } else if (possiblePath = iconDir + indexDesktop; QFileInfo::exists(possiblePath)) {
                d->mDir = iconDir;
                fileName = possiblePath;
                mainSection = QStringLiteral("KDE Icon Theme");
            }
        }
    }

    if (d->mDir.isEmpty()) {
        qCDebug(KICONTHEMES) << "Icon theme" << name << "not found.";
        return;
    }

    // Use KSharedConfig to avoid parsing the file many times, from each component.
    // Need to keep a ref to it to make this useful
    d->sharedConfig = KSharedConfig::openConfig(fileName, KConfig::SimpleConfig);

    KConfigGroup cfg(d->sharedConfig, mainSection);
    d->mName = cfg.readEntry("Name");
    d->mDesc = cfg.readEntry("Comment");
    d->mDepth = cfg.readEntry("DisplayDepth", 32);
    d->mInherits = cfg.readEntry("Inherits", QStringList());
    if (name != defaultThemeName()) {
        for (auto &inheritedTheme : d->mInherits) {
            if (inheritedTheme == QLatin1String("default")) {
                inheritedTheme = defaultThemeName();
            }
        }
    }

    d->hidden = cfg.readEntry("Hidden", false);
    d->followsColorScheme = cfg.readEntry("FollowsColorScheme", false);
    d->example = cfg.readPathEntry("Example", QString());
    d->screenshot = cfg.readPathEntry("ScreenShot", QString());
    d->mExtensions =
        cfg.readEntry("KDE-Extensions", QStringList{QStringLiteral(".png"), QStringLiteral(".svgz"), QStringLiteral(".svg"), QStringLiteral(".xpm")});

    QSet<QString> addedDirs; // Used for avoiding duplicates.
    const QStringList dirs = cfg.readPathEntry("Directories", QStringList()) + cfg.readPathEntry("ScaledDirectories", QStringList());
    for (const auto &dirName : dirs) {
        KConfigGroup cg(d->sharedConfig, dirName);
        for (const auto &themeDir : std::as_const(themeDirs)) {
            const QString currentDir(themeDir + dirName + QLatin1Char('/'));
            if (!addedDirs.contains(currentDir) && QDir(currentDir).exists()) {
                addedDirs.insert(currentDir);
                KIconThemeDir *dir = new KIconThemeDir(themeDir, dirName, cg);
                if (dir->isValid()) {
                    if (dir->scale() > 1) {
                        d->mScaledDirs.append(dir);
                    } else {
                        d->mDirs.append(dir);
                    }
                } else {
                    delete dir;
                }
            }
        }
    }

    KConfigGroup cg(d->sharedConfig, mainSection);
    for (auto &iconGroup : d->m_iconGroups) {
        iconGroup.defaultSize = cg.readEntry(iconGroup.name + QLatin1String("Default"), iconGroup.defaultSize);
        iconGroup.availableSizes = cg.readEntry(iconGroup.name + QLatin1String("Sizes"), QList<int>());
    }
}

KIconTheme::~KIconTheme()
{
    qDeleteAll(d->mDirs);
    qDeleteAll(d->mScaledDirs);
}

QString KIconTheme::name() const
{
    return d->mName;
}

QString KIconTheme::internalName() const
{
    return d->mInternalName;
}

QString KIconTheme::description() const
{
    return d->mDesc;
}

QString KIconTheme::example() const
{
    return d->example;
}

QString KIconTheme::screenshot() const
{
    return d->screenshot;
}

QString KIconTheme::dir() const
{
    return d->mDir;
}

QStringList KIconTheme::inherits() const
{
    return d->mInherits;
}

bool KIconTheme::isValid() const
{
    return !d->mDirs.isEmpty() || !d->mScaledDirs.isEmpty();
}

bool KIconTheme::isHidden() const
{
    return d->hidden;
}

int KIconTheme::depth() const
{
    return d->mDepth;
}

int KIconTheme::defaultSize(KIconLoader::Group group) const
{
    if (group < 0 || group >= KIconLoader::LastGroup) {
        qCWarning(KICONTHEMES) << "Invalid icon group:" << group << ", should be one of KIconLoader::Group";
        return -1;
    }
    return d->m_iconGroups[group].defaultSize;
}

QList<int> KIconTheme::querySizes(KIconLoader::Group group) const
{
    if (group < 0 || group >= KIconLoader::LastGroup) {
        qCWarning(KICONTHEMES) << "Invalid icon group:" << group << ", should be one of KIconLoader::Group";
        return QList<int>();
    }
    return d->m_iconGroups[group].availableSizes;
}

static bool isAnyOrDirContext(const KIconThemeDir *dir, KIconLoader::Context context)
{
    return context == KIconLoader::Any || context == dir->context();
}

QStringList KIconTheme::queryIcons(int size, KIconLoader::Context context) const
{
    // Try to find exact match
    QStringList result;
    const QList<KIconThemeDir *> listDirs = d->mDirs + d->mScaledDirs;
    for (const KIconThemeDir *dir : listDirs) {
        if (!isAnyOrDirContext(dir, context)) {
            continue;
        }

        const int dirSize = dir->size();
        if ((dir->type() == KIconLoader::Fixed && dirSize == size) //
            || (dir->type() == KIconLoader::Scalable && size >= dir->minSize() && size <= dir->maxSize())
            || (dir->type() == KIconLoader::Threshold && abs(size - dirSize) < dir->threshold())) {
            result += dir->iconList();
        }
    }

    return result;
}

QStringList KIconTheme::queryIconsByContext(int size, KIconLoader::Context context) const
{
    int dw;

    // We want all the icons for a given context, but we prefer icons
    // of size "size" . Note that this may (will) include duplicate icons
    // QStringList iconlist[34]; // 33 == 48-16+1
    QStringList iconlist[128]; // 33 == 48-16+1
    // Usually, only the 0, 6 (22-16), 10 (32-22), 16 (48-32 or 32-16),
    // 26 (48-22) and 32 (48-16) will be used, but who knows if someone
    // will make icon themes with different icon sizes.
    const auto listDirs = d->mDirs + d->mScaledDirs;
    for (KIconThemeDir *dir : listDirs) {
        if (!isAnyOrDirContext(dir, context)) {
            continue;
        }
        dw = abs(dir->size() - size);
        iconlist[(dw < 127) ? dw : 127] += dir->iconList();
    }

    QStringList iconlistResult;
    for (int i = 0; i < 128; i++) {
        iconlistResult += iconlist[i];
    }

    return iconlistResult;
}

bool KIconTheme::hasContext(KIconLoader::Context context) const
{
    const auto listDirs = d->mDirs + d->mScaledDirs;
    for (KIconThemeDir *dir : listDirs) {
        if (isAnyOrDirContext(dir, context)) {
            return true;
        }
    }
    return false;
}

QString KIconTheme::iconPathByName(const QString &iconName, int size, KIconLoader::MatchType match) const
{
    return iconPathByName(iconName, size, match, 1 /*scale*/);
}

QString KIconTheme::iconPathByName(const QString &iconName, int size, KIconLoader::MatchType match, qreal scale) const
{
    for (const QString &current : std::as_const(d->mExtensions)) {
        const QString path = iconPath(iconName + current, size, match, scale);
        if (!path.isEmpty()) {
            return path;
        }
    }
    return QString();
}

bool KIconTheme::followsColorScheme() const
{
    return d->followsColorScheme;
}

QString KIconTheme::iconPath(const QString &name, int size, KIconLoader::MatchType match) const
{
    return iconPath(name, size, match, 1 /*scale*/);
}

QString KIconTheme::iconPath(const QString &name, int size, KIconLoader::MatchType match, qreal scale) const
{
    // first look for a scaled image at exactly the requested size
    QString path = d->iconPath(d->mScaledDirs, name, size, scale, KIconLoader::MatchExact);

    // then look for an unscaled one but request it at larger size so it doesn't become blurry
    if (path.isEmpty()) {
        path = d->iconPath(d->mDirs, name, size * scale, 1, match);
    }
    return path;
}

// static
QString KIconTheme::current()
{
    // Static pointers because of unloading problems wrt DSO's.
    if (_themeOverride && !_themeOverride->isEmpty()) {
        *_theme() = *_themeOverride();
    }
    if (!_theme()->isEmpty()) {
        return *_theme();
    }

    QString theme;
    // Check application specific config for a theme setting.
    KConfigGroup app_cg(KSharedConfig::openConfig(QString(), KConfig::NoGlobals), "Icons");
    theme = app_cg.readEntry("Theme", QString());
    if (theme.isEmpty() || theme == QLatin1String("hicolor")) {
        // No theme, try to use Qt's. A Platform plugin might have set
        // a good theme there.
        theme = QIcon::themeName();
    }
    if (theme.isEmpty() || theme == QLatin1String("hicolor")) {
        // Still no theme, try config with kdeglobals.
        KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
        theme = cg.readEntry("Theme", QStringLiteral("breeze"));
    }
    if (theme.isEmpty() || theme == QLatin1String("hicolor")) {
        // Still no good theme, use default.
        theme = defaultThemeName();
    }
    *_theme() = theme;
    return *_theme();
}

void KIconTheme::forceThemeForTests(const QString &themeName)
{
    *_themeOverride() = themeName;
    _theme()->clear(); // ::current sets this again based on conditions
}

// static
QStringList KIconTheme::list()
{
    // Static pointer because of unloading problems wrt DSO's.
    if (!_theme_list()->isEmpty()) {
        return *_theme_list();
    }

    // Find the theme description file. These are either locally in the :/icons resource path or global.
    QStringList icnlibs;

    // local embedded icons have preference
    icnlibs << QStringLiteral(":/icons");

    // global icons
    icnlibs += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);

    // These are not in the icon spec, but e.g. GNOME puts some icons there anyway.
    icnlibs += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("pixmaps"), QStandardPaths::LocateDirectory);

    for (const QString &iconDir : std::as_const(icnlibs)) {
        QDir dir(iconDir);
        const QStringList themeDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &theme : themeDirs) {
            if (theme.startsWith(QLatin1String("default."))) {
                continue;
            }

            const QString prefix = iconDir + QLatin1Char('/') + theme;
            if (!QFileInfo::exists(prefix + QLatin1String("/index.desktop")) //
                && !QFileInfo::exists(prefix + QLatin1String("/index.theme"))) {
                continue;
            }

            if (!KIconTheme(theme).isValid()) {
                continue;
            }

            if (!_theme_list()->contains(theme)) {
                _theme_list()->append(theme);
            }
        }
    }
    return *_theme_list();
}

// static
void KIconTheme::reconfigure()
{
    _theme()->clear();
    _theme_list()->clear();
}

// static
QString KIconTheme::defaultThemeName()
{
    return QStringLiteral("hicolor");
}

/*** KIconThemeDir ***/

KIconThemeDir::KIconThemeDir(const QString &basedir, const QString &themedir, const KConfigGroup &config)
    : mSize(config.readEntry("Size", 0))
    , mScale(config.readEntry("Scale", 1))
    , mBaseDir(basedir)
    , mThemeDir(themedir)
{
    if (mSize == 0) {
        return;
    }

    QString tmp = config.readEntry(QStringLiteral("Context"));
    if (tmp == QLatin1String("Devices")) {
        mContext = KIconLoader::Device;
    } else if (tmp == QLatin1String("MimeTypes")) {
        mContext = KIconLoader::MimeType;
    } else if (tmp == QLatin1String("Applications")) {
        mContext = KIconLoader::Application;
    } else if (tmp == QLatin1String("Actions")) {
        mContext = KIconLoader::Action;
    } else if (tmp == QLatin1String("Animations")) {
        mContext = KIconLoader::Animation;
    } else if (tmp == QLatin1String("Categories")) {
        mContext = KIconLoader::Category;
    } else if (tmp == QLatin1String("Emblems")) {
        mContext = KIconLoader::Emblem;
    } else if (tmp == QLatin1String("Emotes")) {
        mContext = KIconLoader::Emote;
    } else if (tmp == QLatin1String("International")) {
        mContext = KIconLoader::International;
    } else if (tmp == QLatin1String("Places")) {
        mContext = KIconLoader::Place;
    } else if (tmp == QLatin1String("Status")) {
        mContext = KIconLoader::StatusIcon;
    } else if (tmp == QLatin1String("Stock")) { // invalid, but often present context, skip warning
        return;
    } else if (tmp == QLatin1String("FileSystems")) { // invalid, but present context for hicolor, skip warning
        return;
    } else if (tmp == QLatin1String("Legacy")) { // invalid, but often present context for Adwaita, skip warning
        return;
    } else if (tmp == QLatin1String("UI")) { // invalid, but often present context for Adwaita, skip warning
        return;
    } else if (tmp.isEmpty()) {
        // do nothing. key not required
    } else {
        qCDebug(KICONTHEMES) << "Invalid Context=" << tmp << "line for icon theme: " << constructFileName(QString());
        return;
    }
    tmp = config.readEntry(QStringLiteral("Type"), QStringLiteral("Threshold"));
    if (tmp == QLatin1String("Fixed")) {
        mType = KIconLoader::Fixed;
    } else if (tmp == QLatin1String("Scalable")) {
        mType = KIconLoader::Scalable;
    } else if (tmp == QLatin1String("Threshold")) {
        mType = KIconLoader::Threshold;
    } else {
        qCDebug(KICONTHEMES) << "Invalid Type=" << tmp << "line for icon theme: " << constructFileName(QString());
        return;
    }
    if (mType == KIconLoader::Scalable) {
        mMinSize = config.readEntry(QStringLiteral("MinSize"), mSize);
        mMaxSize = config.readEntry(QStringLiteral("MaxSize"), mSize);
    } else if (mType == KIconLoader::Threshold) {
        mThreshold = config.readEntry(QStringLiteral("Threshold"), 2);
    }
    mbValid = true;
}

QString KIconThemeDir::iconPath(const QString &name) const
{
    if (!mbValid) {
        return QString();
    }

    const QString file = constructFileName(name);
    if (QFileInfo::exists(file)) {
        return KLocalizedString::localizedFilePath(file);
    }

    return QString();
}

QStringList KIconThemeDir::iconList() const
{
    const QDir icondir = constructFileName(QString());

    const QStringList formats = QStringList() << QStringLiteral("*.png") << QStringLiteral("*.svg") << QStringLiteral("*.svgz") << QStringLiteral("*.xpm");
    const QStringList lst = icondir.entryList(formats, QDir::Files);

    QStringList result;
    result.reserve(lst.size());
    for (const QString &file : lst) {
        result += constructFileName(file);
    }
    return result;
}
