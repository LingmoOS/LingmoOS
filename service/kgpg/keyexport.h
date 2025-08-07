/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2011 Luis Ángel Fernández Fernández <laffdez@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef KEYEXPORT_H
#define KEYEXPORT_H

#include "ui_keyexport.h"

#include <QDialog>
#include <QStringList>

class KeyExport : public QDialog, public Ui_KeyExport
{
	Q_OBJECT

public:
    explicit KeyExport(QWidget *parent = nullptr, const QStringList &keyservers = QStringList());

protected:
        void accept() override;
};

#endif
