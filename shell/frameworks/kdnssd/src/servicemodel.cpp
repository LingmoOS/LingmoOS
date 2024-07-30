/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2008 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "servicemodel.h"
#include "servicebrowser.h"

namespace KDNSSD
{
struct ServiceModelPrivate {
    ServiceBrowser *m_browser;
};

ServiceModel::ServiceModel(ServiceBrowser *browser, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new ServiceModelPrivate)
{
    d->m_browser = browser;
    browser->setParent(this);
    connect(browser, SIGNAL(serviceAdded(KDNSSD::RemoteService::Ptr)), this, SIGNAL(layoutChanged()));
    connect(browser, SIGNAL(serviceRemoved(KDNSSD::RemoteService::Ptr)), this, SIGNAL(layoutChanged()));
    browser->startBrowse();
}

ServiceModel::~ServiceModel() = default;

int ServiceModel::columnCount(const QModelIndex &) const
{
    return d->m_browser->isAutoResolving() ? 3 : 1;
}
int ServiceModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : d->m_browser->services().size();
}

QModelIndex ServiceModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QModelIndex ServiceModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

bool ServiceModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }
    if (column < 0 || column >= columnCount()) {
        return false;
    }
    if (row < 0 || row >= rowCount(parent)) {
        return false;
    }
    return true;
}

QVariant ServiceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (!hasIndex(index.row(), index.column(), index.parent())) {
        return QVariant();
    }
    const QList<RemoteService::Ptr> srv = d->m_browser->services();
    switch ((uint)role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case ServiceName:
            return srv[index.row()]->serviceName();
        case Host:
            return srv[index.row()]->hostName();
        case Port:
            return srv[index.row()]->port();
        }
        break;
    case ServicePtrRole:
        QVariant ret;
        ret.setValue(srv[index.row()]);
        return ret;
    }
    return QVariant();
}

QVariant ServiceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }
    switch (section) {
    case ServiceName:
        return tr("Name");
    case Host:
        return tr("Host");
    case Port:
        return tr("Port");
    }
    return QVariant();
}

}

#include "moc_servicemodel.cpp"
