/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef STRONGSWANAUTH_H
#define STRONGSWANAUTH_H

#include "settingwidget.h"

#include <NetworkManagerQt/VpnSetting>

class StrongswanAuthWidgetPrivate;

class StrongswanAuthWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StrongswanAuthWidget)
public:
    explicit StrongswanAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~StrongswanAuthWidget() override;

    virtual void readSecrets();

    QVariantMap setting() const override;

public Q_SLOTS:
    void setVisible(bool) override;

private:
    StrongswanAuthWidgetPrivate *const d_ptr;
    void acceptDialog();
};

#endif // STRONGSWANAUTH_H
