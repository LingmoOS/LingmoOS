/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "oceanstyleplugin.h"
#include "oceanstyle.h"

#include <QApplication>

namespace Ocean
{
//_________________________________________________
QStyle *StylePlugin::create(const QString &key)
{
    if (key.compare(QLatin1String("ocean"), Qt::CaseInsensitive) == 0) {
        return new Style;
    }
    return nullptr;
}

//_________________________________________________
QStringList StylePlugin::keys() const
{
    return QStringList(QStringLiteral("Ocean"));
}

}
