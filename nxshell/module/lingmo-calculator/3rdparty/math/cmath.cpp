// cmath.cpp
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

#include "cmath.h"

#include "cnumberparser.h"
#include "floatconvert.h"
#include "hmath.h"

#include <QString>
#include <QStringList>

#include <stdlib.h>
#include <string.h>

/**
 * Creates a new complex number.
 */
CNumber::CNumber()
    : real(0)
    , imag(0)
{}

/**
 * Creates a new complex number from one real number
 */
CNumber::CNumber(const HNumber &hn) :
    real(hn),
    imag(0)
{
}

/**
 * Creates a new complex number from the real and imaginary parts
 */
CNumber::CNumber(const HNumber &x, const HNumber &y)
    : real(x)
    , imag(y)
{
}

/**
 * Copies from another complex number.
 */
CNumber::CNumber(const CNumber &cn)
    : real(cn.real)
    , imag(cn.imag)
{
}

/**
 * Creates a new number from an integer value.
 */
CNumber::CNumber(int i)
    : real(i)
    , imag(0)
{
}

/**
 * Creates a new number from a string.
 */
CNumber::CNumber(const char *str)
{
    CNumberParser parser(str);
    parser.parse(this); // FIXME: Exception management.
}

CNumber::CNumber(const QJsonObject &json)
{
    *this = deSerialize(json);
}

/**
 * Returns true if this number is Not a Number (NaN).
 */
bool CNumber::isNan() const
{
    return real.isNan() || imag.isNan();
}

/**
 * Returns true if this number is zero.
 */
bool CNumber::isZero() const
{
    return real.isZero() && imag.isZero();
}

/**
 * Returns true if this number is a positive REAL
 */
bool CNumber::isPositive() const
{
    return real.isPositive() && imag.isZero();
}

/**
 * Returns true if this number is a negative REAL
 */
bool CNumber::isNegative() const
{
    return real.isNegative() && imag.isZero();
}

/**
 * Returns true if this number is integer and REAL
 */
bool CNumber::isInteger() const
{
    return real.isInteger() && imag.isZero();
}

/**
 * Returns true if this number is a Gaussian integer
 */
bool CNumber::isGaussian() const
{
    return real.isInteger() && imag.isInteger();
}

/**
 * Returns true if this number is a real number
 */
bool CNumber::isReal() const
{
    return imag.isZero();
}

/**
 * Returns true if this number is approximately a real number
 */
bool CNumber::isNearReal() const
{
    return imag.isNearZero();
}

void CNumber::serialize(QJsonObject &json) const
{
    json["value"] = CMath::format(*this, CNumber::Format::Fixed() + CNumber::Format::Precision(DECPRECISION));
}

CNumber CNumber::deSerialize(const QJsonObject &json)
{
    CNumber result;
    if (json.contains("value")) {
        QString str = json["value"].toString();
        str.replace(",", ".");
        result = CNumber(str.toLatin1().constData());
    }
    return result;
}

/**
 * Returns a NaN (Not a Number) with error set to
 * passed parameter.
 */
CNumber CMath::nan(Error error)
{
    // We must always ensure that both numbers have the same NaN error.
    CNumber result;
    result.real = HMath::nan(error);
    result.imag = HMath::nan(error);
    return result;
}

/**
 * Returns the error code kept with a NaN.
 */
Error CNumber::error() const
{
    // real and imag have always the same NaN error.
    return real.error();
}

/**
 * Assigns from another complex number.
 */
CNumber &CNumber::operator=(const CNumber &cn)
{
    real = cn.real;
    imag = cn.imag;
    return *this;
}

/**
 * Adds another complex number.
 */
CNumber CNumber::operator+(const CNumber &num) const
{
    CNumber result;
    result.real = real + num.real;
    result.imag = imag + num.imag;
    return result;
}

/**
 * Adds another complex number.
 */
CNumber &CNumber::operator+=(const CNumber &num)
{
    return operator=(*this + num);
}

/**
 * Subtract from another complex number.
 */
CNumber operator-(const CNumber &n1, const CNumber &n2)
{
    CNumber result;
    result.real = n1.real - n2.real;
    result.imag = n1.imag - n2.imag;
    return result;
}

