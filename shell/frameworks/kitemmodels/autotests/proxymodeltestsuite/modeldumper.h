/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODELDUMPER_H
#define MODELDUMPER_H

#include <QIODevice>

#include "proxymodeltestsuite_export.h"

class QModelIndex;
class QAbstractItemModel;

class PROXYMODELTESTSUITE_EXPORT ModelDumper
{
public:
    ModelDumper();

    QString dumpModel(const QAbstractItemModel *const model) const;
    QString dumpTree(const QAbstractItemModel *const model, const QModelIndex &index) const;
    QString dumpTree(const QAbstractItemModel *const model, const QModelIndex &index, int start, int end) const;

    void dumpModel(const QAbstractItemModel *const model, QIODevice *device) const;
    void dumpTree(const QAbstractItemModel *const model, QIODevice *device, const QModelIndex &index) const;
    void dumpTree(const QAbstractItemModel *const model, QIODevice *device, const QModelIndex &index, int start, int end) const;

private:
    QString dumpLevel(const QAbstractItemModel *const model, const QModelIndex &parent, int level) const;
    QString dumpLevel(const QAbstractItemModel *const model, const QModelIndex &parent, int level, int start, int end) const;
};

#endif
