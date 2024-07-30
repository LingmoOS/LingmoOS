/*
 * SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "decorationdefines.h"
#include <QObject>
#include <QSharedDataPointer>
#include <kdecoration2/kdecoration2_export.h>

class KPluginMetaData;
class DecorationThemeMetaDataPrivate;

namespace KDecoration2
{
/**
 * Class providing type-safe access to data of themes
 *
 * @since 5.23
 * @author Alexander Lohnau
 */
class KDECORATIONS2_EXPORT DecorationThemeMetaData
{
public:
    explicit DecorationThemeMetaData();
    virtual ~DecorationThemeMetaData();
    DecorationThemeMetaData(const DecorationThemeMetaData &other);
    DecorationThemeMetaData &operator=(const DecorationThemeMetaData &other);

    /// User-visible name of the theme
    QString visibleName() const;
    void setVisibleName(const QString &name);

    /// Internal name of the theme
    QString themeName() const;
    void setThemeName(const QString &name);

    /// Name of the kcm to configure the decoration theme
    QString configurationName() const;
    void setConfigurationName(const QString &name);

    /// Border size of the decoration, this gets set based on the "recommendedBorderSize" key in the json metadata
    /// @internal
    KDecoration2::BorderSize borderSize() const;
    void setBorderSize(KDecoration2::BorderSize size);

    /// plugin id of theme provider
    /// @see KPluginMetaData::pluginId
    QString pluginId() const;
    void setPluginId(const QString &id);

private:
    QSharedDataPointer<DecorationThemeMetaDataPrivate> d;
};
/**
 * Class to give the KWin decorationmodel access to the plugin's themes.
 *
 * @since 5.23
 * @author Alexander Lohnau
 */
class KDECORATIONS2_EXPORT DecorationThemeProvider : public QObject
{
    Q_OBJECT

public:
    explicit DecorationThemeProvider(QObject *parent);

    /**
     * List containing information of supported themes
     */
    virtual QList<DecorationThemeMetaData> themes() const = 0;
};
}
