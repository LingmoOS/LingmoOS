// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "osdapplet.h"

#include "pluginfactory.h"
#include <DDBusSender>

OsdApplet::OsdApplet(QObject *parent)
    : DApplet(parent)
{

}

void OsdApplet::sendOsd(const QString &type)
{
    DDBusSender().service("org.lingmo.ocean.Shell")
        .path("/org/lingmo/osdService")
        .interface("org.lingmo.osdService").method("showText").arg(type).call();
}

D_APPLET_CLASS(OsdApplet)

#include "osdapplet.moc"
