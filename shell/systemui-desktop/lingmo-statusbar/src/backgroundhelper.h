/*
 * Copyright (C) 2024 - 2022 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACKGROUNDHELPER_H
#define BACKGROUNDHELPER_H

#include <QObject>
#include <QColor>

class BackgroundHelper : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundHelper(QObject *parent = nullptr);

    Q_INVOKABLE void setColor(QColor c);
    Q_INVOKABLE void setBackgound(const QString &fileName);

private slots:
    void onPrimaryScreenChanged();
    void onChanged();

signals:
    void newColor(QColor color, bool darkMode);

private:
    int m_statusBarHeight;
    int m_type;
    QColor m_color;
    QString m_wallpaper;
};

#endif // BACKGROUNDHELPER_H
