/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef APPLICATIONSTYLESETTINGS_H
#define APPLICATIONSTYLESETTINGS_H

#include <QSettings>
#include <QPalette>
#include <QMetaEnum>

class QStyle;

/*!
 * \brief The ApplicationStyleSettings class
 * \details
 * This class is used to decide the style switch stretagies for independent application.
 * For example, you can choose the color scheme switch stretagy of an application, hold
 * the color in light or dark, or follow the system's palette.
 *
 * \note
 * This API is unstable, if possible, do not use it.
 */
class ApplicationStyleSettings : public QSettings
{
    Q_OBJECT
public:
    enum ColorStretagy {
        System,
        Bright,
        Dark,
        Other
    };
    Q_ENUM(ColorStretagy)

    enum StyleStretagy {
        Default,
        Custom
    };
    Q_ENUM(StyleStretagy)

    static ApplicationStyleSettings *getInstance();

    ColorStretagy currentColorStretagy() {return m_color_stretagy;}
    StyleStretagy currentStyleStretagy() {return m_style_stretagy;}
    const QString currentCustomStyleName();

    void setColor(const QPalette::ColorRole &role, const QColor &color, const QPalette::ColorGroup &group = QPalette::Active);
    const QColor getColor(const QPalette::ColorRole &role, const QPalette::ColorGroup &group = QPalette::Active);

    QPalette getCustomPalette() {return m_custom_palette;}

signals:
    void colorStretageChanged(const ColorStretagy &stretagy);
    void styleStretageChanged(const StyleStretagy &stretagy);

public slots:
    void setColorStretagy(ColorStretagy stretagy);
    void setStyleStretagy(StyleStretagy stretagy);
    void setCustomStyle(const QString &style);

protected:
    void refreshData(bool forceSync = false);
    void readPalleteSettings();

private:
    explicit ApplicationStyleSettings(QObject *parent = nullptr);
    ~ApplicationStyleSettings() {}

    ColorStretagy m_color_stretagy;
    StyleStretagy m_style_stretagy;
    QString m_current_custom_style_name;
    QMetaEnum m_color_role_enum = QMetaEnum::fromType<QPalette::ColorRole>();
    QMetaEnum m_color_group_enum = QMetaEnum::fromType<QPalette::ColorGroup>();

    QPalette m_custom_palette;
};

#endif // APPLICATIONSTYLESETTINGS_H
