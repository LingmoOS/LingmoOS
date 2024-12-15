// Copyright (C) 2011 ~ 2018 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMALLLABEL_H
#define SMALLLABEL_H

#include <QLabel>

namespace oceanui_fcitx_configtool {
namespace widgets {

class FcitxSmallLabel : public QLabel
{
    Q_OBJECT

public:
    using QLabel::QLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace oceanui

#endif // SMALLLABEL_H
