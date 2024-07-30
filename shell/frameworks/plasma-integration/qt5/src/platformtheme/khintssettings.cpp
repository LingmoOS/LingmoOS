/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Kevin Ottens <ervin+bluesystems@kde.org>
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2013 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "khintssettings.h"

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMainWindow>
#include <QPalette>
#include <QScreen>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryFile>
#include <QToolBar>
#include <QToolButton>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>

#include <KSandbox>
#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kiconloader.h>

#include <config-platformtheme.h>
#ifdef UNIT_TEST
#undef HAVE_X11
#define HAVE_X11 0
#endif
#if HAVE_X11
#include <QX11Info>
#include <X11/Xcursor/Xcursor.h>
#endif

static const QString defaultLookAndFeelPackage = QStringLiteral("org.kde.breeze.desktop");

const QDBusArgument &operator>>(const QDBusArgument &argument, QMap<QString, QVariantMap> &map)
{
    argument.beginMap();
    map.clear();

    while (!argument.atEnd()) {
        QString key;
        QVariantMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        map.insert(key, value);
    }

    argument.endMap();
    return argument;
}

KHintsSettings::KHintsSettings(const KSharedConfig::Ptr &kdeglobals)
    : QObject(nullptr)
    , mKdeGlobals(kdeglobals)
    , mUsePortal(KSandbox::isInside())
{
    if (!mKdeGlobals) {
        mKdeGlobals = KSharedConfig::openConfig();
    }
    KConfigGroup cg(mKdeGlobals, "KDE");

    if (mUsePortal) {
        updatePortalSetting();
    }

    const auto cursorBlinkRate = readConfigValue(cg, QStringLiteral("CursorBlinkRate"), 1000).toInt();
    m_hints[QPlatformTheme::CursorFlashTime] = cursorBlinkRate > 0 ? qBound(200, cursorBlinkRate, 2000) : 0; // 0 => no blinking
    m_hints[QPlatformTheme::MouseDoubleClickInterval] = readConfigValue(cg, QStringLiteral("DoubleClickInterval"), 400);
    m_hints[QPlatformTheme::StartDragDistance] = readConfigValue(cg, QStringLiteral("StartDragDist"), 10);
    m_hints[QPlatformTheme::StartDragTime] = readConfigValue(cg, QStringLiteral("StartDragTime"), 500);

    KConfigGroup cgToolbar(mKdeGlobals, "Toolbar style");
    m_hints[QPlatformTheme::ToolButtonStyle] = toolButtonStyle(cgToolbar);

    m_hints[QPlatformTheme::ToolBarIconSize] = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);

    m_hints[QPlatformTheme::ItemViewActivateItemOnSingleClick] = readConfigValue(cg, QStringLiteral("SingleClick"), false);

    m_hints[QPlatformTheme::SystemIconThemeName] = readConfigValue(QStringLiteral("Icons"), QStringLiteral("Theme"), QStringLiteral("breeze"));

    m_hints[QPlatformTheme::SystemIconFallbackThemeName] = QStringLiteral("hicolor");
    m_hints[QPlatformTheme::IconThemeSearchPaths] = xdgIconThemePaths();

    QStringList styleNames{
        QStringLiteral("breeze"),
        QStringLiteral("oxygen"),
        QStringLiteral("fusion"),
        QStringLiteral("windows"),
    };
    const QString configuredStyle = readConfigValue(cg, QStringLiteral("widgetStyle"), QString()).toString();
    if (!configuredStyle.isEmpty()) {
        styleNames.removeOne(configuredStyle);
        styleNames.prepend(configuredStyle);
    }
    const QString lnfStyle = readConfigValue(QStringLiteral("KDE"), QStringLiteral("widgetStyle"), QString()).toString();
    if (!lnfStyle.isEmpty()) {
        styleNames.removeOne(lnfStyle);
        styleNames.prepend(lnfStyle);
    }
    m_hints[QPlatformTheme::StyleNames] = styleNames;

    m_hints[QPlatformTheme::DialogButtonBoxLayout] = QDialogButtonBox::KdeLayout;
    m_hints[QPlatformTheme::DialogButtonBoxButtonsHaveIcons] = readConfigValue(cg, QStringLiteral("ShowIconsOnPushButtons"), true);
    m_hints[QPlatformTheme::UseFullScreenForPopupMenu] = true;
    m_hints[QPlatformTheme::KeyboardScheme] = QPlatformTheme::KdeKeyboardScheme;

    int uiEffectsFlags = readConfigValue(cg, QStringLiteral("GraphicEffectsLevel"), 0) != 0 ? QPlatformTheme::GeneralUiEffect : 0;
    uiEffectsFlags |= QPlatformTheme::HoverEffect;
    m_hints[QPlatformTheme::UiEffects] = uiEffectsFlags;

    m_hints[QPlatformTheme::IconPixmapSizes] = QVariant::fromValue(QList<int>() << 512 << 256 << 128 << 64 << 32 << 22 << 16 << 8);

    m_hints[QPlatformTheme::WheelScrollLines] = readConfigValue(cg, QStringLiteral("WheelScrollLines"), 3);
    if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
        QApplication::setWheelScrollLines(readConfigValue(cg, QStringLiteral("WheelScrollLines"), 3).toInt());
    }

    updateShowIconsInMenuItems(cg);

    m_hints[QPlatformTheme::ShowShortcutsInContextMenus] = true;

    QMetaObject::invokeMethod(this, "delayedDBusConnects", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "setupIconLoader", Qt::QueuedConnection);

    loadPalettes();
    updateCursorTheme();
}

