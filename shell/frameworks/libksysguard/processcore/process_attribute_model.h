/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

#include "processcore_export.h"

namespace KSysGuard
{
class ExtendedProcesses;
class ProcessAttribute;

/**
 * Presents a list of available attributes that can be
 * enabled on a ProceessDataModel
 */
class PROCESSCORE_EXPORT ProcessAttributeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Name = Qt::DisplayRole, /// Human readable translated name of the attribute
        Id = Qt::UserRole, /// Computer readable ID of the attribute
        ShortName = Qt::UserRole + 1, /// A shorter human readable translated name of the attribute
        Description, /// A longer, sentence-based description of the attribute
        Unit, /// The unit, of type KSysGuard::Unit
        Minimum, /// Smallest value this attribute can be in normal situations. A hint for graphing utilities
        Maximum, /// Largest value this attribute can be in normal situations. A hint for graphing utilities
    };
    Q_ENUM(Role)

    ProcessAttributeModel(const QList<ProcessAttribute *> &attributes, QObject *parent = nullptr);
    ~ProcessAttributeModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
