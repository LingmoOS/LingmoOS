/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_PAGE_H
#define PRINTER_PAGE_H

#include <KCupsPrinter.h>

class PrinterPage : public QWidget
{
    Q_OBJECT
public:
    explicit PrinterPage(QWidget *parent = nullptr);
    virtual bool hasChanges()
    {
        return false;
    }

public:
    virtual void save()
    {
    }
    virtual QVariantMap modifiedValues() const;
    virtual QStringList neededValues() const
    {
        return QStringList();
    }
    virtual void setRemote(bool remote);

signals:
    void changed(bool hasChanges);
};

#endif
