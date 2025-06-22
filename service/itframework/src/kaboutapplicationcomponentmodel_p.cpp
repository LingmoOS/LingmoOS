/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Julius Künzel <jk.kdedev@smartlab.uber.space>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kaboutapplicationcomponentmodel_p.h"
#include "debug.h"

namespace KDEPrivate
{
KAboutApplicationComponentModel::KAboutApplicationComponentModel(const QList<KAboutComponent> &componentList, QObject *parent)
    : QAbstractListModel(parent)
    , m_componentList(componentList)
{
    m_profileList.reserve(componentList.size());
    for (const auto &component : std::as_const(m_componentList)) {
        KAboutApplicationComponentProfile profile =
            KAboutApplicationComponentProfile(component.name(), component.description(), component.version(), component.webAddress(), component.license());
        m_profileList.append(profile);
    }
}

int KAboutApplicationComponentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_componentList.count();
}

QVariant KAboutApplicationComponentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qCWarning(DEBUG_KXMLGUI) << "ERROR: invalid index";
        return QVariant();
    }
    if (index.row() >= rowCount()) {
        qCWarning(DEBUG_KXMLGUI) << "ERROR: index out of bounds";
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        //        qCDebug(DEBUG_KXMLGUI) << "Spitting data for name " << m_profileList.at( index.row() ).name();
        QVariant var;
        var.setValue(m_profileList.at(index.row()));
        return var;
    } else {
        return QVariant();
    }
}

Qt::ItemFlags KAboutApplicationComponentModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

} // namespace KDEPrivate
