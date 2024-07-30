/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2008 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDSERVICEMODEL_H
#define KDNSSDSERVICEMODEL_H

#include "kdnssd_export.h"
#include "remoteservice.h"
#include <QAbstractItemModel>
#include <memory>

namespace KDNSSD
{
struct ServiceModelPrivate;
class ServiceBrowser;

/**
 * @class ServiceModel servicemodel.h KDNSSD/ServiceModel
 * @short Model for list of Zeroconf services
 *
 * This class provides a Qt Model for ServiceBrowser to allow easy
 * integration of service discovery into a GUI.  For example, to
 * show the HTTP servers published on the local network, you can do:
 * @code
 * KDNSSD::ServiceModel *serviceModel = new ServiceModel(
 *     new KDNSSD::ServiceBrowser("_http._tcp")
 *     );
 * QComboBox *serviceCombo = new QComboBox();
 * serviceCombo->setModel(serviceModel);
 * @endcode
 *
 * After the user makes a selection, the application typically needs
 * to get a pointer to the selected service in order to get the host
 * name and port.  A RemoteService::Ptr can be obtained from
 * a QModelIndex using:
 * @code
 * void onSelected(const QModelIndex &selection) {
 *     KDNSSD::RemoteService::Ptr service =
 *         selection.data(KDNSSD::ServiceModel::ServicePtrRole)
 *                  .value<KDNSSD::RemoteService::Ptr>();
 * }
 * @endcode
 *
 * @since 4.1
 * @author Jakub Stachowski
 */

class KDNSSD_EXPORT ServiceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /** The additional data roles provided by this model */
    enum AdditionalRoles {
        ServicePtrRole = 0x7E6519DE, ///< gets a RemoteService::Ptr for the service
    };

    /**
     * The default columns for this model.
     *
     * If service browser is not set to resolve automatically,
     * then the model will only ever have one column (the service name).
     */
    enum ModelColumns {
        ServiceName = 0,
        Host = 1,
        Port = 2,
    };

    /**
     * Creates a model for the given service browser and starts browsing
     * for services.
     *
     * The model takes ownership of the browser,
     * so there is no need to delete it afterwards.
     *
     * You should @b not call ServiceBrowser::startBrowse() on @p browser
     * before passing it to ServiceModel.
     */
    explicit ServiceModel(ServiceBrowser *browser, QObject *parent = nullptr);

    ~ServiceModel() override;

    /** @reimp */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /** @reimp */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /** @reimp */
    QModelIndex parent(const QModelIndex &index) const override;
    /** @reimp */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    /** @reimp */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /** @reimp */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    /** @reimp */
    virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;

private:
    std::unique_ptr<ServiceModelPrivate> const d;
    friend struct ServiceModelPrivate;
};

}

#endif
