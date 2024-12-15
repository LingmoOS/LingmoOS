// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitem.h"

class ItemsPage;
namespace apps {
class AppGroup : public AppItem
{
public:
    explicit AppGroup(const QString &groupId, const QString &name, const QList<QStringList> &appItemIDs);
    ~AppGroup() override;

    int folderId() const;
    QList<QStringList> pages() const;
    ItemsPage * itemsPage();

    static bool idIsFolder(const QString & id);
    static QString groupIdFromNumber(int groupId);
    static int parseGroupId(const QString & id);

private:
    void setItemsPerPage(int number);
    void setFolderId(int folderId);

    ItemsPage * m_itemsPage;
};
}
