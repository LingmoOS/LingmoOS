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

#include "application-style-settings.h"
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QStandardPaths>
#include <QStyle>

#include <QFileSystemWatcher>

static ApplicationStyleSettings *global_instance = nullptr;

ApplicationStyleSettings *ApplicationStyleSettings::getInstance()
{
    if (!global_instance)
        global_instance = new ApplicationStyleSettings;
    return global_instance;
}

const QString ApplicationStyleSettings::currentCustomStyleName()
{
    if (m_style_stretagy == Default)
        return nullptr;
    return m_current_custom_style_name;
}

void ApplicationStyleSettings::setColor(const QPalette::ColorRole &role, const QColor &color, const QPalette::ColorGroup &group)
{
    beginGroup(m_color_group_enum.key(group));
    setValue(m_color_role_enum.key(role), color);
    endGroup();
    QtConcurrent::run([=](){
        this->sync();
    });
    auto palette = QApplication::palette();
    palette.setColor(group, role, color);
    QApplication::setPalette(palette);
    qApp->paletteChanged(palette);
}

const QColor ApplicationStyleSettings::getColor(const QPalette::ColorRole &role, const QPalette::ColorGroup &group)
{
    beginGroup(m_color_role_enum.key(group));
    auto data = value(m_color_role_enum.key(role));
    QColor color = qvariant_cast<QColor>(data);
    endGroup();
    if (color.isValid())
        return color;
    return QApplication::palette().color(group, role);
}

void ApplicationStyleSettings::setColorStretagy(ApplicationStyleSettings::ColorStretagy stretagy)
{
    if (m_color_stretagy != stretagy) {
        m_color_stretagy = stretagy;
        setValue("color-stretagy", stretagy);
        Q_EMIT colorStretageChanged(stretagy);
        QtConcurrent::run([=](){
            this->sync();
        });
    }
}

void ApplicationStyleSettings::setStyleStretagy(ApplicationStyleSettings::StyleStretagy stretagy)
{
    if (m_style_stretagy != stretagy) {
        m_style_stretagy = stretagy;
        setValue("style-stretagy", stretagy);
        Q_EMIT styleStretageChanged(stretagy);
        QtConcurrent::run([=](){
            this->sync();
        });
    }
}

void ApplicationStyleSettings::setCustomStyle(const QString &style)
{
    m_current_custom_style_name = style;
    QApplication::setStyle(style);
}

void ApplicationStyleSettings::refreshData(bool forceSync)
{
    sync();
    m_custom_palette = QApplication::palette();

    auto color_stretagy = qvariant_cast<ColorStretagy>(value("color-stretagy"));
    if (color_stretagy != m_color_stretagy) {
        m_color_stretagy = color_stretagy;
        Q_EMIT colorStretageChanged(m_color_stretagy);
    }

    auto style_stretagy = qvariant_cast<StyleStretagy>(value("style-stretagy"));
    if (style_stretagy != m_style_stretagy) {
        m_style_stretagy = style_stretagy;
        Q_EMIT styleStretageChanged(m_style_stretagy);
    }

    auto current_custom_style_name = value("custom-style").toString();
    if (m_current_custom_style_name != current_custom_style_name) {
        m_current_custom_style_name = current_custom_style_name;
        QApplication::setStyle(m_current_custom_style_name);
    }

    readPalleteSettings();

    if (forceSync) {
        QtConcurrent::run([=](){
            this->sync();
        });
    }
}

void ApplicationStyleSettings::readPalleteSettings()
{
    for (int i = 0; i < m_color_group_enum.keyCount(); i++) {
        beginGroup(m_color_group_enum.key(i));
        for (int j = 0; j < m_color_role_enum.keyCount(); j++) {
            auto data = value(m_color_role_enum.key(j));
            if (data.isValid()) {
                m_custom_palette.setColor(QPalette::ColorGroup(i),
                                          QPalette::ColorRole(j),
                                          qvariant_cast<QColor>(data));
            }
        }
    }
}

ApplicationStyleSettings::ApplicationStyleSettings(QObject *parent) : QSettings(parent)
{
    /*!
      \todo make settings together into an ini file.
      */
//    QString configFileName = QApplication::organizationDomain() + "." + QApplication::organizationName() + "." + QApplication::applicationName();
//    QString configDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" + "lingmo";
//    QString configPath = configDir + "/" + configFileName;
//    setPath(QSettings::IniFormat, QSettings::UserScope, configPath);
//    setDefaultFormat(QSettings::IniFormat);
    setDefaultFormat(QSettings::IniFormat);
#if (QT_VERSION >= QT_VERSION_CHECK(5,10,0))
    setAtomicSyncRequired(true);
#endif

    m_color_stretagy = qvariant_cast<ColorStretagy>(value("color-stretagy"));
    m_style_stretagy = qvariant_cast<StyleStretagy>(value("style-stretagy"));
    m_current_custom_style_name = value("custom-style").toString();
    m_custom_palette = QApplication::palette();
    readPalleteSettings();

    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    watcher->addPath(fileName());
    connect(watcher, &QFileSystemWatcher::fileChanged, [=](){
        refreshData();
    });
}