/**
 * Subtract from another complex number.
 */
CNumber &CNumber::operator-=(const CNumber &num)
{
    return operator=(*this - num);
}

/**
 * Multiplies with another complex number.
 */
CNumber CNumber::operator*(const CNumber &num) const
{
    CNumber result;
    result.real = real * num.real - imag * num.imag;
    result.imag = imag * num.real + real * num.imag;
    return result;
}

/**
 * Multiplies with another REAL number.
 */
CNumber CNumber::operator*(const HNumber &num) const
{
    CNumber result;
    result.real = real * num;
    result.imag = imag * num;
    return result;
}

/**
 * Multiplies with another number.
 */
CNumber &CNumber::operator*=(const CNumber &num)
{
    return operator=(*this * num);
}

/**
 * Divides with another complex number.
 */
CNumber CNumber::operator/(const CNumber &num) const
{
    if (num.isZero())
        return CMath::nan(ZeroDivide);
    else {
        CNumber result;
        HNumber divider = num.real * num.real + num.imag * num.imag;
        result.real = (real * num.real + imag * num.imag) / divider;
        result.imag = (imag * num.real - real * num.imag) / divider;
        return result;
    }
}

/**
 * Divides with another REAL number.
 */
CNumber CNumber::operator/(const HNumber &num) const
{
    if (num.isZero())
        return CMath::nan(ZeroDivide);
    else
        return CNumber(real / num, imag / num)  ;
}

/**
 * Divides with another number.
 */
CNumber &CNumber::operator/=(const CNumber &num)
{
    return operator=(*this / num);
}

/**
 * Returns -1, 0, 1 if n1 is less than, equal to, or more than n2.
 * Only valid for real numbers, since complex ones are not an ordered field.
 */
int CNumber::compare(const CNumber &other) const
{
    if (isReal() && other.isReal())
        return real.compare(other.real);
    else
        return false; // FIXME: Return something better.
}

/**
 * Returns true if l is greater than r.
 */
bool operator>(const CNumber &l, const CNumber &r)
{
    return l.compare(r) > 0;
}

/**
 * Returns true if l is less than r.
 */
bool operator<(const CNumber &l, const CNumber &r)
{
    return l.compare(r) < 0;
}

/**
 * Returns true if l is greater than or equal to r.
 */
bool operator>=(const CNumber &l, const CNumber &r)
{
    return l.compare(r) >= 0;
}

/**
 * Returns true if l is less than or equal to r.
 */
bool operator<=(const CNumber &l, const CNumber &r)
{
    return l.compare(r) <= 0;
}

/**
 * Returns true if l is equal to r.
 */
bool operator==(const CNumber &l, const CNumber &r)
{
    return l.compare(r) == 0;
}

/**
 * Returns true if l is not equal to r.
 */
bool operator!=(const CNumber &l, const CNumber &r)
{
    return l.compare(r) != 0;
}

/**
 * Changes the sign.
 */
CNumber operator-(const CNumber &x)
{
    return CNumber(-x.real, -x.imag);
}

CNumber::Format::Format()
    : HNumber::Format()
    , notation(Format::Notation::Null)
{
}

CNumber::Format::Format(const Format &other)
    : HNumber::Format(static_cast<const HNumber::Format &>(other))
    , notation(other.notation)
{
}

CNumber::Format::Format(const HNumber::Format &other)
    : HNumber::Format(other)
    , notation(Notation::Null)
{
}

CNumber::Format CNumber::Format::operator+(const CNumber::Format &other) const
{
    Format result(HNumber::Format::operator+(static_cast<const HNumber::Format &>(other)));
    result.notation = (this->notation != Notation::Null) ? this->notation : other.notation;
    return result;
}

CNumber::Format CNumber::Format::Polar()
{
    Format result;
    result.notation = Format::Notation::Polar;
    return result;
}

CNumber::Format CNumber::Format::Cartesian()
{
    Format result;
    result.notation = Format::Notation::Cartesian;
    return result;
}

/**
 * Returns the constant e (Euler's number).
 */
CNumber CMath::e()
{
    return CNumber(HMath::e());
}

