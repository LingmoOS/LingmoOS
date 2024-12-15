// HMath: C++ high precision math routines
// Copyright (C) 2004 Ariya Hidayat <ariya.hidayat@gmail.com>
// Copyright (C) 2007-2008, 2014, 2016 @heldercorreia
// Copyright (C) 2008 Wolf Lammen
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

#ifndef MATH_HMATH_H
#define MATH_HMATH_H

#include "../core/errors.h"

#include <QJsonObject>
#include <QString>

#include <ostream>

class HMath;
class HNumberPrivate;
class CNumber;
class CMath;
class Rational;

class HNumber
{
    friend class HMath;
    friend class CNumber;
    friend class CMath;
    friend HNumber operator-(const HNumber &);
    friend HNumber operator-(const HNumber &, const HNumber &);
    friend bool operator>(const HNumber &, const HNumber &);
    friend bool operator<(const HNumber &, const HNumber &);
    friend bool operator>=(const HNumber &, const HNumber &);
    friend bool operator<=(const HNumber &, const HNumber &);
    friend bool operator==(const HNumber &, const HNumber &);
    friend bool operator!=(const HNumber &, const HNumber &);

public:
    HNumber();
    HNumber(const HNumber &);
    HNumber(int);
    HNumber(const char *);
    HNumber(const QJsonObject &);
    HNumber(const char *, bool);
    ~HNumber();

    bool isNan() const;
    bool isZero() const;
    bool isNearZero() const;
    bool isPositive() const;
    bool isNegative() const;
    bool isInteger() const;

    void serialize(QJsonObject &) const;
    static HNumber deSerialize(const QJsonObject &);

    int toInt() const;
    Error error() const;

    HNumber &operator=(const HNumber &);
    HNumber operator+(const HNumber &) const;
    HNumber &operator+=(const HNumber &);
    HNumber &operator-=(const HNumber &);
    HNumber operator*(const HNumber &)const;
    HNumber &operator*=(const HNumber &);
    HNumber operator/(const HNumber &) const;
    HNumber &operator/=(const HNumber &);
    HNumber operator%(const HNumber &) const;
    HNumber operator&(const HNumber &)const;
    HNumber &operator&=(const HNumber &);
    HNumber operator|(const HNumber &) const;
    HNumber &operator|=(const HNumber &);
    HNumber operator^(const HNumber &) const;
    HNumber &operator^=(const HNumber &);
    HNumber operator~() const;
    HNumber operator>>(const HNumber &) const;
    HNumber operator<<(const HNumber &) const;

    int realNumberlength() const;

private:
    HNumberPrivate *d;

    int compare(const HNumber &) const;

public:
    struct Format {
        enum class Base { Null, Binary, Decimal, Octal, Hexadecimal };
        enum class RadixChar { Null, Point, Comma };
        enum class Mode { Null, General, Fixed, Scientific, Engineering, Complement};
        enum class AutoCut {Null, Ncut};

        Base base;
        RadixChar radixChar;
        Mode mode;
        AutoCut cut;
        int precision;  // -1 means 'auto'
        static const int PrecisionNull = -1000;

        Format();
        Format(const Format &);
        Format operator+(const Format &) const;

        static Format Binary();
        static Format Octal();
        static Format Decimal();
        static Format Hexadecimal();

        static Format Precision(int);

        static Format Point();
        static Format Comma();

        static Format General();
        static Format Fixed();
        static Format Scientific();
        static Format Engineering();
        static Format Complement();

        static Format NCut();
    };
};

