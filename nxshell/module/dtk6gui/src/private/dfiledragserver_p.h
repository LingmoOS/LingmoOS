// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEDRAGSERVER_P_H
#define DFILEDRAGSERVER_P_H

#include "dfiledragserver.h"

#include <DObjectPrivate>

#include <QMap>
#include <QUuid>

DGUI_BEGIN_NAMESPACE

class DDndSourceInterface;
class DFileDragServer;

class DFileDragServerPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    QMap<QString, QVariant> data;
    QUuid uuid;

    DFileDragServerPrivate(DFileDragServer *q);
    ~DFileDragServerPrivate();

    void writeMimeData(QMimeData *dest);

    static DDndSourceInterface *dbusif;
    static int refcnt;
    static QHash<QString, DFileDragServer*> servermap;

    D_DECLARE_PUBLIC(DFileDragServer)
    friend class DDndSourceInterface;
    friend class DFileDrag;
};
DGUI_END_NAMESPACE

#endif // DFILEDRAGSERVER_P_H
