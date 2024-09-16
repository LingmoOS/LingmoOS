// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "examplepanel.h"

#include "pluginfactory.h"

ExamplePanel::ExamplePanel(QObject *parent)
    : DPanel(parent)
{
}

bool ExamplePanel::load()
{
    return DPanel::load();
}

bool ExamplePanel::init()
{
    DPanel::init();
    QObject::connect(this, &DApplet::rootObjectChanged, this, [this]() {
        Q_ASSERT(rootObject());
        Q_ASSERT(window());
    });

    return true;
}

D_APPLET_CLASS(ExamplePanel)

#include "examplepanel.moc"