class HMath
{
public:
    // FORMAT
    static QString format(const HNumber &, HNumber::Format = HNumber::Format());
    // PARSING
    static HNumber parse_str(const char *, const char **out);
    // CONSTANTS
    static HNumber e();
    static HNumber phi();
    static HNumber pi();
    static HNumber nan(Error = Success);
    // GENERAL MATH
    static HNumber rad2deg(const HNumber &);
    static HNumber deg2rad(const HNumber &);
    static HNumber rad2gon(const HNumber &);
    static HNumber gon2rad(const HNumber &);
    static HNumber max(const HNumber &, const HNumber &);
    static HNumber min(const HNumber &, const HNumber &);
    static HNumber abs(const HNumber &);
    static HNumber integer(const HNumber &);
    static HNumber frac(const HNumber &);
    static HNumber floor(const HNumber &);
    static HNumber ceil(const HNumber &);
    static HNumber gcd(const HNumber &, const HNumber &);
    static HNumber idiv(const HNumber &, const HNumber &);
    static HNumber round(const HNumber &, int prec = 0);
    static HNumber trunc(const HNumber &, int prec = 0);
    static HNumber sqrt(const HNumber &);
    static HNumber cbrt(const HNumber &);
    static HNumber raise(const HNumber &, int);
    static HNumber raise(const HNumber &, const HNumber &);
    static HNumber sgn(const HNumber &);
    // EXPONENTIAL FUNCTION AND RELATED
    static HNumber exp(const HNumber &);
    static HNumber ln(const HNumber &);
    static HNumber lg(const HNumber &);
    static HNumber lb(const HNumber &);
    static HNumber log(const HNumber &base, const HNumber &x);
    static HNumber sinh(const HNumber &);
    static HNumber cosh(const HNumber &);
    static HNumber tanh(const HNumber &);
    static HNumber arsinh(const HNumber &);
    static HNumber arcosh(const HNumber &);
    static HNumber artanh(const HNumber &);
    // TRIGONOMETRY
    static HNumber sin(const HNumber &);
    static HNumber cos(const HNumber &);
    static HNumber tan(const HNumber &);
    static HNumber cot(const HNumber &);
    static HNumber sec(const HNumber &);
    static HNumber csc(const HNumber &);
    static HNumber arcsin(const HNumber &);
    static HNumber arccos(const HNumber &);
    static HNumber arctan(const HNumber &);
    static HNumber arctan2(const HNumber &, const HNumber &);
    // HIGHER MATH FUNCTIONS
    static HNumber factorial(const HNumber &, const HNumber &base = HNumber(1));
    static HNumber gamma(const HNumber &);
    static HNumber lnGamma(const HNumber &);
    static HNumber erf(const HNumber &);
    static HNumber erfc(const HNumber &);
    // PROBABILITY
    static HNumber nCr(const HNumber &n, const HNumber &k);
    static HNumber nPr(const HNumber &n, const HNumber &r);
    static HNumber binomialPmf(const HNumber &k, const HNumber &n, const HNumber &p);
    static HNumber binomialCdf(const HNumber &k, const HNumber &n, const HNumber &p);
    static HNumber binomialMean(const HNumber &n, const HNumber &p);
    static HNumber binomialVariance(const HNumber &n, const HNumber &p);
    static HNumber hypergeometricPmf(const HNumber &k, const HNumber &N, const HNumber &M,
                                     const HNumber &n);
    static HNumber hypergeometricCdf(const HNumber &k, const HNumber &N, const HNumber &M,
                                     const HNumber &n);
    static HNumber hypergeometricMean(const HNumber &N, const HNumber &M, const HNumber &n);
    static HNumber hypergeometricVariance(const HNumber &N, const HNumber &M, const HNumber &n);
    static HNumber poissonPmf(const HNumber &k, const HNumber &l);
    static HNumber poissonCdf(const HNumber &k, const HNumber &l);
    static HNumber poissonMean(const HNumber &l);
    static HNumber poissonVariance(const HNumber &l);
    // LOGIC
    static HNumber mask(const HNumber &, const HNumber &bits);
    static HNumber sgnext(const HNumber &, const HNumber &bits);
    static HNumber ashr(const HNumber &, const HNumber &bits);
    static HNumber lshr(const HNumber &, const HNumber &bits);
    static HNumber rosh(const HNumber &, const HNumber &bits);
    static HNumber rcsh(const HNumber &, const HNumber &bits);
    // IEEE-754 CONVERSION
    static HNumber decodeIeee754(const HNumber &, const HNumber &exp_bits,
                                 const HNumber &significand_bits);
    static HNumber decodeIeee754(const HNumber &, const HNumber &exp_bits,
                                 const HNumber &significand_bits, const HNumber &exp_bias);
    static HNumber encodeIeee754(const HNumber &, const HNumber &exp_bits,
                                 const HNumber &significand_bits);
    static HNumber encodeIeee754(const HNumber &, const HNumber &exp_bits,
                                 const HNumber &significand_bits, const HNumber &exp_bias);
};

std::ostream &operator<<(std::ostream &, const HNumber &);

#endif  // MATH_HMATH_H
