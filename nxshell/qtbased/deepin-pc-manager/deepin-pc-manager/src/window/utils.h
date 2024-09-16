// Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef V20_DISPLAY_UTILS_H
#define V20_DISPLAY_UTILS_H

#include "namespace.h"

#include <QMargins>
#include <QVariant>

Q_DECLARE_METATYPE(QMargins)

DEF_NAMESPACE_BEGIN

// const QMargins ThirdPageContentsMargins(10, 10, 10, 10);
const QMargins ThirdPageContentsMargins(0, 0, 0, 0);
const QMargins ListViweItemMargin(10, 8, 10, 8);
const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);
const QMargins ListViweRightSubscriptItemMargin(42, 0, 0, 8);
const QVariant VListViewRightSubscripItemMargin =
    QVariant::fromValue(ListViweRightSubscriptItemMargin);
const int List_Interval = 10;
DEF_NAMESPACE_END

#endif // V20_DISPLAY_UTILS_H
