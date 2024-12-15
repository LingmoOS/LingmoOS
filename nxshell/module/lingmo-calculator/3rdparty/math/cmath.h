// cmath.h
// Complex number support : type definition and function for complex numbers.
//
// This file is part of the SpeedCrunch project
// Copyright (C) 2013, 2015-2016 Hadrien Theveneau <theveneau@gmail.com>.
// Copyright (C) 2015-2016 Pol Welter.
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

#ifndef MATH_CMATH_H
#define MATH_CMATH_H

#include "hmath.h"
#include "rational.h"

#include <QMap>
#include <QString>

class CMath;

class CNumber
{
    friend class CMath;
    friend CNumber operator-(const CNumber &);
    friend CNumber operator-(const CNumber &, const CNumber &);
    friend bool operator>(const CNumber &, const CNumber &);
    friend bool operator<(const CNumber &, const CNumber &);
    friend bool operator>=(const CNumber &, const CNumber &);
    friend bool operator<=(const CNumber &, const CNumber &);
    friend bool operator==(const CNumber &, const CNumber &);
    friend bool operator!=(const CNumber &, const CNumber &);

public:
    CNumber();
    CNumber(const HNumber &);
    CNumber(const HNumber &, const HNumber &);
    CNumber(const CNumber &);
    CNumber(int);
    CNumber(const char *);
    CNumber(const QJsonObject &);
    ~CNumber() { }

    bool isNan() const;
    bool isZero() const;
    bool isPositive() const;
    bool isNegative() const;
    bool isInteger() const;
    bool isGaussian() const;
    bool isReal() const;
    bool isNearReal() const;

    void serialize(QJsonObject &) const;
    static CNumber deSerialize(const QJsonObject &);

    int toInt() const; // Removed, too problematic for complex numbers.
    Error error() const;

    CNumber &operator=(const CNumber &);
    CNumber operator+(const CNumber &) const;
    CNumber &operator+=(const CNumber &);
    CNumber &operator-=(const CNumber &);
    CNumber operator*(const CNumber &) const;
    CNumber operator*(const HNumber &) const;
    CNumber operator*(int x) { return operator * (HNumber(x)); } // Overload ambiguity resolution.
    CNumber &operator*=(const CNumber &);
    CNumber operator/(const CNumber &) const;
    CNumber operator/(const HNumber &) const;
    CNumber operator/(int x) { return operator / (HNumber(x)); } // Overload ambiguity resolution.
    CNumber &operator/=(const CNumber &);
    CNumber operator%(const CNumber &) const;
    CNumber operator&(const CNumber &) const;
    CNumber &operator&=(const CNumber &);
    CNumber operator|(const CNumber &) const;
    CNumber &operator|=(const CNumber &);
    CNumber operator^(const CNumber &) const;
    CNumber &operator^=(const CNumber &);
    CNumber operator~() const;
    CNumber operator>>(const CNumber &) const;
    CNumber operator<<(const CNumber &) const;

private:
    int compare(const CNumber &) const;

public:
    HNumber real;
    HNumber imag;

    // FIXME: Better access control to real and imag.

    // Invariants:
    // - real and imag are neither or both NaN.
    // - real and imag have the same NaN error.

    struct Format : public HNumber::Format  {
        enum class Notation {Null, Cartesian, Polar};
        Notation notation;

        Format();
        Format(const Format &);
        Format(const HNumber::Format &);
        Format operator+(const Format &) const;

        static Format Polar();
        static Format Cartesian();
    };
};

