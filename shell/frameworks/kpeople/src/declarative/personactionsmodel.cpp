/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "actions.h"
#include "kpeople_debug.h"
#include "personactionsmodel_p.h"
#include "persondata.h"
#include <QAction>

namespace KPeople
{
class PersonActionsPrivate
{
public:
    PersonActionsPrivate()
        : person(nullptr)
    {
    }

    QList<QAction *> actions;
    QString id;
    KPeople::PersonData *person;
};
}

using namespace KPeople;

PersonActionsModel::PersonActionsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new PersonActionsPrivate)
{
}

PersonActionsModel::~PersonActionsModel()
{
    delete d_ptr;
}

QHash<int, QByteArray> PersonActionsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[IconNameRole] = "iconName";
    roles[ActionRole] = "action";
    roles[ActionTypeRole] = "actionType";
    return roles;
}

void PersonActionsModel::setPersonUri(const QString &id)
{
    Q_D(PersonActions);

    if (id == d->id) {
        return;
    }

    delete d->person;
    d->id = id;

    if (!id.isEmpty()) {
        d->person = new PersonData(id, this);
        connect(d->person, &PersonData::dataChanged, this, &PersonActionsModel::resetActions);

        resetActions();
    } else {
        beginResetModel();
        d->actions.clear();
        endResetModel();
    }

    Q_EMIT personChanged();
}

void PersonActionsModel::resetActions()
{
    Q_D(PersonActions);

    beginResetModel();
    d->actions = KPeople::actionsForPerson(d->id, this);
    endResetModel();
}

QString PersonActionsModel::personUri() const
{
    Q_D(const PersonActions);
    return d->id;
}

QVariant PersonActionsModel::data(const QModelIndex &index, int role) const
{
    Q_D(const PersonActions);

    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return d->actions[index.row()]->text();
    case Qt::DecorationRole:
        return d->actions[index.row()]->icon();
    case Qt::ToolTip:
        return d->actions[index.row()]->toolTip();
    case IconNameRole:
        return d->actions[index.row()]->icon().name();
    case ActionRole:
        return QVariant::fromValue<QObject *>(d->actions[index.row()]);
    case ActionTypeRole:
        return d->actions[index.row()]->property("actionType");
    }

    return QVariant();
}

int PersonActionsModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const PersonActions);

    return parent.isValid() ? 0 : d->actions.size();
}

void PersonActionsModel::triggerAction(int row) const
{
    Q_D(const PersonActions);
    if (d->actions.count() >= row) {
        qWarning() << "no action in row" << row << ". Actions available:" << d->actions.count();
        return;
    }
    d->actions[row]->trigger();
}

QList<QAction *> PersonActionsModel::actions() const
{
    Q_D(const PersonActions);
    return d->actions;
}

#include "moc_personactionsmodel_p.cpp"
