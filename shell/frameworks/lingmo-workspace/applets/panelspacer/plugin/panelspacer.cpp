/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panelspacer.h"

#include <QDebug>
#include <QProcess>

#include <Lingmo/Containment>
#include <Lingmo/Corona>
#include <LingmoQuick/AppletQuickItem>

PanelSpacer::PanelSpacer(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Lingmo::Applet(parent, data, args)
{
}

LingmoQuick::AppletQuickItem *PanelSpacer::containmentGraphicObject() const
{
    if (!containment())
        return nullptr; // Return nothing if there is no containment to prevent a Segmentation Fault
    return LingmoQuick::AppletQuickItem::itemForApplet(containment());
}

K_PLUGIN_CLASS(PanelSpacer)

#include "panelspacer.moc"
