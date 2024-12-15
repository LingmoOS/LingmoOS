// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDOCEAN_H
#define BACKGROUNDOCEAN_H

#include "ddplugin_background_global.h"
#include "backgroundservice.h"
#include "appearance_interface.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DWindowManagerHelper>

#include <QObject>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

DDP_BACKGROUND_BEGIN_NAMESPACE

using InterFace = org::lingmo::ocean::Appearance1;

class BackgroundOCEAN : public BackgroundService
{
    Q_OBJECT
public:
    explicit BackgroundOCEAN(QObject *parent = nullptr);
    ~BackgroundOCEAN() override;

public:
    QString background(const QString &screen) override;
    QString getDefaultBackground() override;

protected:
    QString getBackgroundFromOCEAN(const QString &screen);
    QString getBackgroundFromConfig(const QString &screen);
private slots:
    void onAppearanceValueChanged(const QString &key);

protected:
    InterFace *interface = nullptr;
    DTK_CORE_NAMESPACE::DConfig *apperanceConf = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif   // BACKGROUNDOCEAN_H
