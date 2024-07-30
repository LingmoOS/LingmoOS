/*
 * Copyright (C) 2015 Weng Xuetian <wengxt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef WACOMTABLETENGINE_H
#define WACOMTABLETENGINE_H

#include "dbustabletinterface.h"
#include <Plasma5Support/DataEngine>
namespace Plasma = Plasma5Support;

struct TabletData {
    QString name;
    QStringList profiles;
    int currentProfile;
    bool stylusMode;
    bool hasTouch;
    bool touch;
};

class WacomTabletEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    WacomTabletEngine(QObject *parent, const QVariantList &args);
    ~WacomTabletEngine();

    Plasma::Service *serviceForSource(const QString &source) Q_DECL_OVERRIDE;
private slots:
    void onDBusConnected();
    void onDBusDisconnected();
    void onTabletAdded(const QString &tabletId);
    void onTabletRemoved(const QString &tabletId);
    void setProfile(const QString &tabletId, const QString &profile);

private:
    QMap<QString, TabletData> m_tablets;
    QString m_source;
};

#endif // WACOMTABLETENGINE_H
