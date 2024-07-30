/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractItemModel>

#include "process_attribute_model.h"

#include "processcore_export.h"

namespace KSysGuard
{
class CGroup;

class CGroupDataModelPrivate;

/**
 * @brief The CGroupDataModel class is a list model of all cgroups from a given root
 * Data is exposed as per ProcessDataModel with configurable columns
 *
 * Data is refreshed on a timer
 */
class PROCESSCORE_EXPORT CGroupDataModel : public QAbstractItemModel
{
    Q_OBJECT
    /**
     * @copydoc ProcessDataModel::availableAttributes
     */
    Q_PROPERTY(QStringList availableAttributes READ availableAttributes CONSTANT)
    /**
     * @copydoc ProcessDataModel::enabledAttributes
     */
    Q_PROPERTY(QStringList enabledAttributes READ enabledAttributes WRITE setEnabledAttributes NOTIFY enabledAttributesChanged)
    /**
     * @copydoc ProcessDataModel::attributesModel
     */
    Q_PROPERTY(QObject *attributesModel READ attributesModel CONSTANT)
    /**
     * @copydoc ProcessDataModel::enabled
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    /**
     * @copydoc setRoot
     */
    Q_PROPERTY(QString setRoot READ root WRITE setRoot NOTIFY rootChanged)

    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)

public:
    CGroupDataModel(QObject *parent = nullptr);
    CGroupDataModel(const QString &root, QObject *parent = nullptr);
    ~CGroupDataModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @copydoc ProcessDataModel::availableAttributes
     */
    QStringList availableAttributes() const;
    /**
     * @copydoc ProcessDataModel::enabledAttributes
     */
    QStringList enabledAttributes() const;
    /**
     * @copydoc ProcessDataModel::setEnabledAttributes
     */
    void setEnabledAttributes(const QStringList &enabledAttributes);

    QModelIndex getQModelIndex(CGroup *cgroup, int column) const;

    /**
     * @copydoc ProcessDataModel::attributesModel
     */
    ProcessAttributeModel *attributesModel();
    /**
     * @copydoc ProcessDataModel::isEnabled
     */
    bool isEnabled() const;
    /**
     * @copydoc ProcessDataModel::setEnabled
     */
    void setEnabled(bool isEnabled);

    QString root() const;
    /**
     * Set the root cgroup to start listing from
     * e.g "user.slice/user-1000.slice"
     *
     * The default is blank
     */
    void setRoot(const QString &root);

    /**
     * Trigger an update of the model
     */
    void update();

    bool isAvailable() const;

Q_SIGNALS:
    void enabledAttributesChanged();
    void enabledChanged();
    void rootChanged();
    void availableChanged();

protected:
    virtual bool filterAcceptsCGroup(const QString &id);

private:
    QScopedPointer<CGroupDataModelPrivate> d;
    void update(CGroup *node);
};

}
