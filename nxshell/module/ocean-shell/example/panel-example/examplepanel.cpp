// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "examplepanel.h"

#include "pluginfactory.h"
#include "appletbridge.h"

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

    DAppletBridge bridge("org.lingmo.ds.example.applet-data");

    qDebug() << "It's state of the bridge:" << bridge.isValid();
    if (auto applet = bridge.applet()) {
        qDebug() << "Get property:" << applet->property("mainText");
        QString id("call");
        qDebug() << "Invoke argument:" << id;
        qDebug() << "Invoke method staus:" << QMetaObject::invokeMethod(applet, "call", Qt::DirectConnection,
                                  Q_RETURN_ARG(QString, id), Q_ARG(const QString&, id));
        qDebug() << "Invoked returd value:" << id;

        QObject::connect(applet, SIGNAL(sendSignal(const QString &)), SLOT(onReceivedSignal(const QString &)));

        QMetaObject::invokeMethod(applet, "sendSignal", Qt::DirectConnection, Q_ARG(const QString&, id));
    }

    {
        DAppletBridge bridge("org.lingmo.ds.example.containment");
        qDebug() << "Customize MetaObject of the applet:" << bridge.isValid();
        if (auto applet = bridge.applet()) {
            QString id("call");
            qDebug() << "Invoke argument:" << id;
            qDebug() << "Invoke method staus:" << QMetaObject::invokeMethod(applet, "call", Qt::DirectConnection,
                                      Q_RETURN_ARG(QString, id), Q_ARG(const QString&, id));
            qDebug() << "Invoked returd value:" << id;
        }
    }

    QObject::connect(this, &DApplet::rootObjectChanged, this, [this]() {
        Q_ASSERT(rootObject());
        Q_ASSERT(window());
    });

    return true;
}

void ExamplePanel::onReceivedSignal(const QString &id)
{
    qDebug() << "Received signal:" << id;
}

D_APPLET_CLASS(ExamplePanel)

#include "examplepanel.moc"
