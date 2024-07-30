/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kcommandbarmodel_p.h"
#include "kcommandbar.h" // For ActionGroup
#include "kconfigwidgets_debug.h"

#include <KLocalizedString>

#include <QAction>
#include <QMenu>

#include <unordered_set>

QString KCommandBarModel::Item::displayName() const
{
    const QString group = KLocalizedString::removeAcceleratorMarker(groupName);
    const QString command = KLocalizedString::removeAcceleratorMarker(action->text());

    return group + QStringLiteral(": ") + command;
}

KCommandBarModel::KCommandBarModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_clearHistoryAction = new QAction(tr("Clear History"), this);
    m_clearHistoryAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
    connect(m_clearHistoryAction, &QAction::triggered, this, [this]() {
        m_lastTriggered.clear();
    });
}

void fillRows(QList<KCommandBarModel::Item> &rows, const QString &title, const QList<QAction *> &actions, std::unordered_set<QAction *> &uniqueActions)
{
    for (const auto &action : actions) {
        // We don't want diabled actions
        if (!action->isEnabled()) {
            continue;
        }

        // Is this action actually a menu?
        if (auto menu = action->menu()) {
            auto menuActionList = menu->actions();

            // Empty? => Maybe the menu loads action on aboutToShow()?
            if (menuActionList.isEmpty()) {
                Q_EMIT menu->aboutToShow();
                menuActionList = menu->actions();
            }

            const QString menuTitle = menu->title();
            fillRows(rows, menuTitle, menuActionList, uniqueActions);
            continue;
        }

        if (action->text().isEmpty() && !action->isSeparator()) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "Action" << action << "in group" << title << "has no text";
            continue;
        }

        if (uniqueActions.insert(action).second) {
            rows.push_back(KCommandBarModel::Item{title, action, -1});
        }
    }
}

void KCommandBarModel::refresh(const QList<KCommandBar::ActionGroup> &actionGroups)
{
    int totalActions = std::accumulate(actionGroups.begin(), actionGroups.end(), 0, [](int a, const KCommandBar::ActionGroup &ag) {
        return a + ag.actions.count();
    });
    ++totalActions; // for m_clearHistoryAction

    QList<Item> temp_rows;
    std::unordered_set<QAction *> uniqueActions;
    temp_rows.reserve(totalActions);
    for (const auto &ag : actionGroups) {
        const auto &agActions = ag.actions;
        fillRows(temp_rows, ag.name, agActions, uniqueActions);
    }

    temp_rows.push_back({tr("Command Bar"), m_clearHistoryAction, -1});

    /**
     * For each action in last triggered actions,
     *  - Find it in the actions
     *  - Use the score variable to set its score
     *
     * Items in m_lastTriggered are stored in descending order
     * by their usage i.e., the first item in the vector is the most
     * recently invoked action.
     *
     * Here we traverse them in reverse order, i.e., from least recent to
     * most recent and then assign a score to them in a way that most recent
     * ends up having the highest score. Thus when proxy model does the sorting
     * later, most recent item will end up on the top
     */
    int score = 0;
    std::for_each(m_lastTriggered.crbegin(), m_lastTriggered.crend(), [&score, &temp_rows](const QString &act) {
        auto it = std::find_if(temp_rows.begin(), temp_rows.end(), [act](const KCommandBarModel::Item &i) {
            return i.action->text() == act;
        });
        if (it != temp_rows.end()) {
            it->score = score++;
        }
    });

    beginResetModel();
    m_rows = std::move(temp_rows);
    endResetModel();
}

QVariant KCommandBarModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto &entry = m_rows[index.row()];
    const int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (col == Column_Command) {
            return entry.displayName();
        }
        Q_ASSERT(col == Column_Shortcut);
        return entry.action->shortcut().toString();
    case Qt::DecorationRole:
        if (col == Column_Command) {
            return entry.action->icon();
        }
        break;
    case Qt::ToolTipRole: {
        QString toolTip = entry.displayName();
        if (!entry.action->shortcut().isEmpty()) {
            toolTip += QLatin1Char('\n');
            toolTip += entry.action->shortcut().toString();
        }
        return toolTip;
    }
    case Qt::UserRole: {
        return QVariant::fromValue(entry.action);
    }
    case Role::Score:
        return entry.score;
    }

    return {};
}

void KCommandBarModel::actionTriggered(const QString &name)
{
    if (m_lastTriggered.size() == 6) {
        m_lastTriggered.pop_back();
    }
    m_lastTriggered.push_front(name);
}

QStringList KCommandBarModel::lastUsedActions() const
{
    return m_lastTriggered;
}

void KCommandBarModel::setLastUsedActions(const QStringList &actionNames)
{
    m_lastTriggered = actionNames;

    while (m_lastTriggered.size() > 6) {
        m_lastTriggered.pop_back();
    }
}

#include "moc_kcommandbarmodel_p.cpp"