KHintsSettings::~KHintsSettings()
{
    qDeleteAll(m_palettes);
}

QVariant KHintsSettings::readConfigValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    KConfigGroup userCg(mKdeGlobals, group);
    return readConfigValue(userCg, key, defaultValue);
}

QVariant KHintsSettings::readConfigValue(const KConfigGroup &cg, const QString &key, const QVariant &defaultValue) const
{
    if (mUsePortal) {
        const QString settingName = QStringLiteral("org.kde.kdeglobals.%1").arg(cg.name());
        auto groupIt = mKdeGlobalsPortal.constFind(settingName);
        if (groupIt != mKdeGlobalsPortal.constEnd()) {
            auto valueIt = groupIt.value().constFind(key);
            if (valueIt != groupIt.value().constEnd()) {
                return valueIt.value();
            }
        }
    }

    return cg.readEntry(key, defaultValue);
}

QStringList KHintsSettings::xdgIconThemePaths() const
{
    QStringList paths;

    // make sure we have ~/.local/share/icons in paths if it exists
    paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);

    const QFileInfo homeIconDir(QDir::homePath() + QStringLiteral("/.icons"));
    if (homeIconDir.isDir()) {
        paths << homeIconDir.absoluteFilePath();
    }

    return paths;
}

void KHintsSettings::delayedDBusConnects()
{
    QDBusConnection::sessionBus()
        .connect(QString(), QStringLiteral("/KToolBar"), QStringLiteral("org.kde.KToolBar"), QStringLiteral("styleChanged"), this, SLOT(toolbarStyleChanged()));
    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("notifyChange"),
                                          this,
                                          SLOT(slotNotifyChange(int, int)));
    if (mUsePortal) {
        QDBusConnection::sessionBus().connect(QString(),
                                              QStringLiteral("/org/freedesktop/portal/desktop"),
                                              QStringLiteral("org.freedesktop.portal.Settings"),
                                              QStringLiteral("SettingChanged"),
                                              this,
                                              SLOT(slotPortalSettingChanged(QString, QString, QDBusVariant)));
    }
}

void KHintsSettings::setupIconLoader()
{
    connect(KIconLoader::global(), &KIconLoader::iconChanged, this, &KHintsSettings::iconChanged);
}

void KHintsSettings::toolbarStyleChanged()
{
    mKdeGlobals->reparseConfiguration();
    KConfigGroup cg(mKdeGlobals, "Toolbar style");

    m_hints[QPlatformTheme::ToolButtonStyle] = toolButtonStyle(cg);
    // from gtksymbol.cpp
    QWidgetList widgets = QApplication::allWidgets();
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *widget = widgets.at(i);
        if (qobject_cast<QToolButton *>(widget)) {
            QEvent event(QEvent::StyleChange);
            QApplication::sendEvent(widget, &event);
        }
    }
}

