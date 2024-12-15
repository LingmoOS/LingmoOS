// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

#include <QDBusObjectPath>

DS_BEGIN_NAMESPACE
namespace am {
class AMApplet : public DApplet
{
    Q_OBJECT
public:
    explicit AMApplet(QObject *parent = nullptr);
    ~AMApplet();

    virtual bool load() override;

private slots:
    void onInterfacesRemoved(const QDBusObjectPath &objPath, const QStringList &interfaces);

private:
    void updateAppsLaunchedTimes(const QString &appId);
};

}
DS_END_NAMESPACE
