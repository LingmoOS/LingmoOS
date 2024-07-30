/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2003 Michael v.Ostheim <ostheimm@users.berlios.de>
 */

#include <QDebug>
#include <QFontMetrics>
#include <QFrame>
#include <QLabel>
#include <QString>

#include "displaynumber.h"

DisplayNumber::DisplayNumber(QWidget *parent, int digits, int prec)
    : QLabel(parent)
{
    setPrecision(prec);
    setWidth(digits);

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setBackgroundRole(QPalette::Base);
    setAlignment(Qt::AlignCenter);
    setFocusPolicy(Qt::NoFocus);
}

DisplayNumber::~DisplayNumber()
{
}

void DisplayNumber::setFont(const QFont &f)
{
    QLabel::setFont(f);
    setWidth(dg);
}

void DisplayNumber::setWidth(int digits)
{
    QFontMetrics fm(font());
    QString s(QStringLiteral("0123456789.+-"));
    int width = 0, charWidth = 0;

    for (int i = 0; i < 11; i++, width = fm.boundingRect(s[i]).width()) {
        charWidth = (width > charWidth) ? width : charWidth;
    }

    dg = digits;
    setMinimumWidth(dg * charWidth + charWidth / 2);
}

void DisplayNumber::setNum(double num)
{
    QString text;
    setText(text.setNum(num, 'f', precision));
}

#include "moc_displaynumber.cpp"
