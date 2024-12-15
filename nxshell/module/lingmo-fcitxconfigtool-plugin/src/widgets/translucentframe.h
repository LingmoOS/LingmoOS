// Copyright (C) 2016 ~ 2018 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_TRANSLUCENTFRAME_H
#define DCC_TRANSLUCENTFRAME_H

#include <QFrame>

namespace oceanui_fcitx_configtool {
namespace widgets {

class FcitxTranslucentFrame : public QFrame
{
    Q_OBJECT
public:
    explicit FcitxTranslucentFrame(QWidget *parent = 0);
};

} // namespace widgets
} // namespace oceanui

#endif // DCC_TRANSLUCENTFRAME_H
