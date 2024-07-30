/*
    SPDX-FileCopyrightText: 2015 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSONSSORTFILTERPROXYMODEL_H
#define PERSONSSORTFILTERPROXYMODEL_H

#include <QScopedPointer>
#include <QSortFilterProxyModel>
#include <kpeople/kpeople_export.h>

namespace KPeople
{
class PersonsSortFilterProxyModelPrivate;

/**
 * Helps filtering and sorting PersonsModel
 *
 * Especially useful for creating interfaces around specific properties rather
 * than the complete set as a whole.
 *
 * @sa PersonsModel
 * @since 5.12
 */
class KPEOPLE_EXPORT PersonsSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /** Specifies the properties that should be provided by the contact for the contact to be shown. */
    Q_PROPERTY(QStringList requiredProperties READ requiredProperties WRITE setRequiredProperties)
public:
    explicit PersonsSortFilterProxyModel(QObject *parent = nullptr);
    ~PersonsSortFilterProxyModel() override;

    QStringList requiredProperties() const;
    void setRequiredProperties(const QStringList &props);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    Q_INVOKABLE void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    Q_DISABLE_COPY(PersonsSortFilterProxyModel)

    QScopedPointer<PersonsSortFilterProxyModelPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(PersonsSortFilterProxyModel)
};

}

#endif // PERSONSSORTFILTERPROXYMODEL_H
