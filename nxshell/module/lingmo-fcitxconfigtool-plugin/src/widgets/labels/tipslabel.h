// Copyright (C) 2016 ~ 2018 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSLABEL_H
#define TIPSLABEL_H

#include "smalllabel.h"

namespace oceanui_fcitx_configtool {
namespace widgets {

class FcitxTipsLabel : public FcitxSmallLabel
{
    Q_OBJECT
public:
    using FcitxSmallLabel::FcitxSmallLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace oceanui_fcitx_configtool

#endif // TIPSLABEL_H
