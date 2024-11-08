/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <lingmo/lingmo.h>

#include <QAction>
#include <QApplication>
#include <QMenu>

#include "containment.h"

namespace Lingmo
{
Types::Types(QObject *parent)
    : QObject(parent)
{
}

Types::~Types()
{
}

} // Lingmo namespace

#include "moc_lingmo.cpp"
