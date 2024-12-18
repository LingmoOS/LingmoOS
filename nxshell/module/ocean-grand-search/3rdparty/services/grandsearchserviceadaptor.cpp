/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp com.lingmo.ocean.GrandSearch.xml -i grandsearchservice.h -a grandsearchserviceadaptor -c GrandSearchServiceAdaptor
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "grandsearchserviceadaptor.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class GrandSearchServiceAdaptor
 */

using namespace GrandSearch;

GrandSearchServiceAdaptor::GrandSearchServiceAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

GrandSearchServiceAdaptor::~GrandSearchServiceAdaptor()
{
    // destructor
}

bool GrandSearchServiceAdaptor::IsVisible()
{
    // handle method call com.lingmo.ocean.GrandSearch.IsVisible
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsVisible", Q_RETURN_ARG(bool, out0));
    return out0;
}

void GrandSearchServiceAdaptor::SetVisible(bool visible)
{
    // handle method call com.lingmo.ocean.GrandSearch.SetVisible
    QMetaObject::invokeMethod(parent(), "SetVisible", Q_ARG(bool, visible));
}

