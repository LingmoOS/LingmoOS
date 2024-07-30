/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef CODEGENTABLEVIEW_H
#define CODEGENTABLEVIEW_H

#include <QTableView>

class MethodTableView : public QTableView
{
    Q_OBJECT
public:
    MethodTableView(QWidget *parent = 0);
};

class ArgsTableView : public QTableView
{
    Q_OBJECT
public:
    ArgsTableView(QWidget *parent = 0);
};

#endif
