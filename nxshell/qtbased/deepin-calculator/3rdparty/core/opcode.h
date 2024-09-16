// This file is part of the SpeedCrunch project
// Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2008, 2009, 2010, 2013 @heldercorreia
// Copyright (C) 2015 Pol Welter <polwelter@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#ifndef CORE_OPCODE_H
#define CORE_OPCODE_H

#include<QString>


class Opcode
{
public:
    enum  Type { Nop, Load, Ref, Function, Add, Sub, Neg, Mul, Div, Pow,
                 Fact, Modulo, IntDiv, Sal, Sar, BAnd, BOr, Conv, Pct, Yroot, Log,
                 BNand, BNor, BXor, Shl, Shr, Rol, Ror, Rcl, Rcr
               };

    Type type;
    unsigned index;

    // TODO: this is only needed for Conv Op. Maybe refactor this to a smarter place?
    // TODO: only keep a pointer to the string
    QString text;

    Opcode() : type(Nop), index(0) {}
    Opcode(Type t) : type(t), index(0) {}
    Opcode(Type t, QString txt) : type(t), index(0), text(txt) {}
    Opcode(Type t, unsigned i): type(t), index(i) {}
};

#endif // CORE_OPCODE_H
