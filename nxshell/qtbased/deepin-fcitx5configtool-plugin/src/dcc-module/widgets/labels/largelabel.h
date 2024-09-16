// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LARGELABEL_H
#define LARGELABEL_H

#include <QLabel>

namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxLargeLabel : public QLabel
{
    Q_OBJECT

public:
    using QLabel::QLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace dcc

#endif // LARGELABEL_H
