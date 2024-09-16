// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dimfileinfo.h"
#include "dabstractfileinfo.h"

#include "../app/src/corelib/helper.h"
#include "../app/src/corelib/dfilediskinfo.h"
#include "../app/src/corelib/dzlibfile.h"

#include <private/dfileinfo_p.h>
#include <dfilemenumanager.h>
#include <dfilesystemmodel.h>

#include <QAction>

DIMFileInfo::DIMFileInfo(const QString &path)
    : DFileInfo("dim://" + path)
{
    DFileInfoPrivate *d = reinterpret_cast<DFileInfoPrivate*>(qGetPtrHelper(d_ptr));

    d->setUrl(DUrl(d->fileInfo.absoluteFilePath()), true);
}

bool DIMFileInfo::canShare() const
{
    return false;
}

QString DIMFileInfo::fileDisplayName() const
{
    if (partInfo)
        return partInfo.name();

    return DFileInfo::fileDisplayName();
}

DAbstractFileInfo::FileType DIMFileInfo::fileType() const
{
    return Backups;
}

QString DIMFileInfo::mimeTypeDisplayName() const
{
    if (partInfo) {
        return QObject::tr("Partition Image File");
    }

    const QString &name = baseName();

    if (name == "headgear") {
        return QObject::tr("Disk Headgear Data");
    } else if (name == "pt") {
        return QObject::tr("Disk Partition Table Info");
    } else if (name == "info") {
        return QObject::tr("Disk Detail");
    }

    return DFileInfo::mimeTypeDisplayName();
}

QVector<MenuAction> DIMFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> list;

    if (type == SpaceArea) {
        list << MenuAction::DisplayAs
             << MenuAction::SortBy
             << DFMGlobal::Property;

        return list;
    }

    list << DFMGlobal::Property;

    return list;
}

QList<int> DIMFileInfo::userColumnRoles() const
{
    QList<int> roles;

    roles << DFileSystemModel::FileDisplayNameRole
          << DFileSystemModel::FileMimeTypeRole
          << DFileSystemModel::FileSizeRole
             // partition file system type
          << DFileSystemModel::FileUserRole
             // partition label
          << DFileSystemModel::FileUserRole + 1
             // partition total size
          << DFileSystemModel::FileUserRole + 2
             // partition used size
          << DFileSystemModel::FileUserRole + 3;

    return roles;
}

QVariant DIMFileInfo::userColumnDisplayName(int userColumnRole) const
{
    switch (userColumnRole) {
    case DFileSystemModel::FileUserRole:
        return QObject::tr("File System");
    case DFileSystemModel::FileUserRole + 1:
        return QObject::tr("Label");
    case DFileSystemModel::FileUserRole + 2:
        return QObject::tr("Total Size");
    case DFileSystemModel::FileUserRole + 3:
        return QObject::tr("Used Size");
    default:
        break;
    }

    return DFileInfo::userColumnDisplayName(userColumnRole);
}

QVariant DIMFileInfo::userColumnData(int userColumnRole) const
{
    ensureInfo();

    switch (userColumnRole) {
    case DFileSystemModel::FileUserRole:
        return partInfo ? partInfo.fileSystemTypeName() : "";
    case DFileSystemModel::FileUserRole + 1:
        return partInfo ? partInfo.partLabel() : "";
    case DFileSystemModel::FileUserRole + 2:
        return partInfo ? Helper::sizeDisplay(partInfo.totalSize()) : "";
    case DFileSystemModel::FileUserRole + 3:
        return partInfo ? Helper::sizeDisplay(partInfo.usedSize()) : "";
    default:
        break;
    }

    return DFileInfo::userColumnData(userColumnRole);
}

int DIMFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    switch (userColumnRole) {
    case DFileSystemModel::FileMimeTypeRole:
        return qMax(fontMetrics.horizontalAdvance(QObject::tr("Disk Partition Table Info")), DFileInfo::userColumnWidth(userColumnRole, fontMetrics));
    case DFileSystemModel::FileUserRole:
    case DFileSystemModel::FileUserRole + 1:
    case DFileSystemModel::FileUserRole + 2:
    case DFileSystemModel::FileUserRole + 3:
        return fontMetrics.horizontalAdvance(userColumnDisplayName(userColumnRole).toString());
    default:
        break;
    }

    return DFileInfo::userColumnWidth(userColumnRole, fontMetrics);
}

bool DIMFileInfo::columnDefaultVisibleForRole(int role) const
{
    switch (role) {
    case DFileSystemModel::FileUserRole + 1:
        return false;
    case DFileSystemModel::FileUserRole:
    case DFileSystemModel::FileUserRole + 2:
    case DFileSystemModel::FileUserRole + 3:
        return true;
    }

    return DFileInfo::columnDefaultVisibleForRole(role);
}

quint8 DIMFileInfo::supportViewMode() const
{
    return 0x02;
}

QIODevice *DIMFileInfo::createIODevice() const
{
    if (isDir())
        return 0;

    return new DZlibFile(absoluteFilePath());
}

void DIMFileInfo::ensureInfo() const
{
    if (partInfo)
        return;

    bool ok = false;
    int index = baseName().toInt(&ok);

    if (!ok) {
        return;
    }

    DFileDiskInfo disk_info(DUrl(absolutePath()).path());

    if (!disk_info)
        return;

    const QList<DPartInfo> infos = disk_info.childrenPartList();

    if (index <= infos.count())
        const_cast<DIMFileInfo*>(this)->partInfo = infos.at(index - 1);
}
