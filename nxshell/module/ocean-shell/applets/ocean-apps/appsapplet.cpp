// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsapplet.h"
#include "amappitemmodel.h"
#include "appgroupmanager.h"
#include "pluginfactory.h"

#include <DUtil>
#include <QtConcurrent>

namespace apps
{
AppsApplet::AppsApplet(QObject *parent)
    : DApplet(parent)
    , m_appModel(new AMAppItemModel(this))
    , m_groupModel(new AppGroupManager(m_appModel, this))
{
}

AppsApplet::~AppsApplet()
{

}

bool AppsApplet::load()
{
    return true;
}

QAbstractItemModel *AppsApplet::groupModel() const
{
    return m_groupModel;
}

QAbstractItemModel *AppsApplet::appModel() const
{
    return m_appModel;
}

D_APPLET_CLASS(AppsApplet)
}

#include "appsapplet.moc"
