/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "path-completer.h"
#include "path-bar-model.h"

#include <QDebug>

using namespace Peony;

PathCompleter::PathCompleter(QObject *parent) : QCompleter(parent)
{

}

PathCompleter::PathCompleter(QAbstractItemModel *model, QObject *parent) : QCompleter(parent)
{
    Q_UNUSED(model);
}

QStringList PathCompleter::splitPath(const QString &path) const
{
//    QAbstractItemModel *m = model();
//    PathBarModel* model = static_cast<PathBarModel*>(m);

//    auto l = model->stringList();

//    if (path.endsWith("/")) {
//        model->setRootUri(path);
//    } else {
//        QString tmp0 = path;
//        QString tmp = path;
//        tmp.chop(path.size() - path.lastIndexOf("/"));
//        if (tmp.endsWith("/")) {
//            tmp.append("/");
//        }
//        model->setRootUri(tmp);
//    }

//    auto splitPath = QCompleter::splitPath(path);
    return QCompleter::splitPath(path);
}

QString PathCompleter::pathFromIndex(const QModelIndex &index) const
{
    //qDebug()<<index.data();
    return QCompleter::pathFromIndex(index);
}
