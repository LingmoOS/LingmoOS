/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2003 Michael v.Ostheim <ostheimm@users.berlios.de>
 */

#ifndef DISPLAYNUMBER_H
#define DISPLAYNUMBER_H

#include <qlabel.h>

/**
 *@author Michael v.Ostheim
 */

class DisplayNumber : public QLabel
{
    Q_OBJECT
public:
    explicit DisplayNumber(QWidget *parent = nullptr, int digits = 0, int prec = 0);
    ~DisplayNumber() override;
    void setFont(const QFont &f);
    void setNum(double num);
    void setWidth(int digits);
    void setPrecision(int prec)
    {
        precision = prec;
    }

private:
    int dg, precision;
};

#endif