void KHintsSettings::slotNotifyChange(int type, int arg)
{
    mKdeGlobals->reparseConfiguration();
    KConfigGroup cg(mKdeGlobals, "KDE");

    switch (type) {
    case PaletteChanged: {
        // Don't change the palette if the application has a custom one set
        if (!qApp->property("KDE_COLOR_SCHEME_PATH").toString().isEmpty()) {
            break;
        }
        loadPalettes();

        // QApplication::setPalette and QGuiApplication::setPalette are different functions
        // and non virtual. Call the correct one
        if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
            QPalette palette = *m_palettes[QPlatformTheme::SystemPalette];
            QApplication::setPalette(palette);
            // QTBUG QGuiApplication::paletteChanged() signal is only emitted by QGuiApplication
            // so things like SystemPalette QtQuick item that use it won't notice a palette
            // change when a QApplication which causes e.g. QML System Settings modules to not update
            Q_EMIT qApp->paletteChanged(palette);
        } else if (qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
            QGuiApplication::setPalette(*m_palettes[QPlatformTheme::SystemPalette]);
        }
        break;
    }
    case SettingsChanged: {
        SettingsCategory category = static_cast<SettingsCategory>(arg);
        if (category == SETTINGS_QT || category == SETTINGS_MOUSE) {
            updateQtSettings(cg);
        } else if (category == SETTINGS_STYLE) {
            m_hints[QPlatformTheme::DialogButtonBoxButtonsHaveIcons] = cg.readEntry("ShowIconsOnPushButtons", true);
            m_hints[QPlatformTheme::UiEffects] = cg.readEntry("GraphicEffectsLevel", 0) != 0 ? QPlatformTheme::GeneralUiEffect : 0;

            updateShowIconsInMenuItems(cg);
        }
        break;
    }
    case ToolbarStyleChanged: {
        toolbarStyleChanged();
        break;
    }
    case IconChanged:
        iconChanged(arg); // Once the KCM is ported to use IconChanged, this should not be needed
        break;
    case CursorChanged:
        updateCursorTheme();
        updateX11CursorTheme();
        break;
    case StyleChanged: {
        QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        if (!app) {
            return;
        }

        // HOTFIX here. Hardcoded default value is duplicated and may be inconsistent with the one actually defined in kcm_style kcfg
        const QString theme = readConfigValue(cg, QStringLiteral("widgetStyle"), QStringLiteral("breeze")).toString();

        QStringList styleNames;
        if (theme != QStringLiteral("breeze")) {
            styleNames << theme;
        }
        styleNames << QStringLiteral("breeze") << QStringLiteral("oxygen") << QStringLiteral("fusion") << QStringLiteral("windows");
        const QString lnfStyle = readConfigValue(QStringLiteral("KDE"), QStringLiteral("widgetStyle"), QString()).toString();
        if (!lnfStyle.isEmpty() && !styleNames.contains(lnfStyle)) {
            styleNames.prepend(lnfStyle);
        }
        m_hints[QPlatformTheme::StyleNames] = styleNames;

        app->setStyle(theme);
        loadPalettes();
        break;
    }
    default:
        qWarning() << "Unknown type of change in KGlobalSettings::slotNotifyChange: " << type;
    }
}

void KHintsSettings::slotPortalSettingChanged(const QString &group, const QString &key, const QDBusVariant &value)
{
    if (group == QLatin1String("org.kde.kdeglobals.General") && key == QLatin1String("ColorScheme")) {
        // For colors obtain complete configuration again
        updatePortalSetting();
        slotNotifyChange(PaletteChanged, 0);
    } else if (group == QLatin1String("org.kde.kdeglobals.KDE") && key == QLatin1String("widgetStyle")) {
        mKdeGlobalsPortal[group][key] = value.variant().toString();
        slotNotifyChange(StyleChanged, 0);
    } else if (group == QLatin1String("org.kde.kdeglobals.Icons") && key == QLatin1String("Theme")) {
        mKdeGlobalsPortal[group][key] = value.variant().toString();
        // Change icons for each group
        for (int i = 0; i <= 5; ++i) {
            iconChanged(i);
        }
    } else if (group == QLatin1String("org.kde.kdeglobals.Toolbar style") && key == QLatin1String("ToolButtonStyle")) {
        mKdeGlobalsPortal[group][key] = value.variant().toString();
        toolbarStyleChanged();
    }
}

