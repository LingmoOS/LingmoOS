/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef LINGMO_QUICK_ITEMS_SETTINGS_H
#define LINGMO_QUICK_ITEMS_SETTINGS_H
#include <QObject>
namespace LingmoUIQuick {
class SettingsPrivate;
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString liteAnimation READ liteAnimation NOTIFY liteAnimationChanged)
    Q_PROPERTY(QString liteFunction READ liteFunction NOTIFY liteFunctionChanged)
    Q_PROPERTY(bool tabletMode READ tabletMode NOTIFY tabletModeChanged)
    Q_PROPERTY(QString platformName READ platformName)

public:
    static Settings *instance();
    QString liteAnimation();
    QString liteFunction();
    bool tabletMode();
    void refresh();
    QString platformName();

Q_SIGNALS:
    void liteAnimationChanged();
    void liteFunctionChanged();
    void tabletModeChanged();

private:
    explicit Settings(QObject *parent = nullptr);
    SettingsPrivate *d = nullptr;
};
}

#endif //LINGMO_QUICK_ITEMS_SETTINGS_H
