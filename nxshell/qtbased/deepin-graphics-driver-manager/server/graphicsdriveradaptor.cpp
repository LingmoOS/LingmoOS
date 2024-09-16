// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsdriveradaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class GraphicsDriverAdaptor
 */

GraphicsDriverAdaptor::GraphicsDriverAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

GraphicsDriverAdaptor::~GraphicsDriverAdaptor()
{
    // destructor
}

void GraphicsDriverAdaptor::CancelInstall()
{
    // handle method call com.deepin.daemon.GraphicsDriver.CancelInstall
    QMetaObject::invokeMethod(parent(), "CancelInstall");
}

QString GraphicsDriverAdaptor::GetCurrDriverName()
{
    // handle method call com.deepin.daemon.GraphicsDriver.GetCurrDriverName
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetCurrDriverName", Q_RETURN_ARG(QString, out0));
    return out0;
}

QString GraphicsDriverAdaptor::GetDevice()
{
    // handle method call com.deepin.daemon.GraphicsDriver.GetDevice
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetDevice", Q_RETURN_ARG(QString, out0));
    return out0;
}

QString GraphicsDriverAdaptor::GetNewDriverName()
{
    // handle method call com.deepin.daemon.GraphicsDriver.GetNewDriverName
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetNewDriverName", Q_RETURN_ARG(QString, out0));
    return out0;
}

QString GraphicsDriverAdaptor::GetOldDriverName()
{
    // handle method call com.deepin.daemon.GraphicsDriver.GetOldDriverName
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetOldDriverName", Q_RETURN_ARG(QString, out0));
    return out0;
}

QString GraphicsDriverAdaptor::GetResolutionTitle()
{
    // handle method call com.deepin.daemon.GraphicsDriver.GetResolutionTitle
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetResolutionTitle", Q_RETURN_ARG(QString, out0));
    return out0;
}

bool GraphicsDriverAdaptor::IsTestSuccess()
{
    // handle method call com.deepin.daemon.GraphicsDriver.IsTestSuccess
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsTestSuccess", Q_RETURN_ARG(bool, out0));
    return out0;
}

void GraphicsDriverAdaptor::PrepareInstall(const QString &name, const QString &language)
{
    // handle method call com.deepin.daemon.GraphicsDriver.PrepareInstall
    QMetaObject::invokeMethod(parent(), "PrepareInstall", Q_ARG(QString, name), Q_ARG(QString, language));
}

void GraphicsDriverAdaptor::RealInstall()
{
    // handle method call com.deepin.daemon.GraphicsDriver.RealInstall
    QMetaObject::invokeMethod(parent(), "RealInstall");
}

void GraphicsDriverAdaptor::TestInstall()
{
    // handle method call com.deepin.daemon.GraphicsDriver.TestInstall
    QMetaObject::invokeMethod(parent(), "TestInstall");
}

void GraphicsDriverAdaptor::TestSuccess()
{
    // handle method call com.deepin.daemon.GraphicsDriver.TestSuccess
    QMetaObject::invokeMethod(parent(), "TestSuccess");
}

