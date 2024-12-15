// quantity.h
// Support for units and dimensions
//
// This file is part of the SpeedCrunch project
// Copyright (C) 2016 Pol Welter.
// Copyright (C) 2016 @heldercorreia
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

#ifndef QUANTITY_H
#define QUANTITY_H

#include "cmath.h"
#include "hmath.h"
#include "../core/errors.h"

#include <QMap>

class CNumber;
class HNumber;
class QJsonObject;
class QString;
class Rational;

class DMath;
class Quantity;

Quantity operator*(const HNumber &, const Quantity &);
Quantity operator*(const CNumber &, const Quantity &);
Quantity operator/(const HNumber &, const Quantity &);
Quantity operator/(const CNumber &, const Quantity &);

class Quantity
{
    friend class DMath;
    friend Quantity operator-(const Quantity &);
    friend Quantity operator-(const Quantity &, const Quantity &);
    friend bool operator>(const Quantity &, const Quantity &);
    friend bool operator<(const Quantity &, const Quantity &);
    friend bool operator>=(const Quantity &, const Quantity &);
    friend bool operator<=(const Quantity &, const Quantity &);
    friend bool operator==(const Quantity &, const Quantity &);
    friend bool operator!=(const Quantity &, const Quantity &);

public:
    Quantity();
    Quantity(const Quantity &);
    Quantity(int);
    Quantity(const QJsonObject &);
    Quantity(const HNumber &);
    Quantity(const CNumber &);
    ~Quantity();

    bool isNan() const;
    bool isZero() const;
    bool isReal() const;
    bool isPositive() const;
    bool isNegative() const;
    bool isInteger() const;
    bool isEven() const;

    bool hasUnit() const;
    CNumber unit() const;
    QString unitName() const;
    CNumber numericValue() const;
    Quantity &setDisplayUnit(const CNumber unit, const QString &name);
    void stripUnits();
    bool hasDimension() const;
    bool isDimensionless() const;
    QMap<QString, Rational> getDimension() const;
    void modifyDimension(const QString &key, const Rational &exponent);
    void copyDimension(const Quantity &);
    void clearDimension();
    bool sameDimension(const Quantity &other) const;
    void cleanDimension();

    void serialize(QJsonObject &) const;
    static Quantity deSerialize(const QJsonObject &);

    Error error() const;

    Quantity &operator=(const Quantity &);
    Quantity operator+(const Quantity &) const;
    Quantity &operator+=(const Quantity &);
    Quantity &operator-=(const Quantity &);
    Quantity operator*(const Quantity &) const;
    Quantity operator*(const CNumber &) const;
    Quantity operator*(const HNumber &) const;
    Quantity &operator*=(const Quantity &);
    Quantity operator/(const Quantity &) const;
    Quantity operator/(const HNumber &) const;
    Quantity operator/(const CNumber &) const;
    Quantity &operator/=(const Quantity &);
    Quantity operator%(const Quantity &) const;
    Quantity operator&(const Quantity &) const;
    Quantity &operator&=(const Quantity &);
    Quantity operator|(const Quantity &) const;
    Quantity &operator|=(const Quantity &);
    Quantity operator^(const Quantity &) const;
    Quantity &operator^=(const Quantity &);
    Quantity operator~() const;
    Quantity operator>>(const Quantity &) const;
    Quantity operator<<(const Quantity &) const;

    class Format : public CNumber::Format
    {
    public:
        Format();
        Format(const CNumber::Format &);
        Format(const HNumber::Format &);
        Format operator+(const Format &) const;

        void serialize(QJsonObject &) const;
        static Format deSerialize(const QJsonObject &);
        bool isNull() const;
    };

    Format format() const { return m_format; }
    Quantity &setFormat(Format);

private:
    CNumber m_numericValue;
    QMap<QString, Rational> m_dimension;
    CNumber *m_unit;
    QString m_unitName;
    Format m_format;
};

/*
 * Math functions for quantities with dimensions
 */
class DMath
{
public:
    static bool complexMode;

    static QString format(const Quantity, Quantity::Format = Quantity::Format());

