/*
 * Copyright (C) 2021 LingmoOS Team.
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

#ifndef FONTS_H
#define FONTS_H

#include <QObject>
#include <QList>
#include <QFontDatabase>
#include <QStringList>

class Fonts : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList families READ families NOTIFY familiesChanged)

public:
    explicit Fonts(QObject *parent = nullptr);

    QStringList families() const;

Q_SIGNALS:
    void familiesChanged();

protected:
    void init();

private:
    QStringList m_families;
    QFontDatabase m_fontDatabase;

};

#endif // FONTS_H
