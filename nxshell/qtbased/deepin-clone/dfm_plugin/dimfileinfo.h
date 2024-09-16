// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DIMFILEINFO_H
#define DIMFILEINFO_H

#include <dabstractfileinfo.h>
#include <dfileinfo.h>

#include "../app/src/corelib/dpartinfo.h"

class DIMFileInfo : public DFileInfo
{
public:
    explicit DIMFileInfo(const QString &path);

    bool canShare() const override;

    QString fileDisplayName() const override;
    FileType fileType() const override;
    QString mimeTypeDisplayName() const override;

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;

    QList<int> userColumnRoles() const override;
    QVariant userColumnDisplayName(int userColumnRole) const override;
    /// get custom column data
    QVariant userColumnData(int userColumnRole) const override;
    /// get custom column width
    int userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const override;
    /// user column default visible for role
    bool columnDefaultVisibleForRole(int role) const override;

    quint8 supportViewMode() const override;

    QIODevice *createIODevice() const override;

private:
    void ensureInfo() const;

    DPartInfo partInfo;
};

#endif // DIMFILEINFO_H
