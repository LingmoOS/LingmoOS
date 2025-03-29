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

#ifndef XDGDESKTOPFILE_H
#define XDGDESKTOPFILE_H

#include <QObject>
#include <QVariant>
#include <QMap>

class XdgDesktopFile
{
public:
    explicit XdgDesktopFile(const QString &fileName = QString());

    bool valid() const;

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);

    bool load();
    bool save();

    QStringList keys() const;

    QString localeName() const;
    QString prefix() const;

    QString fileName() const;

private:
    bool read(const QString &prefix);

private:
    bool m_isValid;
    QString m_fileName;
    QMap<QString, QVariant> m_items;
};

#endif // XDGDESKTOPFILE_H
