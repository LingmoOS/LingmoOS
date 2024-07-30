/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KHINTS_SETTINGS_H
#define KHINTS_SETTINGS_H

#include <QDBusVariant>
#include <QObject>
#include <QVariant>

#include <ksharedconfig.h>
#include <qpa/qplatformtheme.h>

class KConfigGroup;

class QPalette;
class KHintsSettings : public QObject
{
    Q_OBJECT
public:
    /**
     * An identifier for change signals.
     * @note Copied from KGlobalSettings
     */
    enum ChangeType {
        PaletteChanged = 0,
        FontChanged,
        StyleChanged,
        SettingsChanged,
        IconChanged,
        CursorChanged,
        ToolbarStyleChanged,
        ClipboardConfigChanged,
        BlockShortcuts,
        NaturalSortingChanged,
    };
    /**
     * Valid values for the settingsChanged signal
     * @note Copied from KGlobalSettings
     */
    enum SettingsCategory {
        SETTINGS_MOUSE,
        SETTINGS_COMPLETION,
        SETTINGS_PATHS,
        SETTINGS_POPUPMENU,
        SETTINGS_QT,
        SETTINGS_SHORTCUTS,
        SETTINGS_LOCALE,
        SETTINGS_STYLE,
    };
    explicit KHintsSettings(const KSharedConfig::Ptr &kdeglobals = KSharedConfig::Ptr());
    ~KHintsSettings() override;

    inline QVariant hint(QPlatformTheme::ThemeHint hint) const
    {
        return m_hints[hint];
    }

    inline QPalette *palette(QPlatformTheme::Palette type) const
    {
        return m_palettes[type];
    }
    QStringList xdgIconThemePaths() const;

private Q_SLOTS:
    void delayedDBusConnects();
    void setupIconLoader();
    void toolbarStyleChanged();
    void slotNotifyChange(int type, int arg);
    void slotPortalSettingChanged(const QString &group, const QString &key, const QDBusVariant &value);

private:
    QVariant readConfigValue(const QString &group, const QString &key, const QVariant &defaultValue);
    QVariant readConfigValue(const KConfigGroup &cg, const QString &key, const QVariant &defaultValue) const;
    void loadPalettes();
    void iconChanged(int group);
    void updateQtSettings(KConfigGroup &cg);
    void updateShowIconsInMenuItems(KConfigGroup &cg);
    Qt::ToolButtonStyle toolButtonStyle(const KConfigGroup &cg);
    void updateCursorTheme();
    void updateX11CursorTheme();
    void updatePortalSetting();

    QHash<QPlatformTheme::Palette, QPalette *> m_palettes;
    QHash<QPlatformTheme::ThemeHint, QVariant> m_hints;
    KSharedConfigPtr mKdeGlobals;
    QMap<QString, QVariantMap> mKdeGlobalsPortal;
    bool mUsePortal;
};

#endif // KHINTS_SETTINGS_H
