/*
 *   Copyright 2018 Martin Kacej <m.kacej@atlas.sk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef WIFISETTINGS_H
#define WIFISETTINGS_H

#include <QObject>

class WifiSettings : public QObject
{
    Q_OBJECT

public:
    WifiSettings(QObject *parent = nullptr);
    Q_INVOKABLE QVariantMap getConnectionSettings(const QString &connection, const QString &type);
    Q_INVOKABLE QVariantMap getActiveConnectionInfo(const QString &connection);
    Q_INVOKABLE void addConnectionFromQML(const QVariantMap &QMLmap);
    Q_INVOKABLE void updateConnectionFromQML(const QString &path, const QVariantMap &map);
    Q_INVOKABLE QString getAccessPointDevice();
    Q_INVOKABLE QString getAccessPointConnection();

    Q_INVOKABLE bool addOtherConnection(const QString ssid, const QString userName,
                                        const QString pwd, const QString type);

    virtual ~WifiSettings();

private:
    QString m_userName;
};

#endif // WIFISETTINGS_H
