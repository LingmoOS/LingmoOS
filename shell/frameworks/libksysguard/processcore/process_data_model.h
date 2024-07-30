/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractItemModel>
#include <processcore/processes.h>

#include "processcore_export.h"

namespace KSysGuard
{
class Process;
class ProcessAttributeModel;

/**
 * This class contains a model of all running processes
 * Rows represent processes
 * Columns represent a specific attribute, such as CPU usage
 * Attributes can be enabled or disabled
 *
 * This class abstracts the process data so that it can be presented without the client
 * needing to understand the semantics of each column
 * It is designed to be consumable by a QML API
 */
class PROCESSCORE_EXPORT ProcessDataModel : public QAbstractItemModel
{
    Q_OBJECT

    /**
     * A list of ids of all available attributes.
     */
    Q_PROPERTY(QStringList availableAttributes READ availableAttributes CONSTANT)
    /**
     * A list of attributes that should be displayed by this model.
     *
     * Each attribute will correspond to a column, assuming the attribute exists.
     * \property availableAttributes provides a list of all attributes that are
     * available.
     *
     * By default, this is empty and thus nothing will be shown.
     */
    Q_PROPERTY(QStringList enabledAttributes READ enabledAttributes WRITE setEnabledAttributes NOTIFY enabledAttributesChanged)
    /**
     * Provides an instance of a model that lists all available attributes for this model.
     *
     * It provides extra information on top of the list of ids in availableAttributes.
     *
     * \sa ProcessAttributeModel
     */
    Q_PROPERTY(QAbstractItemModel *attributesModel READ attributesModel CONSTANT)
    /**
     * Should this model be updated or not. Defaults to true.
     */
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    /**
     * If true this model is a flat list, otherwise is a tree following the process tree structure. Default is true
     */
    Q_PROPERTY(bool flatList READ flatList WRITE setFlatList NOTIFY flatListChanged)

public:
    enum AdditionalRoles {
        Value = Qt::UserRole, /// The raw value of the attribute. This is unformatted and could represent an int, real or string
        FormattedValue, /// A string containing the value in a locale friendly way with appropriate suffix "eg. 5Mb" "20%"

        PIDs, /// The PIDs associated with this row
        Minimum, /// Smallest value this reading can be in normal situations. A hint for graphing utilities
        Maximum, /// Largest value this reading can be in normal situations. A hint for graphing utilities

        Attribute, /// The attribute id associated with this column
        Name, /// The full name of this attribute
        ShortName, /// A shorter name of this attribute, compressed for viewing
        Unit, /// The unit associated with this attribute. Returned value is of the type KSysGuard::Unit

        UpdateInterval, /// The amount of time in milliseconds between each update of the model.
    };
    Q_ENUM(AdditionalRoles)

    explicit ProcessDataModel(QObject *parent = nullptr);
    ~ProcessDataModel() override;

    /**
     * A list of attribute IDs that can be enabled
     */
    QStringList availableAttributes() const;

    /**
     * The list of available attributes that can be enabled, presented as a model
     * See @availableAttributes
     */
    ProcessAttributeModel *attributesModel();

    /**
     * The currently enabled attributes
     */
    QStringList enabledAttributes() const;
    /**
     * Select which process attributes should be enabled
     * The order determines the column order
     *
     * The default value is empty
     */
    void setEnabledAttributes(const QStringList &enabledAttributes);

    bool enabled() const;
    void setEnabled(bool newEnabled);
    Q_SIGNAL void enabledChanged();

    bool flatList() const;
    void setFlatList(bool flat);
    Q_SIGNAL void flatListChanged();

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

Q_SIGNALS:
    void enabledAttributesChanged();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
