// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTRASHMANAGER_H
#define DTRASHMANAGER_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DTrashManagerPrivate;
class LIBDTKCORESHARED_EXPORT DTrashManager : public QObject, public DObject
{
public:
    static DTrashManager *instance();

    bool trashIsEmpty() const;
    bool cleanTrash();
    bool moveToTrash(const QString &filePath, bool followSymlink = false);

protected:
    DTrashManager();

private:
    D_DECLARE_PRIVATE(DTrashManager)
};

DCORE_END_NAMESPACE

#endif // DTRASHMANAGER_H