void KHintsSettings::iconChanged(int group)
{
    KIconLoader::Group iconGroup = (KIconLoader::Group)group;
    if (iconGroup != KIconLoader::MainToolbar) {
        m_hints[QPlatformTheme::SystemIconThemeName] = readConfigValue(QStringLiteral("Icons"), QStringLiteral("Theme"), QStringLiteral("breeze"));
        return;
    }

    const int currentSize = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
    if (m_hints[QPlatformTheme::ToolBarIconSize] == currentSize) {
        return;
    }

    m_hints[QPlatformTheme::ToolBarIconSize] = currentSize;

    // If we are not a QApplication, means that we are a QGuiApplication, then we do nothing.
    if (!qobject_cast<QApplication *>(QCoreApplication::instance())) {
        return;
    }

    const QWidgetList widgets = QApplication::allWidgets();
    for (QWidget *widget : widgets) {
        if (qobject_cast<QToolBar *>(widget) || qobject_cast<QMainWindow *>(widget)) {
            QEvent event(QEvent::StyleChange);
            QApplication::sendEvent(widget, &event);
        }
    }
}

void KHintsSettings::updateQtSettings(KConfigGroup &cg)
{
    int flash = qBound(200, cg.readEntry("CursorBlinkRate", 1000), 2000);
    m_hints[QPlatformTheme::CursorFlashTime] = flash;

    int doubleClickInterval = cg.readEntry("DoubleClickInterval", 400);
    m_hints[QPlatformTheme::MouseDoubleClickInterval] = doubleClickInterval;

    int startDragDistance = cg.readEntry("StartDragDist", 10);
    m_hints[QPlatformTheme::StartDragDistance] = startDragDistance;

    int startDragTime = cg.readEntry("StartDragTime", 500);
    m_hints[QPlatformTheme::StartDragTime] = startDragTime;

    m_hints[QPlatformTheme::ItemViewActivateItemOnSingleClick] = cg.readEntry("SingleClick", false);

    updateShowIconsInMenuItems(cg);

    int wheelScrollLines = cg.readEntry("WheelScrollLines", 3);
    m_hints[QPlatformTheme::WheelScrollLines] = wheelScrollLines;
    QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (app) {
        QApplication::setWheelScrollLines(cg.readEntry("WheelScrollLines", 3));
    }
}

void KHintsSettings::updateShowIconsInMenuItems(KConfigGroup &cg)
{
    bool showIcons = readConfigValue(cg, QStringLiteral("ShowIconsInMenuItems"), true).toBool();
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, !showIcons);
}

Qt::ToolButtonStyle KHintsSettings::toolButtonStyle(const KConfigGroup &cg)
{
    const QString buttonStyle = readConfigValue(cg, QStringLiteral("ToolButtonStyle"), QStringLiteral("TextBesideIcon")).toString().toLower();
    return buttonStyle == QLatin1String("textbesideicon") ? Qt::ToolButtonTextBesideIcon
        : buttonStyle == QLatin1String("icontextright")   ? Qt::ToolButtonTextBesideIcon
        : buttonStyle == QLatin1String("textundericon")   ? Qt::ToolButtonTextUnderIcon
        : buttonStyle == QLatin1String("icontextbottom")  ? Qt::ToolButtonTextUnderIcon
        : buttonStyle == QLatin1String("textonly")        ? Qt::ToolButtonTextOnly
                                                          : Qt::ToolButtonIconOnly;
}

