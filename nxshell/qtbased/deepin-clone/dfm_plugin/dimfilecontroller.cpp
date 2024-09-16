// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dimfilecontroller.h"
#include "dimfileinfo.h"

#include <dfmevent.h>
#include <dfileinfo.h>

#include <QFileInfo>
#include <QDir>
#include <QDebug>

const QList<DAbstractFileInfoPointer> DIMFileController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QList<DAbstractFileInfoPointer> list;
    const QString &base_dim = event->url().toString();

    for (const QString &name : QDir(base_dim).entryList(event->nameFilters(), event->filters())) {
        list << DAbstractFileInfoPointer(new DIMFileInfo(event->url().path() + QDir::separator() + name));
    }

    return list;
}

const DAbstractFileInfoPointer DIMFileController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new DIMFileInfo(event->url().path()));
}