/**
 * Returns the constant pi.
 */
CNumber CMath::pi()
{
    return CNumber(HMath::pi());
}

/**
 * Returns the constant phi (golden number).
 */
CNumber CMath::phi()
{
    return CNumber(HMath::phi());
}

/**
 * Returns the constant i.
 */
CNumber CMath::i()
{
    return CNumber(0, 1);
}

// TODO: Improve complex number formatting.

/**
 * Formats the given number as string.
 */
QString CMath::format(const CNumber &cn, CNumber::Format format)
{
    if (cn.isNan())
        return "NaN";
    else if (cn.imag.isNearZero()) // Number is real.
        return HMath::format(cn.real, format);
    if (format.notation == CNumber::Format::Notation::Polar) {
        QString strRadius = HMath::format(CMath::abs(cn).real, format);
        HNumber phase = CMath::phase(cn).real;
        if (phase.isZero())
            return strRadius;
        QString strPhase = HMath::format(phase, format);
        return QString("%1 * exp(j*%2)").arg(strRadius, strPhase);
    } else {
        QString real_part = cn.real.isZero() ? "" : HMath::format(cn.real, format);
        QString imag_part = "";
        QString separator = "";
        QString prefix    = ""; // TODO: Insert two modes, one for a+jb and one for a+bj.
        QString postfix   = "j"; // TODO: Insert two modes, one for a+bi and one for a+bj.

        if (cn.imag.isPositive()) {
            separator = cn.real.isZero() ? "" : "+";
            imag_part = HMath::format(cn.imag, format);
        } else {
            separator = "-";
            imag_part = HMath::format(-cn.imag, format);
        }
        return real_part + separator + prefix + imag_part + postfix;
    }
}

/**
 * Returns the norm of n.
 */
CNumber CMath::abs(const CNumber &n)
{
    return HMath::sqrt(n.real * n.real + n.imag * n.imag);
}

/*
 * Returns the complex conjugate of n
 */
CNumber CMath::conj(const CNumber &n)
{
    return CNumber(n.real, -n.imag);
}

/**
 * Returns the square root of n.
 */
CNumber CMath::sqrt(const CNumber &n)
{
    CNumber result;
    HNumber s = (n.imag.isPositive() || n.imag.isZero()) ? 1 : -1;

    // cf https://en.wikipedia.org/wiki/Square_root#Square_roots_of_negative_and_complex_numbers.
    result.real =     HMath::sqrt((abs(n).real + n.real) / 2);
    result.imag = s * HMath::sqrt((abs(n).real - n.real) / 2);

    return result;
}

/**
 * Raises n1 to an integer n.
 */
CNumber CMath::raise(const CNumber &n1, int n)
{
    return CMath::exp(CMath::ln(n1) * n);
}

/**
 * Raises n1 to n2.
 */
CNumber CMath::raise(const CNumber &n1, const CNumber &n2)
{
    if (n1.isZero() && (n2.real > 0))
        return CNumber(0);
    return CMath::exp(CMath::ln(n1) * n2);
}

/**
 * Returns e raised to x.
 */
CNumber CMath::exp(const CNumber &x)
{
    HNumber abs = HMath::exp(x.real);
    return CNumber(abs * HMath::cos(x.imag), abs * HMath::sin(x.imag));
}

/**
 * Returns the complex natural logarithm of x.
 */
CNumber CMath::ln(const CNumber &x)
{
    CNumber result;
    // If real number
    if (x.imag.isZero()) {
        // If zero number, out of domain
        if (x.real.isZero())
            return CMath::nan(OutOfDomain);
        // Here, non-zero number
        // Real part
        result.real = HMath::ln(HMath::abs(x.real));
        // If NaN, ensures NaN and same error in both imag and real parts
        if (result.real.isNan()) {
            result.imag = result.real;
            return result;
        }
        // Imag part according to sign of real part
        result.imag = x.real.isPositive() ? 0 : HMath::pi();
    }
    // Else, complex number
    else {
        HNumber abs = CMath::abs(x).real;
        result.real = HMath::ln(abs);
        // Principal Value logarithm
        // https://en.wikipedia.org/wiki/Complex_logarithm#Definition_of_principal_value
        auto imag = HMath::arccos(x.real / abs);
        result.imag = (x.imag.isPositive() || x.imag.isZero()) ? imag : -imag;
    }
    return result;
}