void KHintsSettings::loadPalettes()
{
    qDeleteAll(m_palettes);
    m_palettes.clear();

    if (mUsePortal && mKdeGlobalsPortal.contains(QStringLiteral("org.kde.kdeglobals.Colors:View"))) {
        // Construct a temporary KConfig file containing color setting so we can create a KColorScheme from it
        QTemporaryFile file;
        file.open();

        KSharedConfigPtr tempConfig = KSharedConfig::openConfig(file.fileName(), KConfig::SimpleConfig);
        for (auto groupIt = mKdeGlobalsPortal.constBegin(); groupIt != mKdeGlobalsPortal.constEnd(); ++groupIt) {
            if (groupIt.key().startsWith(QStringLiteral("org.kde.kdeglobals.Colors:"))) {
                KConfigGroup tempGroup(tempConfig, groupIt.key().right(groupIt.key().length() - QStringLiteral("org.kde.kdeglobals.").length()));
                for (auto valueIt = groupIt.value().constBegin(); valueIt != groupIt.value().constEnd(); ++valueIt) {
                    tempGroup.writeEntry(valueIt.key(), valueIt.value());
                }
            }
        }
        m_palettes[QPlatformTheme::SystemPalette] = new QPalette(KColorScheme::createApplicationPalette(tempConfig));
    } else if (mKdeGlobals->hasGroup("Colors:View")) {
        m_palettes[QPlatformTheme::SystemPalette] = new QPalette(KColorScheme::createApplicationPalette(mKdeGlobals));
    } else {
        KConfigGroup cg(mKdeGlobals, "KDE");
        const QString looknfeel = readConfigValue(cg, QStringLiteral("LookAndFeelPackage"), defaultLookAndFeelPackage).toString();
        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QStringLiteral("plasma/look-and-feel/") + looknfeel + QStringLiteral("/contents/colors"));
        if (!path.isEmpty()) {
            m_palettes[QPlatformTheme::SystemPalette] = new QPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(path)));
            return;
        }

        const QString scheme = readConfigValue(QStringLiteral("General"), QStringLiteral("ColorScheme"), QStringLiteral("BreezeLight")).toString();
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/") + scheme + QStringLiteral(".colors"));

        if (path.isEmpty()) {
            qWarning() << "Could not find color scheme" << scheme << "falling back to BreezeLight";
            path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/BreezeLight.colors"));
        }

        m_palettes[QPlatformTheme::SystemPalette] = new QPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(path)));
    }
}

void KHintsSettings::updateCursorTheme()
{
    KSharedConfig::Ptr inputConfig = KSharedConfig::openConfig(QStringLiteral("kcminputrc"));
    KConfigGroup mouseConfig(inputConfig, "Mouse");

    const QString cursorTheme = readConfigValue(mouseConfig, QStringLiteral("cursorTheme"), QStringLiteral("breeze_cursors")).toString();
    const int cursorSize = readConfigValue(mouseConfig, QStringLiteral("cursorSize"), 24).toInt();

    if (QGuiApplication::platformName() == QLatin1String("wayland")) {
        qputenv("XCURSOR_THEME", cursorTheme.toUtf8());
        qputenv("XCURSOR_SIZE", QByteArray::number(cursorSize));
    }
}

void KHintsSettings::updateX11CursorTheme()
{
#if HAVE_X11
    if (QX11Info::isPlatformX11()) {
        KConfig config(QStringLiteral("kcminputrc"));
        KConfigGroup g(&config, "Mouse");

        int size = g.readEntry("cursorSize", 24);
        const QString theme = g.readEntry("cursorTheme", QString());
        // Note that in X11R7.1 and earlier, calling XcursorSetTheme()
        // with a NULL theme would cause Xcursor to use "default", but
        // in 7.2 and later it will cause it to revert to the theme that
        // was configured when the application was started.
        XcursorSetTheme(QX11Info::display(), theme.isNull() ? "default" : QFile::encodeName(theme).constData());
        XcursorSetDefaultSize(QX11Info::display(), size);
    }
#endif
}

void KHintsSettings::updatePortalSetting()
{
    mKdeGlobalsPortal.clear();

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                                          QStringLiteral("org.freedesktop.portal.Settings"),
                                                          QStringLiteral("ReadAll"));
    message << QStringList{QStringLiteral("org.kde.kdeglobals.*")};

    // FIXME: async?
    QDBusMessage resultMessage = QDBusConnection::sessionBus().call(message);
    if (resultMessage.type() == QDBusMessage::ReplyMessage) {
        QDBusArgument dbusArgument = resultMessage.arguments().at(0).value<QDBusArgument>();
        dbusArgument >> mKdeGlobalsPortal;
    }
}
