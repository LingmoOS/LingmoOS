// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSLABEL_H
#define TIPSLABEL_H

#include "smalllabel.h"

namespace def {
namespace widgets {

class TipsLabel : public SmallLabel
{
    Q_OBJECT
public:
    using SmallLabel::SmallLabel; // using default inheritance constructor
};

} // namespace widgets
} // namespace dcc

#endif // TIPSLABEL_H
