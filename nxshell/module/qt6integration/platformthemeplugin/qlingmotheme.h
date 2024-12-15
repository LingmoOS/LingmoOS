/*
 * SPDX-FileCopyrightText: 2017-2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#ifndef QLINGMOTHEME_H
#define QLINGMOTHEME_H

#include <QMimeDatabase>

#include <private/qgenericunixthemes_p.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformnativeinterface.h>

class DThemeSettings;
class QLingmoTheme : public QGenericUnixTheme
{
public:
    QLingmoTheme();
    ~QLingmoTheme();

    bool usePlatformNativeDialog(DialogType type) const Q_DECL_OVERRIDE;
    QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const Q_DECL_OVERRIDE;

    QIconEngine *createIconEngine(const QString &iconName) const Q_DECL_OVERRIDE;
    QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const Q_DECL_OVERRIDE;
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    QIcon fileIcon(const QFileInfo &fileInfo,
                   QPlatformTheme::IconOptions iconOptions = {}) const Q_DECL_OVERRIDE;
#else
    QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
                           QPlatformTheme::IconOptions iconOptions = 0) const Q_DECL_OVERRIDE;
#endif

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;
#endif

    QVariant themeHint(ThemeHint hint) const Q_DECL_OVERRIDE;
    const QPalette *palette(Palette type) const override;
    const QFont *font(Font type) const Q_DECL_OVERRIDE;
    DThemeSettings *settings() const;
    static DThemeSettings *getSettings();

    static const char *name;

private:
    static bool m_usePlatformNativeDialog;
    static QMimeDatabase m_mimeDatabase;
    static DThemeSettings *m_settings;

    friend class QLingmoFileDialogHelper;
    friend class QLingmoPlatformMenu;
};

#endif // QLINGMOTHEME_H
