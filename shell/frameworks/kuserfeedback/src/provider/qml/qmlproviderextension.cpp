/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "qmlproviderextension.h"

using namespace KUserFeedback;

QmlProviderExtension::QmlProviderExtension(QObject* parent) :
    QObject(parent)
{
}

QmlProviderExtension::~QmlProviderExtension()
{
}

void QmlProviderExtension::sourceAppend(QQmlListProperty<QmlAbstractDataSource> *prop, QmlAbstractDataSource *value)
{
    static_cast<QmlProviderExtension*>(prop->object)->m_sourceWrappers.push_back(value);
    static_cast<Provider*>(prop->object->parent())->addDataSource(value->source());
}

QmlProviderExtension::sizetype QmlProviderExtension::sourceCount(QQmlListProperty<QmlAbstractDataSource> *prop)
{
    return static_cast<QmlProviderExtension*>(prop->object)->m_sourceWrappers.size();
}

QmlAbstractDataSource* QmlProviderExtension::sourceAt(QQmlListProperty<QmlAbstractDataSource> *prop, QmlProviderExtension::sizetype index)
{
    return static_cast<QmlProviderExtension*>(prop->object)->m_sourceWrappers.at(index);
}

void QmlProviderExtension::sourceClear(QQmlListProperty<QmlAbstractDataSource> *prop)
{
    static_cast<QmlProviderExtension*>(prop->object)->m_sourceWrappers.clear();
}

QQmlListProperty<QmlAbstractDataSource> QmlProviderExtension::sources()
{
    return QQmlListProperty<QmlAbstractDataSource>(this, nullptr,
        QmlProviderExtension::sourceAppend, QmlProviderExtension::sourceCount,
        QmlProviderExtension::sourceAt, QmlProviderExtension::sourceClear);
}

#include "moc_qmlproviderextension.cpp"
