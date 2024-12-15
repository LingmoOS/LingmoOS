// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCALEFACTORS_H
#define SCALEFACTORS_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, double> ScaleFactors;

void registerScaleFactorsMetaType();

#endif // SCALEFACTORS_H
