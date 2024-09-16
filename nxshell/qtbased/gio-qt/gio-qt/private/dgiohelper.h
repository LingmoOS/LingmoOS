// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOHELPER_H
#define DGIOHELPER_H

#include <giomm/icon.h>

#include <QStringList>

namespace DGioPrivate {
    QStringList getThemedIconNames(Glib::RefPtr<const Gio::Icon> icon);
    gchar * converToGChar(const QByteArray& array);
}

#endif // DGIOHELPER_H
