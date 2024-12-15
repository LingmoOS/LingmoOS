// Copyright (C) 2011 ~ 2018 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALLABEL_H
#define NORMALLABEL_H

#include <QLabel>

namespace oceanui_fcitx_configtool {
namespace widgets {

class FcitxNormalLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace oceanui

#endif // NORMALLABEL_H
