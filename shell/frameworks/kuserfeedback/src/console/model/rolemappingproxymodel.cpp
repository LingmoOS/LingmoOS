/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "rolemappingproxymodel.h"

using namespace KUserFeedback::Console;

RoleMappingProxyModel::RoleMappingProxyModel(QObject* parent) :
    QIdentityProxyModel(parent)
{
}

RoleMappingProxyModel::~RoleMappingProxyModel() = default;

void RoleMappingProxyModel::addRoleMapping(int fromRole, int toRole)
{
    m_roleMapping.insert(fromRole, toRole);
}

QVariant RoleMappingProxyModel::data(const QModelIndex& index, int role) const
{
    const auto it = m_roleMapping.find(role);
    if (it != m_roleMapping.end())
        return QIdentityProxyModel::data(index, it.value());
    return QIdentityProxyModel::data(index, role);
}

#include "moc_rolemappingproxymodel.cpp"
