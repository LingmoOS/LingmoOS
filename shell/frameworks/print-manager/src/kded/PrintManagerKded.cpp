/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrintManagerKded.h"
#include "NewPrinterNotification.h"
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PrintManagerKded, "printmanager.json")

PrintManagerKded::PrintManagerKded(QObject *parent, const QVariantList &args)
    : KDEDModule(parent)
{
    Q_UNUSED(args)

    new NewPrinterNotification(this);
}

PrintManagerKded::~PrintManagerKded()
{
}

#include "PrintManagerKded.moc"

#include "moc_PrintManagerKded.cpp"
