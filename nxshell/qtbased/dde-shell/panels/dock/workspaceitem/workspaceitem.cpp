// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applet.h"
#include "workspaceitem.h"
#include "pluginfactory.h"

#include <DDBusSender>

namespace dock {

WorkspaceItem::WorkspaceItem(QObject *parent)
    : DApplet(parent)
{

}

WorkspaceModel *WorkspaceItem::dataModel() const
{
    return WorkspaceModel::instance();
}

D_APPLET_CLASS(WorkspaceItem)
}


#include "workspaceitem.moc"
