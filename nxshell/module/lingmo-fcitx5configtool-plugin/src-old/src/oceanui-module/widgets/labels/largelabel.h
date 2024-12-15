// Copyright (C) 2011 ~ 2018 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Lingmo Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LARGELABEL_H
#define LARGELABEL_H

#include <QLabel>

namespace oceanui_fcitx_configtool {
namespace widgets {

class FcitxLargeLabel : public QLabel
{
    Q_OBJECT

public:
    using QLabel::QLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace oceanui

#endif // LARGELABEL_H