/**
 * Returns the common logarithm of x.
 */
CNumber CMath::lg(const CNumber &x)
{
    return CMath::ln(x) / HMath::ln(10);
}

/**
 * Returns the binary logarithm of x.
 */
CNumber CMath::lb(const CNumber &x)
{
    return CMath::ln(x) / HMath::ln(2);
}

/**
 * Returns the logarithm of x to base.
 * If x is non positive, returns NaN.
 */
CNumber CMath::log(const CNumber &base, const CNumber &x)
{
    return CMath::ln(x) / CMath::ln(base);
}

/**
 * Returns the complex sine of x. Note that x must be in radians.
 */
CNumber CMath::sin(const CNumber &x)
{
    // cf. https://en.wikipedia.org/wiki/Sine#Sine_with_a_complex_argument.
    return CNumber(HMath::sin(x.real) * HMath::cosh(x.imag), HMath::cos(x.real) * HMath::sinh(x.imag));
}

/**
 * Returns the cosine of x. Note that x must be in radians.
 */
CNumber CMath::cos(const CNumber &x)
{
    // Expanded using Wolfram Mathematica 9.0.
    return CNumber(HMath::cos(x.real) * HMath::cosh(x.imag), -HMath::sin(x.real) * HMath::sinh(x.imag));
}

/**
 * Returns the tangent of x. Note that x must be in radians.
 */
CNumber CMath::tan(const CNumber &x)
{
    return CMath::sin(x) / CMath::cos(x);
}

/**
 * Returns the hyperbolic sine of x.
 */
CNumber CMath::sinh(const CNumber &x)
{
    return (exp(x) - exp(-x)) / HNumber(2);
}

/**
 * Returns the hyperbolic cosine of x.
 */
CNumber CMath::cosh(const CNumber &x)
{
    return (exp(x) + exp(-x)) / HNumber(2);
}

/**
 * Returns the hyperbolic tangent of x.
 */
CNumber CMath::tanh(const CNumber &x)
{
    return sinh(x) / cosh(x);
}

/**
 * Returns the cotangent of x. Note that x must be in radians.
 */
CNumber CMath::cot(const CNumber &x)
{
    return cos(x) / sin(x);
}

/**
 * Returns the secant of x. Note that x must be in radians.
 */
CNumber CMath::sec(const CNumber &x)
{
    return CNumber(1) / cos(x);
}

/**
 * Returns the cosecant of x. Note that x must be in radians.
 */
CNumber CMath::csc(const CNumber &x)
{
    return CNumber(1) / sin(x);
}

/**
 * Returns the area hyperbolic sine of x.
 */
CNumber CMath::arsinh(const CNumber &x)
{
    return CMath::ln(x + CMath::sqrt(x * x + CNumber(1)));
}

/**
 * Returns the area hyperbolic cosine of x.
 */
CNumber CMath::arcosh(const CNumber &x)
{
    return CMath::ln(x + CMath::sqrt(x + CNumber(1)) * CMath::sqrt(x - CNumber(1)));
}

/**
 * Returns the area hyperbolic tangent of x.
 */
CNumber CMath::artanh(const CNumber &x)
{
    return (CNumber("0.5") * CMath::ln(CNumber(1) + x)) - (CNumber("0.5") * CMath::ln(CNumber(1) - x));
}

/**
 * Returns the phase of x.
 */
CNumber CMath::phase(const CNumber &x)
{
    return HMath::arctan2(x.real, x.imag);
}

/**
 * Returns the arc tangent of x.
 */
CNumber CMath::arctan(const CNumber &x)
{
    return CMath::i() * (CMath::ln(CNumber(1) - CMath::i() * x) - CMath::ln(CNumber(1) + CMath::i() * x)) / 2;
}

/**
 * Returns the arc sine of x.
 */
CNumber CMath::arcsin(const CNumber &x)
{
    return -CMath::i() * CMath::ln(CMath::i() * x + sqrt(CNumber(1) - x * x));
}

