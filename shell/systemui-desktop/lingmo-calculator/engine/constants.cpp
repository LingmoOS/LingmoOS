/* Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "constants.h"

Constants::Constants(QObject* parent): QObject(parent)
{
    setObjectName("Constants");

    // http://en.wikipedia.org/wiki/Physical_constant#Table_of_atomic_and_nuclear_constants
    constantList += Constant(QString("Bohr radius"), "0.5291772108e-10", "m", QString("Atomic & Nuclear"));
    constantList += Constant(QString("Fermi coupling constant"), "1.16639e-5", "Ge/V^2", QString("Atomic & Nuclear"));
    constantList += Constant(QString("fine-structure constant"), "7.297352568e-3", QString(), QString("Atomic & Nuclear"));
    constantList += Constant(QString("Hartree energy"), "4.35974417e-18", "J", QString("Atomic & Nuclear"));
    constantList += Constant(QString("quantum of circulation"), "3.636947550e-4", "m^2/s", QString("Atomic & Nuclear"));
    constantList += Constant(QString("Rydberg constant"), "10973731.568525", "/m", QString("Atomic & Nuclear"));
    constantList += Constant(QString("Thomson cross section"), "0.665245873e-28", "m^2", QString("Atomic & Nuclear"));
    constantList += Constant(QString("weak mixing angle"), "0.22215", QString(), QString("Atomic & Nuclear"));

    // http://www.astronomynotes.com/tables/tablesa.htm
    constantList += Constant(QString("astronomical unit"), "149597870.691", "km", QString("Astronomy"));
    constantList += Constant(QString("light year"), "9.460536207e12", "km", QString("Astronomy"));
    constantList += Constant(QString("parsec"), "3.08567802e13", "km", QString("Astronomy"));
    constantList += Constant(QString("sidereal year"), "365.2564", "days", QString("Astronomy"));
    constantList += Constant(QString("tropical year"), "365.2422", "days", QString("Astronomy"));
    constantList += Constant(QString("Gregorian year"), "365.2425", "days", QString("Astronomy"));
    constantList += Constant(QString("Earth mass"), "5.9736e24", "kg", QString("Astronomy"));
    constantList += Constant(QString("Sun mass"), "1.9891e30", "kg", QString("Astronomy"));
    constantList += Constant(QString("mean Earth radius"), "6371", "km", QString("Astronomy"));
    constantList += Constant(QString("Sun radius"), "6.96265e5", "km", QString("Astronomy"));
    constantList += Constant(QString("Sun luminosity"), "3.827e26", "W", QString("Astronomy"));

    for (int k = 0; k < constantList.count(); k++) {
        QStringList cats = constantList[k].categories;

        for (int i = 0; i < cats.count(); i++) {
            if (!categoryList.contains(cats[i])) {
                categoryList += cats[i];
            }
        }
    }

    categoryList.sort();
}