    // CONSTANTS
    static Quantity e();
    static Quantity phi();
    static Quantity pi();
    static Quantity nan(Error = Success);
    static Quantity i();
    // GENERAL MATH
    static Quantity rad2deg(const Quantity &);
    static Quantity deg2rad(const Quantity &);
    static Quantity rad2gon(const Quantity &);
    static Quantity gon2rad(const Quantity &);
    static Quantity abs(const Quantity &);
    static Quantity integer(const Quantity &);
    static Quantity frac(const Quantity &);
    static Quantity floor(const Quantity &);
    static Quantity ceil(const Quantity &);
    static Quantity gcd(const Quantity &, const Quantity &);
    static Quantity idiv(const Quantity &, const Quantity &);
    static Quantity round(const Quantity &, int prec = 0);
    static Quantity trunc(const Quantity &, int prec = 0);
    static Quantity sqrt(const Quantity &);
    static Quantity cbrt(const Quantity &);
    static Quantity raise(const Quantity &, int);
    static Quantity raise(const Quantity &, const Quantity &);
    static Quantity sgn(const Quantity &);
    // EXPONENTIAL FUNCTION AND RELATED
    static Quantity exp(const Quantity &);
    static Quantity ln(const Quantity &);
    static Quantity lg(const Quantity &);
    static Quantity lb(const Quantity &);
    static Quantity log(const Quantity &base, const Quantity &);
    static Quantity sinh(const Quantity &);
    static Quantity cosh(const Quantity &);
    static Quantity tanh(const Quantity &);
    static Quantity arsinh(const Quantity &);
    static Quantity arcosh(const Quantity &);
    static Quantity artanh(const Quantity &);
    // COMPLEX
    static Quantity real(const Quantity &);
    static Quantity imag(const Quantity &);
    static Quantity conj(const Quantity &);
    static Quantity phase(const Quantity &);
    // TRIGONOMETRY
    static Quantity sin(const Quantity &);
    static Quantity cos(const Quantity &);
    static Quantity tan(const Quantity &);
    static Quantity cot(const Quantity &);
    static Quantity sec(const Quantity &);
    static Quantity csc(const Quantity &);
    static Quantity arcsin(const Quantity &);
    static Quantity arccos(const Quantity &);
    static Quantity arctan(const Quantity &);
    static Quantity arctan2(const Quantity &, const Quantity &y);
    // HIGHER MATH FUNCTIONS
    static Quantity factorial(const Quantity &x, const Quantity &base = CNumber(1));
    static Quantity gamma(const Quantity &);
    static Quantity lnGamma(const Quantity &);
    static Quantity erf(const Quantity &);
    static Quantity erfc(const Quantity &);
    // PROBABILITY
    static Quantity nCr(const Quantity &n, const Quantity &k);
    static Quantity nPr(const Quantity &n, const Quantity &r);
    static Quantity binomialPmf(const Quantity &k, const Quantity &n, const Quantity &p);
    static Quantity binomialCdf(const Quantity &k, const Quantity &n, const Quantity &p);
    static Quantity binomialMean(const Quantity &n, const Quantity &p);
    static Quantity binomialVariance(const Quantity &n, const Quantity &p);
    static Quantity hypergeometricPmf(const Quantity &k, const Quantity &N, const Quantity &M, const Quantity &n);
    static Quantity hypergeometricCdf(const Quantity &k, const Quantity &N, const Quantity &M, const Quantity &n);
    static Quantity hypergeometricMean(const Quantity &N, const Quantity &M, const Quantity &n);
    static Quantity hypergeometricVariance(const Quantity &N, const Quantity &M, const Quantity &n);
    static Quantity poissonPmf(const Quantity &k, const Quantity &l);
    static Quantity poissonCdf(const Quantity &k, const Quantity &l);
    static Quantity poissonMean(const Quantity &l);
    static Quantity poissonVariance(const Quantity &l);
    // LOGIC
    static Quantity mask(const Quantity &, const Quantity &bits);
    static Quantity sgnext(const Quantity &, const Quantity &bits);
    static Quantity ashr(const Quantity &, const Quantity &bits);
    static Quantity lshr(const Quantity &, const Quantity &bits); //逻辑移位
    static Quantity rosh(const Quantity &, const Quantity &bits); //循环移位
    static Quantity rcsh(const Quantity &, const Quantity &bits); //带进位循环移位
    // IEEE-754 CONVERSION
    static Quantity decodeIeee754(const Quantity &, const Quantity &exp_bits, const Quantity &significand_bits);
    static Quantity decodeIeee754(const Quantity &, const Quantity &exp_bits, const Quantity &significand_bits,
                                  const Quantity &exp_bias);
    static Quantity encodeIeee754(const Quantity &, const Quantity &exp_bits, const Quantity &significand_bits);
    static Quantity encodeIeee754(const Quantity &, const Quantity &exp_bits, const Quantity &significand_bits,
                                  const Quantity &exp_bias);
};

#endif // QUANTITY_H
