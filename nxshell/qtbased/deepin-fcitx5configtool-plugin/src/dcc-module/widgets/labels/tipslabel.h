// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSLABEL_H
#define TIPSLABEL_H

#include "smalllabel.h"

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxTipsLabel : public FcitxSmallLabel
{
    Q_OBJECT
public:
    using FcitxSmallLabel::FcitxSmallLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // TIPSLABEL_H