/**
 * Returns the arc cosine of x.
 */
CNumber CMath::arccos(const CNumber &x)
{
    return -CMath::i() * CMath::ln(x + sqrt(x * x - CNumber(1)));
}

/**
 * Converts an angle from radians to degrees.
 * Also accepts complex arguments.
 */
CNumber CMath::rad2deg(const CNumber &x)
{
    return CNumber(HMath::rad2deg(x.real), HMath::rad2deg(x.imag));
}

/**
 * Converts an angle from degrees to radians.
 * Also accepts complex arguments.
 */
CNumber CMath::deg2rad(const CNumber &x)
{
    return CNumber(HMath::deg2rad(x.real), HMath::deg2rad(x.imag));
}

/**
 * Converts an angle from radians to gons.
 * Also accepts complex arguments.
 */
CNumber CMath::rad2gon(const CNumber &x)
{
    return CNumber(HMath::rad2gon(x.real), HMath::rad2gon(x.imag));
}

/**
 * Converts an angle from gons to radians.
 * Also accepts complex arguments.
 */
CNumber CMath::gon2rad(const CNumber &x)
{
    return CNumber(HMath::gon2rad(x.real), HMath::gon2rad(x.imag));
}

// Wrappers towards functions defined only on real numbers
// =======================================================

// NaN is treated like real numbers for the purposes of wrappers.
#define ENSURE_REAL(number, error) \
    if((number).isNan() || !(number).isNearReal()) \
        return CMath::nan(error);

#define REAL_WRAPPER_CNUMBER_1(fct, error) \
    CNumber CNumber::fct() const \
    { \
        ENSURE_REAL(*this, error); \
        return CNumber(this->real.fct()); \
    }

#define REAL_WRAPPER_CNUMBER_2(fct, error) \
    CNumber CNumber::fct(const CNumber& x) const \
    { \
        ENSURE_REAL(*this, error); \
        ENSURE_REAL(x, error); \
        return CNumber(this->real.fct(x.real)); \
    }

#define REAL_WRAPPER_CNUMBER_3(fct, error) \
    CNumber& CNumber::fct(const CNumber& x) \
    { \
        if(!this->isReal()) { \
            *this = CMath::nan(error); \
            return *this; \
        } \
        if (!x.isReal()) { \
            *this = CMath::nan(error); \
            return *this; \
        } \
        this->real.fct(x.real); \
        return *this; \
    }

#define REAL_WRAPPER_CNUMBER_4(fct, error) \
    int CNumber::fct() const \
    { \
        if (!this->isNearReal()) \
            return 0; /* FIXME: Better fail value. */ \
        return this->real.fct(); \
    }

#define REAL_WRAPPER_CMATH_NUM(fct, error) \
    CNumber CMath::fct(const CNumber& x) \
    { \
        ENSURE_REAL(x, error); \
        return CNumber(HMath::fct(x.real)); \
    }

#define REAL_WRAPPER_CMATH_NUM_NUM(fct, error) \
    CNumber CMath::fct(const CNumber& x1, const CNumber& x2) \
    { \
        ENSURE_REAL(x1, error); \
        ENSURE_REAL(x2, error); \
        return CNumber(HMath::fct(x1.real, x2.real)); \
    }

#define REAL_WRAPPER_CMATH_NUM_INT(fct, error) \
    CNumber CMath::fct(const CNumber& x1, int n) \
    { \
        ENSURE_REAL(x1, error); \
        return CNumber(HMath::fct(x1.real, n)); \
    }

#define REAL_WRAPPER_CMATH_NUM_NUM_NUM(fct, error) \
    CNumber CMath::fct(const CNumber& x1, const CNumber& x2, const CNumber& x3) \
    { \
        ENSURE_REAL(x1, error); \
        ENSURE_REAL(x2, error); \
        ENSURE_REAL(x3, error); \
        return CNumber(HMath::fct(x1.real, x2.real, x3.real)); \
    }

