/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2008 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "domainmodel.h"
#include "domainbrowser.h"
#include <QStringList>

namespace KDNSSD
{
struct DomainModelPrivate {
    DomainBrowser *m_browser;
};

DomainModel::DomainModel(DomainBrowser *browser, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new DomainModelPrivate)
{
    d->m_browser = browser;
    browser->setParent(this);
    connect(browser, SIGNAL(domainAdded(QString)), this, SIGNAL(layoutChanged()));
    connect(browser, SIGNAL(domainRemoved(QString)), this, SIGNAL(layoutChanged()));
    browser->startBrowse();
}

DomainModel::~DomainModel() = default;

int DomainModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}
int DomainModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : d->m_browser->domains().size();
}

QModelIndex DomainModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QModelIndex DomainModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

bool DomainModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }
    if (column != 0) {
        return false;
    }
    if (row < 0 || row >= rowCount(parent)) {
        return false;
    }
    return true;
}

QVariant DomainModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (!hasIndex(index.row(), index.column(), index.parent())) {
        return QVariant();
    }
    const QStringList domains = d->m_browser->domains();
    if (role == Qt::DisplayRole) {
        return domains[index.row()];
    }
    return QVariant();
}

}

#include "moc_domainmodel.cpp"
