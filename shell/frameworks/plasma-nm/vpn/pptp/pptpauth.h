/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PPTPAUTH_H
#define PPTPAUTH_H

#include <NetworkManagerQt/VpnSetting>

#include "settingwidget.h"

class PptpAuthWidgetPrivate;

class PptpAuthWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PptpAuthWidget)
public:
    explicit PptpAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~PptpAuthWidget() override;

    QVariantMap setting() const override;

private:
    PptpAuthWidgetPrivate *const d_ptr;
};

#endif // PPTPAUTH_H
