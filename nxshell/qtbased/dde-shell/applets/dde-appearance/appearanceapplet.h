// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

#include "Appearance1.h"

DS_BEGIN_NAMESPACE
namespace dde {
class AppearanceApplet : public DApplet
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity NOTIFY opacityChanged FINAL)
public:
    explicit AppearanceApplet(QObject *parent = nullptr);
    ~AppearanceApplet();

    virtual bool load() override;
    qreal opacity() const;
signals:
    void opacityChanged();
private:
    void initDBusProxy();
private:
    QScopedPointer<org::deepin::dde::Appearance1> m_interface;
};

}
DS_END_NAMESPACE
