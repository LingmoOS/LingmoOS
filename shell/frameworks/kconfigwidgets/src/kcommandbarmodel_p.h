/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCOMMANDBARMODEL_H
#define KCOMMANDBARMODEL_H

#include "kcommandbar.h"

#include <QAbstractTableModel>
#include <QList>

class QAction;

class KCommandBarModel final : public QAbstractTableModel
{
    Q_OBJECT
public:
    struct Item {
        QString displayName() const;

        QString groupName;
        QAction *action;
        int score;
    };

    KCommandBarModel(QObject *parent = nullptr);

    enum Role { Score = Qt::UserRole + 1 };

    enum Column { Column_Command, Column_Shortcut, Column_Count };

    /**
     * Resets the model
     *
     * If you are using last Used actions functionality, make sure
     * to set the last used actions before calling this function
     */
    void refresh(const QList<KCommandBar::ActionGroup> &actionGroups);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return m_rows.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return Column_Count;
    }

    /**
     * reimplemented function to update score that is calculated by KFuzzyMatcher
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!index.isValid())
            return false;
        if (role == Role::Score) {
            auto row = index.row();
            m_rows[row].score = value.toInt();
        }
        return QAbstractTableModel::setData(index, value, role);
    }

    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * action with name @p name was triggered, store it in m_lastTriggered
     */
    void actionTriggered(const QString &name);

    /**
     * last used actions
     * max = 6;
     */
    QStringList lastUsedActions() const;

    /**
     * incoming lastUsedActions
     *
     * should be set before calling refresh()
     */
    void setLastUsedActions(const QStringList &actionNames);

private:
    QList<Item> m_rows;

    /**
     * Last triggered actions by user
     *
     * Ordered descending i.e., least recently invoked
     * action is at the end
     */
    QStringList m_lastTriggered;

    QAction *m_clearHistoryAction;
};

Q_DECLARE_TYPEINFO(KCommandBarModel::Item, Q_RELOCATABLE_TYPE);

#endif // KCOMMANDBARMODEL_H
