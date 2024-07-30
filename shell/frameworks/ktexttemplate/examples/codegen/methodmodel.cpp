/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "methodmodel.h"

MethodModel::MethodModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderLabels(QStringList() << "Access"
                                            << "virtual"
                                            << "Type"
                                            << "Name"
                                            << "Const");
}

ArgsModel::ArgsModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

QVariant ArgsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return "Type";
        case 1:
            return "Name";
        case 2:
            return "Default";
        }
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

#include "moc_methodmodel.cpp"
