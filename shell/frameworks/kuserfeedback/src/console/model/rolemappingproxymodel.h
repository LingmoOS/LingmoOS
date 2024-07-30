/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_ROLEMAPPINGPROXYMODEL_H
#define KUSERFEEDBACK_CONSOLE_ROLEMAPPINGPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QHash>

namespace KUserFeedback {
namespace Console {

/** Proxy model to change roles. */
class RoleMappingProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit RoleMappingProxyModel(QObject *parent = nullptr);
    ~RoleMappingProxyModel() override;

    void addRoleMapping(int fromRole, int toRole);

    QVariant data(const QModelIndex & index, int role) const override;

private:
    QHash<int, int> m_roleMapping;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_ROLEMAPPINGPROXYMODEL_H
