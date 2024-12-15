// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROLES_H
#define ROLES_H

#include <QObject>

class Roles : public QObject
{
    Q_OBJECT
    Q_ENUMS(RoleNames)
public:
    using QObject::QObject;
    ~Roles() = default;
    enum RoleNames { BlankRole = Qt::UserRole + 1, DBImgInfoRole, Thumbnail, ReloadThumbnail, ItemTypeRole, SelectedRole, SourceIndex
    , ModelTypeRole, FileNameRole, UrlRole, FilePathRole, PathHashRole, RemainDaysRole
                   };
};

#endif
