/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Kevin Ottens <ervin+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEPLATFORMTHEME_H
#define KDEPLATFORMTHEME_H

#include <qpa/qplatformtheme.h>

#include <QHash>
#include <QKeySequence>

class KHintsSettings;
class KFontSettingsData;
class KWaylandIntegration;
class X11Integration;
class QIconEngine;
class QWindow;

class KdePlatformTheme : public QPlatformTheme
{
public:
    KdePlatformTheme();
    ~KdePlatformTheme() override;

    QVariant themeHint(ThemeHint hint) const override;
    QIcon fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const override;

    const QPalette *palette(Palette type = SystemPalette) const override;
    const QFont *font(Font type) const override;
    QIconEngine *createIconEngine(const QString &iconName) const override;
    QList<QKeySequence> keyBindings(QKeySequence::StandardKey key) const override;

    QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;
    bool usePlatformNativeDialog(DialogType type) const override;

    QString standardButtonText(int button) const override;

    QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;

    QPlatformMenuBar *createPlatformMenuBar() const override;

    void globalMenuBarExistsNow();
    void globalMenuBarNoLongerExists();
    void windowCreated(QWindow *window);

private:
    void setMenuBarForWindow(QWindow *window, const QString &serviceName, const QString &objectPath) const;
    void loadSettings();
    void setQtQuickControlsTheme();

    static void setWindowProperty(QWindow *window, const QByteArray &name, const QByteArray &value);

    static bool useXdgDesktopPortal();

    KHintsSettings *m_hints = nullptr;
    KFontSettingsData *m_fontsData = nullptr;
    QScopedPointer<KWaylandIntegration> m_kwaylandIntegration;
    QScopedPointer<X11Integration> m_x11Integration;
};

#endif // KDEPLATFORMTHEME_H
