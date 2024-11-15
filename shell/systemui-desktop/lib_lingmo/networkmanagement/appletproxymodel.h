/*
    Copyright 2013-2014 Jan Grulich <jgrulich@redhat.com>
    Copyright 2021 Reven Martin <aj@lingmoos.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef APPLETPROXYMODEL_H
#define APPLETPROXYMODEL_H

#include <networkmanager_export.h>
#include <QSortFilterProxyModel>

#include "networkmodelitem.h"

class NETWORKMANAGER_EXPORT AppletProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QAbstractItemModel * sourceModel READ sourceModel WRITE setSourceModel)

public:
    explicit AppletProxyModel(QObject *parent = nullptr);
    ~AppletProxyModel() override;

    enum Type {
        UnknownType = 0,
        WiredType,
        WirelessType
    };
    Q_ENUM(Type)

    Type type() const;
    void setType(Type type);

signals:
    void typeChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    Type m_type = UnknownType;
};

#endif // APPLETPROXYMODEL_H
