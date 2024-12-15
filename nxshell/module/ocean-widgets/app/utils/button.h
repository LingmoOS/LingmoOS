// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <DPushButton>

WIDGETS_FRAME_BEGIN_NAMESPACE
class TransparentButton : public DTK_WIDGET_NAMESPACE::DPushButton {
    Q_OBJECT
public:
    explicit TransparentButton(QWidget *parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

};

WIDGETS_FRAME_END_NAMESPACE