#define REAL_WRAPPER_CMATH_NUM_NUM_NUM_NUM(fct, error) \
    CNumber CMath::fct(const CNumber& x1, const CNumber& x2, const CNumber& x3, const CNumber& x4) \
    { \
        ENSURE_REAL(x1, error); \
        ENSURE_REAL(x2, error); \
        ENSURE_REAL(x3, error); \
        ENSURE_REAL(x4, error); \
        return CNumber(HMath::fct(x1.real, x2.real, x3.real, x4.real)); \
    }

// CNumber
REAL_WRAPPER_CNUMBER_4(toInt, OutOfDomain)
REAL_WRAPPER_CNUMBER_2(operator%, OutOfDomain)
REAL_WRAPPER_CNUMBER_2(operator&, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_3(operator&=, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_2(operator|, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_3(operator|=, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_2(operator^, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_3(operator^=, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_1(operator~, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_2(operator>>, OutOfLogicRange)
REAL_WRAPPER_CNUMBER_2(operator<<, OutOfLogicRange)
// CMath GENERAL MATH
REAL_WRAPPER_CMATH_NUM(integer, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(frac, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(floor, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(ceil, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(gcd, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(idiv, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_INT(round, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_INT(trunc, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(cbrt, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(sgn, OutOfDomain)
// CMath EXPONENTIAL FUNCTION AND RELATED
REAL_WRAPPER_CMATH_NUM_NUM(arctan2, OutOfDomain)
// CMath TRIGONOMETRY
/* All trigonometry functions accept complex numbers */
// CMath HIGHER MATH FUNCTIONS
REAL_WRAPPER_CMATH_NUM_NUM(factorial, NotImplemented)
REAL_WRAPPER_CMATH_NUM(erf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(erfc, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(gamma, NotImplemented)
REAL_WRAPPER_CMATH_NUM(lnGamma, NotImplemented)
// CMath PROBABILITY
REAL_WRAPPER_CMATH_NUM_NUM(nCr, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(nPr, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM(binomialPmf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM(binomialCdf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(binomialMean, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(binomialVariance, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM_NUM(hypergeometricPmf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM_NUM(hypergeometricCdf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM(hypergeometricMean, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM(hypergeometricVariance, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(poissonPmf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM(poissonCdf, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(poissonMean, OutOfDomain)
REAL_WRAPPER_CMATH_NUM(poissonVariance, OutOfDomain)
// CMath LOGIC
REAL_WRAPPER_CMATH_NUM_NUM(mask, OutOfLogicRange)
REAL_WRAPPER_CMATH_NUM_NUM(sgnext, OutOfLogicRange)
REAL_WRAPPER_CMATH_NUM_NUM(ashr, OutOfLogicRange)
REAL_WRAPPER_CMATH_NUM_NUM(lshr, OutOfLogicRange)
REAL_WRAPPER_CMATH_NUM_NUM(rosh, OutOfLogicRange)
REAL_WRAPPER_CMATH_NUM_NUM(rcsh, OutOfLogicRange)

// CMath IEEE-754 CONVERSION
REAL_WRAPPER_CMATH_NUM_NUM_NUM(decodeIeee754, OutOfDomain)
REAL_WRAPPER_CMATH_NUM_NUM_NUM_NUM(decodeIeee754, OutOfDomain)

CNumber CMath::encodeIeee754(const CNumber &val, const CNumber &exp_bits, const CNumber &significand_bits,
                             const CNumber &exp_bias)
{
    ENSURE_REAL(exp_bits, OutOfDomain);
    ENSURE_REAL(significand_bits, OutOfDomain);
    ENSURE_REAL(exp_bias, OutOfDomain);
    if (!val.isNan() && !val.isNearReal())
        return CMath::nan(OutOfDomain);
    return CNumber(HMath::encodeIeee754(val.real, exp_bits.real, significand_bits.real, exp_bias.real));
}

CNumber CMath::encodeIeee754(const CNumber &val, const CNumber &exp_bits, const CNumber &significand_bits)
{
    ENSURE_REAL(exp_bits, OutOfDomain);
    ENSURE_REAL(significand_bits, OutOfDomain);
    if (!val.isNan() && !val.isNearReal())
        return CMath::nan(OutOfDomain);
    return CNumber(HMath::encodeIeee754(val.real, exp_bits.real, significand_bits.real));
}
