/*
    SPDX-FileCopyrightText: 2015 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "personssortfilterproxymodel.h"
#include "backends/abstractcontact.h"
#include "personsmodel.h"

namespace KPeople
{
class PersonsSortFilterProxyModelPrivate
{
public:
    QStringList m_keys;
};

PersonsSortFilterProxyModel::PersonsSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d_ptr(new PersonsSortFilterProxyModelPrivate)
{
}

PersonsSortFilterProxyModel::~PersonsSortFilterProxyModel()
{
}

QStringList PersonsSortFilterProxyModel::requiredProperties() const
{
    Q_D(const PersonsSortFilterProxyModel);
    return d->m_keys;
}

void PersonsSortFilterProxyModel::setRequiredProperties(const QStringList &props)
{
    Q_D(PersonsSortFilterProxyModel);
    d->m_keys = props;
    invalidate();
}

bool PersonsSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_D(const PersonsSortFilterProxyModel);
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    Q_ASSERT(idx.isValid());

    if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent)) {
        return false;
    }

    const AbstractContact::Ptr contact = idx.data(KPeople::PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();
    Q_ASSERT(contact);

    // Don't filter if no keys are set
    if (d->m_keys.isEmpty()) {
        return true;
    }

    for (const QString &key : std::as_const(d->m_keys)) {
        if (!contact->customProperty(key).isNull()) {
            return true;
        }
    }

    return false;
}

void PersonsSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    QSortFilterProxyModel::sort(column, order);
}
}

#include "moc_personssortfilterproxymodel.cpp"
