/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef METHOD_MODEL_H
#define METHOD_MODEL_H

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class MethodModel : public QStandardItemModel
{
    Q_OBJECT
public:
    MethodModel(QObject *parent = 0);
};

class ArgsModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ArgsModel(QObject *parent = 0);

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

#endif
