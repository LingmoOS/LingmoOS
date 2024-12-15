// This file is part of the SpeedCrunch project
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

#ifndef RATIONAL_H
#define RATIONAL_H

class HNumber;
class QString;

class Rational
{
    int m_num;
    int m_denom;
    bool m_valid;

    inline int gcd(int a, int b) const {return (b==0) ? a : gcd(b, a%b);}
    void normalize();
    int compare(const Rational & other) const;

public:
    Rational() : m_num(0), m_denom(1), m_valid(true) {}
    Rational(const HNumber &num);
    Rational(const double &num);
    Rational(const QString & str);
    Rational(const int a, const int b) : m_num(a), m_denom(b), m_valid(true) {normalize();}

    int numerator() const {return m_num;}
    int denominator() const {return m_denom;}

    Rational operator*(const Rational & other) const;
    Rational operator/(const Rational & other) const;
    Rational operator+(const Rational & other) const;
    Rational operator-(const Rational & other) const;
    Rational &operator=(const Rational & other);
    Rational &operator+=(const Rational & other);
    Rational &operator-=(const Rational & other);
    Rational &operator*=(const Rational & other);
    Rational &operator/=(const Rational & other);
    bool operator<(const Rational & other) const;
    bool operator==(const Rational & other) const;
    bool operator!=(const Rational & other) const {return !operator==(other);}
    bool operator>(const Rational & other) const;

    bool isZero() const;
    bool isValid() const;

    QString toString() const;
    HNumber toHNumber( )const;
    double toDouble() const;
};

#endif // RATIONAL_H
