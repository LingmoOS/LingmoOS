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

#ifndef PATHCOMPLETER_H
#define PATHCOMPLETER_H

#include "explorer-core_global.h"
#include <QCompleter>

namespace Peony {

/*!
 * \brief The PathCompleter class
 * \details
 * This class is used to complete the path based on gvfs.
 * Note that the explorer's file system model is desgining-different from
 * QFileSystemModel, so that the QCompleter should aslo do some special
 * treatments.
 * PathCompleter binds with a derived QStringListModel, the PathModel.
 * This model dynamiclly changes its contents when PathCompleter changed
 * its split string. The most cases of that is the line edit which bind with
 * a completer changed its texts. Completer will trigger the splitPath()
 * method, then the model will decided if its contents need be refereshed.
 * \see PathModel
 */
class PEONYCORESHARED_EXPORT PathCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit PathCompleter(QObject *parent = nullptr);
    explicit PathCompleter(QAbstractItemModel *model, QObject *parent = nullptr);

protected:
    QStringList splitPath(const QString &path) const override;
    QString pathFromIndex(const QModelIndex &index) const override;
};

}

#endif // PATHCOMPLETER_H
