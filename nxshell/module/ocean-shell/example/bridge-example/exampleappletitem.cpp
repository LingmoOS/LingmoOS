// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exampleappletitem.h"

#include <QQueue>

#include <containment.h>
#include <pluginloader.h>
#include <pluginfactory.h>
#include <appletbridge.h>

DS_USE_NAMESPACE

ExampleAppletItem::ExampleAppletItem(QObject *parent)
    : DApplet(parent)
    , m_appsModel(new QIdentityProxyModel(this))
{

}

bool ExampleAppletItem::init()
{
    DApplet::init();

    DAppletBridge bridge("org.lingmo.ds.ocean-apps");
    DAppletProxy * amAppsProxy = bridge.applet();

    if (amAppsProxy) {
        QAbstractItemModel * model = amAppsProxy->property("appModel").value<QAbstractItemModel *>();
        qDebug() << "appModel role names" << model->roleNames();
        m_appsModel->setSourceModel(model);
    } else {
        qWarning() << "Applet `org.lingmo.ds.ocean-apps` not found.";
        qWarning() << "You probably forget the `-p org.lingmo.ds.ocean-apps` argument for ocean-shell";
    }

    return true;
}

D_APPLET_CLASS(ExampleAppletItem)

#include "exampleappletitem.moc"
