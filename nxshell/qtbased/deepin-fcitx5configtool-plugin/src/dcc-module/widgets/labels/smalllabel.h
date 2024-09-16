// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMALLLABEL_H
#define SMALLLABEL_H

#include <QLabel>

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxSmallLabel : public QLabel
{
    Q_OBJECT

public:
    using QLabel::QLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace dcc

#endif // SMALLLABEL_H
