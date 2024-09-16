// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include "appearance_interface.h"
#include <QObject>
#include <DBlurEffectWidget>

WIDGETS_FRAME_BEGIN_NAMESPACE
DWIDGET_USE_NAMESPACE
    using Appearance = org::deepin::dde::Appearance1;

class Appearancehandler : public QObject {
    Q_OBJECT
public:
    explicit Appearancehandler(QObject *parent = nullptr);
    virtual ~Appearancehandler() override;

    void addTargetWidget(DBlurEffectWidget *target);

    int alpha() const;

private:
    Appearance *m_appearance = nullptr;
    QList<QPointer<QWidget>> m_targets;
};
WIDGETS_FRAME_END_NAMESPACE
