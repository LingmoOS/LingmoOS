/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>
    SPDX-FileCopyrightText: 2016 Ableton AG <info@ableton.com>
    SPDX-FileContributor: Stephen Kelly <stephen.kelly@ableton.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMODELINDEXPROXYMAPPER_H
#define KMODELINDEXPROXYMAPPER_H

#include <QObject>

#include "kitemmodels_export.h"

#include <memory>

class QAbstractItemModel;
class QModelIndex;
class QItemSelection;
class KModelIndexProxyMapperPrivate;

/**
 * @class KModelIndexProxyMapper kmodelindexproxymapper.h KModelIndexProxyMapper
 *
 * @brief This class facilitates easy mapping of indexes and selections through proxy models.
 *
 * In a complex system of proxy models there can be a need to map indexes and selections between them,
 * and sometimes to do so without knowledge of the path from one model to another.
 *
 * For example,
 *
 * @verbatim
 *     Root model
 *         |
 *       /    \
 *   Proxy 1   Proxy 3
 *      |       |
 *   Proxy 2   Proxy 4
 * @endverbatim
 *
 * If there is a need to map indexes between proxy 2 and proxy 4, a KModelIndexProxyMapper can be created
 * to facilitate mapping of indexes between them.
 *
 * @code
 *   m_indexMapper = new KModelIndexProxyMapper(proxy2, proxy4, this);
 *
 *  ...
 *
 *   const QModelIndex proxy4Index = m_mapLeftToRight(proxy2->index(0, 0));
 *   Q_ASSERT(proxy4Index.model() == proxy4);
 * @endcode
 *
 * Note that the aim is to achieve black box connections so that there is no need for application code to
 * know the structure of proxy models in the path between left and right and attempt to manually map them.
 *
 * @verbatim
 *     Root model
 *         |
 *   ---------------
 *   |  Black Box  |
 *   ---------------
 *      |       |
 *   Proxy 2   Proxy 4
 * @endverbatim
 *
 * The isConnected property indicates whether there is a
 * path from the left side to the right side.
 *
 * @author Stephen Kelly <steveire@gmail.com>
 *
 */
class KITEMMODELS_EXPORT KModelIndexProxyMapper : public QObject
{
    Q_OBJECT

    /**
     * Indicates whether there is a chain that can be followed from leftModel to rightModel.
     *
     * This value can change if the sourceModel of an intermediate proxy is changed.
     */
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
public:
    /**
     * Constructor
     */
    KModelIndexProxyMapper(const QAbstractItemModel *leftModel, const QAbstractItemModel *rightModel, QObject *parent = nullptr);

    ~KModelIndexProxyMapper() override;

    /**
     * Maps the @p index from the left model to the right model.
     */
    QModelIndex mapLeftToRight(const QModelIndex &index) const;

    /**
     * Maps the @p index from the right model to the left model.
     */
    QModelIndex mapRightToLeft(const QModelIndex &index) const;

    /**
     * Maps the @p selection from the left model to the right model.
     */
    QItemSelection mapSelectionLeftToRight(const QItemSelection &selection) const;

    /**
     * Maps the @p selection from the right model to the left model.
     */
    QItemSelection mapSelectionRightToLeft(const QItemSelection &selection) const;

    bool isConnected() const;

Q_SIGNALS:
    void isConnectedChanged();

private:
    //@cond PRIVATE
    Q_DECLARE_PRIVATE(KModelIndexProxyMapper)
    std::unique_ptr<KModelIndexProxyMapperPrivate> const d_ptr;
    //@endcond
};

#endif
