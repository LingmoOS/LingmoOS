/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modeldumper.h"

#include <QAbstractItemModel>

ModelDumper::ModelDumper()
{
}

static int num;

void ModelDumper::dumpModel(const QAbstractItemModel *const model, QIODevice *device) const
{
    num = 1;

    device->write(dumpLevel(model, QModelIndex(), 1).toLatin1());
}

QString ModelDumper::dumpModel(const QAbstractItemModel *const model) const
{
    num = 1;
    return dumpLevel(model, QModelIndex(), 1);
}

QString ModelDumper::dumpTree(const QAbstractItemModel *const model, const QModelIndex &index) const
{
    num = 1;
    return dumpLevel(model, index, 1);
}

QString ModelDumper::dumpTree(const QAbstractItemModel *const model, const QModelIndex &index, int start, int end) const
{
    num = 1;
    return dumpLevel(model, index, 1, start, end);
}

void ModelDumper::dumpTree(const QAbstractItemModel *const model, QIODevice *device, const QModelIndex &index) const
{
    num = 1;
    device->write(dumpLevel(model, index, 1).toLatin1());
}

void ModelDumper::dumpTree(const QAbstractItemModel *const model, QIODevice *device, const QModelIndex &index, int start, int end) const
{
    num = 1;
    device->write(dumpLevel(model, index, 1, start, end).toLatin1());
}

QString ModelDumper::dumpLevel(const QAbstractItemModel *const model, const QModelIndex &parent, int level) const
{
    const int rowCount = model->rowCount(parent);
    return dumpLevel(model, parent, level, 0, rowCount - 1);
}

QString ModelDumper::dumpLevel(const QAbstractItemModel *const model, const QModelIndex &parent, int level, int start, int end) const
{
    QString lines;
    for (int row = start; row <= end; ++row) {
        QString line;
        line.append("\"");
        for (int l = 0; l < level; ++l) {
            line.append("- ");
        }
        line.append(QString::number(num++));
        line.append("\"");
        line.append("\n");
        lines.append(line);
        static const int column = 0;
        const QModelIndex idx = model->index(row, column, parent);
        if (model->hasChildren(idx)) {
            lines.append(dumpLevel(model, idx, level + 1));
        }
    }
    return lines;
}
