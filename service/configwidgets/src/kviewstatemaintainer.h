/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KVIEWSTATEMAINTAINER_H
#define KVIEWSTATEMAINTAINER_H

#include <KViewStateMaintainerBase>

#include <QAbstractItemView>

#include <KConfigGroup>

/**
 * @class KViewStateMaintainer kviewstatemaintainer.h KViewStateMaintainer
 *
 * @brief Encapsulates the maintenance of state between resets of QAbstractItemModel
 *
 * @code
 *   m_collectionViewStateMaintainer = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(KSharedConfig::openConfig()->group("collectionView"));
 *   m_collectionViewStateMaintainer->setView(m_collectionView);
 *
 *   m_collectionCheckStateMaintainer = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(KSharedConfig::openConfig()->group("collectionCheckState"));
 *   m_collectionCheckStateMaintainer->setSelectionModel(m_checkableProxy->selectionModel());
 * @endcode
 *
 * @see KConfigViewStateSaver
 */
template<typename StateSaver>
class KViewStateMaintainer : public KViewStateMaintainerBase
{
    typedef StateSaver StateRestorer;

public:
    KViewStateMaintainer(const KConfigGroup &configGroup, QObject *parent = nullptr)
        : KViewStateMaintainerBase(parent)
        , m_configGroup(configGroup)
    {
    }

    /* reimp */ void saveState()
    {
        StateSaver saver;
        saver.setView(view());
        saver.setSelectionModel(selectionModel());
        saver.saveState(m_configGroup);
        m_configGroup.sync();
    }

    /* reimp */ void restoreState()
    {
        StateRestorer *restorer = new StateRestorer;
        restorer->setView(view());
        restorer->setSelectionModel(selectionModel());
        restorer->restoreState(m_configGroup);
    }

private:
    KConfigGroup m_configGroup;
};

#endif
