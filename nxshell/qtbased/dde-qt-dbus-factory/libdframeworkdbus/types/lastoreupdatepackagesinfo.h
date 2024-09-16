// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LASTOREUPDATEPACKAGESINFO_H
#define LASTOREUPDATEPACKAGESINFO_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, QStringList> LastoreUpdatePackagesInfo;

void registerLastoreUpdatePackagesInfoMetaType();


#endif // LASTOREUPDATEPACKAGESINFO_H
