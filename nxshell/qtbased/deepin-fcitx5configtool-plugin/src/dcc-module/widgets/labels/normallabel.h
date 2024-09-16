// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALLABEL_H
#define NORMALLABEL_H

#include <QLabel>

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxNormalLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace dcc

#endif // NORMALLABEL_H