class CMath
{
public:
    // FORMAT
    static QString format(const CNumber &, CNumber::Format = CNumber::Format());
    // CONSTANTS
    static CNumber e();
    static CNumber phi();
    static CNumber pi();
    static CNumber nan(Error error = Success);
    static CNumber i();
    // GENERAL MATH
    static CNumber rad2deg(const CNumber &);
    static CNumber deg2rad(const CNumber &);
    static CNumber rad2gon(const CNumber &);
    static CNumber gon2rad(const CNumber &);
    static CNumber abs(const CNumber &);
    static CNumber integer(const CNumber &);
    static CNumber frac(const CNumber &);
    static CNumber floor(const CNumber &);
    static CNumber ceil(const CNumber &);
    static CNumber gcd(const CNumber &, const CNumber &);
    static CNumber idiv(const CNumber &, const CNumber &);
    static CNumber round(const CNumber &, int prec = 0);
    static CNumber trunc(const CNumber &, int prec = 0);
    static CNumber sqrt(const CNumber &);
    static CNumber cbrt(const CNumber &);
    static CNumber raise(const CNumber &, int);
    static CNumber raise(const CNumber &, const CNumber &);
    static CNumber sgn(const CNumber &);
    // EXPONENTIAL FUNCTION AND RELATED
    static CNumber exp(const CNumber &);
    static CNumber ln(const CNumber &);
    static CNumber lg(const CNumber &);
    static CNumber lb(const CNumber &);
    static CNumber log(const CNumber &base, const CNumber &x);
    static CNumber sinh(const CNumber &);
    static CNumber cosh(const CNumber &);
    static CNumber tanh(const CNumber &);
    static CNumber arsinh(const CNumber &);
    static CNumber arcosh(const CNumber &);
    static CNumber artanh(const CNumber &);
    // COMPLEX SPECIFIC
    static CNumber real(const CNumber &x) {return x.real;}
    static CNumber imag(const CNumber &x) {return x.imag;}
    static CNumber conj(const CNumber &x);
    static CNumber phase(const CNumber &);
    // TRIGONOMETRY
    static CNumber sin(const CNumber &);
    static CNumber cos(const CNumber &);
    static CNumber tan(const CNumber &);
    static CNumber cot(const CNumber &);
    static CNumber sec(const CNumber &);
    static CNumber csc(const CNumber &);
    static CNumber arcsin(const CNumber &);
    static CNumber arccos(const CNumber &);
    static CNumber arctan(const CNumber &);
    static CNumber arctan2(const CNumber &, const CNumber &);
    // HIGHER MATH FUNCTIONS
    static CNumber factorial(const CNumber &, const CNumber &base = CNumber(1));
    static CNumber gamma(const CNumber &);
    static CNumber lnGamma(const CNumber &);
    static CNumber erf(const CNumber &);
    static CNumber erfc(const CNumber &);
    // PROBABILITY
    static CNumber nCr(const CNumber &, const CNumber &);
    static CNumber nPr(const CNumber &, const CNumber &);
    static CNumber binomialPmf(const CNumber &k, const CNumber &n, const CNumber &p);
    static CNumber binomialCdf(const CNumber &k, const CNumber &n, const CNumber &p);
    static CNumber binomialMean(const CNumber &n, const CNumber &p);
    static CNumber binomialVariance(const CNumber &n, const CNumber &p);
    static CNumber hypergeometricPmf(const CNumber &k, const CNumber &N, const CNumber &M, const CNumber &n);
    static CNumber hypergeometricCdf(const CNumber &k, const CNumber &N, const CNumber &M, const CNumber &n);
    static CNumber hypergeometricMean(const CNumber &N, const CNumber &M, const CNumber &n);
    static CNumber hypergeometricVariance(const CNumber &N, const CNumber &M, const CNumber &n);
    static CNumber poissonPmf(const CNumber &k, const CNumber &l);
    static CNumber poissonCdf(const CNumber &k, const CNumber &l);
    static CNumber poissonMean(const CNumber &l);
    static CNumber poissonVariance(const CNumber &l);
    // LOGIC
    static CNumber mask(const CNumber &, const CNumber &bits);
    static CNumber sgnext(const CNumber &, const CNumber &bits);
    static CNumber ashr(const CNumber &, const CNumber &bits);
    static CNumber lshr(const CNumber &, const CNumber &bits);
    static CNumber rosh(const CNumber &, const CNumber &bits);
    static CNumber rcsh(const CNumber &, const CNumber &bits);
    // IEEE-754 CONVERSION
    static CNumber decodeIeee754(const CNumber &, const CNumber &exp_bits, const CNumber &significand_bits);
    static CNumber decodeIeee754(const CNumber &, const CNumber &exp_bits, const CNumber &significand_bits,
                                 const CNumber &exp_bias);
    static CNumber encodeIeee754(const CNumber &, const CNumber &exp_bits, const CNumber &significand_bits);
    static CNumber encodeIeee754(const CNumber &, const CNumber &exp_bits, const CNumber &significand_bits,
                                 const CNumber &exp_bias);
};

std::ostream &operator<<(std::ostream &, const CNumber &);

#endif // CMATH_CMATH_H
