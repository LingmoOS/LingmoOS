/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Lingmo/Applet>
#include <LingmoQuick/AppletQuickItem>

namespace Lingmo
{
class Containment;
}

class PanelSpacer;

class PanelSpacer : public Lingmo::Applet
{
    Q_OBJECT
    Q_PROPERTY(LingmoQuick::AppletQuickItem *containmentItem READ containmentGraphicObject CONSTANT)

public:
    PanelSpacer(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    LingmoQuick::AppletQuickItem *containmentGraphicObject() const;
};
