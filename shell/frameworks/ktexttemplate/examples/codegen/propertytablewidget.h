/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef PROPERTYTABLEWIDGET_H
#define PROPERTYTABLEWIDGET_H

#include <QTableWidget>

class PropertyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    PropertyTableWidget(QWidget *parent = 0);

    virtual void contextMenuEvent(QContextMenuEvent *contextMenuEvent);

public Q_SLOTS:
    void slotAdd();
    void slotRemove();
    void slotClear();
};

#endif
