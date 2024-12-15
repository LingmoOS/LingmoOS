// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "widgetsinterface.h"

WIDGETS_BEGIN_NAMESPACE

class IWidget::Private {
public:
    explicit Private(){}
    WidgetHandler *handler = nullptr;
};

WIDGETS_END_NAMESPACE
