/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2008 Jakub Stachowski <qbast@go2.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDNSSDDOMAINMODEL_H
#define KDNSSDDOMAINMODEL_H

#include "kdnssd_export.h"
#include <QAbstractItemModel>
#include <memory>

namespace KDNSSD
{
struct DomainModelPrivate;
class DomainBrowser;

/**
 * @class DomainModel domainmodel.h KDNSSD/DomainModel
 * @short Model for list of Zeroconf domains
 *
 * This class provides a Qt Model for DomainBrowser to allow easy
 * integration of domain discovery into a GUI.  For example, to
 * provide a combo box listing available domains, you can do:
 * @code
 * KDNSSD::DomainModel *domainModel = new DomainModel(
 *     new KDNSSD::DomainBrowser(KDNSSD::DomainBrowser::Browsing)
 *     );
 * QComboBox *domainCombo = new QComboBox();
 * domainCombo->setModel(domainModel);
 * @endcode
 *
 * @since 4.1
 * @author Jakub Stachowski
 */

class KDNSSD_EXPORT DomainModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /**
     * Creates a model for given domain browser and starts
     * browsing for domains.
     *
     * The model takes ownership of the browser,
     * so there is no need to delete it afterwards.
     *
     * You should @b not call DomainBrowser::startBrowse() on @p browser
     * before passing it to DomainModel.
     *
     * @param browser the domain browser that will provide the domains
     *                to be listed by the model
     * @param parent  the parent object (see QObject documentation)
     */
    explicit DomainModel(DomainBrowser *browser, QObject *parent = nullptr);

    ~DomainModel() override;

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
    virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;

private:
    std::unique_ptr<DomainModelPrivate> const d;
    friend struct DomainModelPrivate;
};

}

#endif
