// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARRAYINT_H
#define ARRAYINT_H

#include <QDBusMetaType>

using ArrayInt = QList<int>;
Q_DECLARE_METATYPE(ArrayInt);

void registerArrayIntMetaType();

#endif // ARRAYINT_H
