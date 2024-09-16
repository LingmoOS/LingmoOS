// This file is part of the SpeedCrunch project
// Copyright (C) 2004-2006 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2007, 2009 Wolf Lammen
// Copyright (C) 2007-2009, 2013, 2014 @heldercorreia
// Copyright (C) 2009 Andreas Scherer <andreas_coder@freenet.de>
// Copyright (C) 2011 Enrico Rós <enrico.ros@gmail.com>
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

#include "functions.h"

#include "settings.h"
#include "../math/hmath.h"
#include "../math/cmath.h"

#include <QCoreApplication>
#include <QHash>

#include <algorithm>
#include <functional>
#include <cfloat>
#include <cmath>
#include <numeric>

#define FUNCTION_INSERT(ID) insert(new Function(#ID, function_ ## ID, this))
#define FUNCTION_USAGE(ID, USAGE) find(#ID)->setUsage(QString::fromLatin1(USAGE));
#define FUNCTION_USAGE_TR(ID, USAGE) find(#ID)->setUsage(USAGE);
#define FUNCTION_NAME(ID, NAME) find(#ID)->setName(NAME)

#define ENSURE_MINIMUM_ARGUMENT_COUNT(i) \
    if (args.count() < i) { \
        f->setError(InvalidParamCount); \
        return CMath::nan(InvalidParamCount); \
    }

#define ENSURE_ARGUMENT_COUNT(i) \
    if (args.count() != (i)) { \
        f->setError(InvalidParamCount); \
        return CMath::nan(InvalidParamCount); \
    }

#define ENSURE_EITHER_ARGUMENT_COUNT(i, j) \
    if (args.count() != (i) && args.count() != (j)) { \
        f->setError(InvalidParamCount); \
        return CMath::nan(InvalidParamCount); \
    }

#define ENSURE_SAME_DIMENSION() \
    for(int i=0; i<args.count()-1; ++i) { \
        if(!args.at(i).sameDimension(args.at((i)+1))) \
            return DMath::nan(InvalidDimension);\
    }

#define ENSURE_REAL_ARGUMENT(i) \
    if (!args[i].isReal()) { \
        f->setError(OutOfDomain); \
        return CMath::nan(); \
    }

#define ENSURE_REAL_ARGUMENTS() \
    for (int i = 0; i < args.count(); i++) { \
        ENSURE_REAL_ARGUMENT(i); \
    }

#define CONVERT_ARGUMENT_ANGLE(angle) \
    if (Settings::instance()->angleUnit == 'd') { \
        if (angle.isReal()) \
            angle = DMath::deg2rad(angle); \
        else { \
            f->setError(OutOfDomain); \
            return DMath::nan(); \
        } \
    } \
    else if (Settings::instance()->angleUnit == 'g') { \
        if (angle.isReal()) \
            angle = DMath::gon2rad(angle); \
        else { \
            f->setError(OutOfDomain); \
            return DMath::nan(); \
        } \
    }

#define CONVERT_RESULT_ANGLE(result) \
    if (Settings::instance()->angleUnit == 'd') \
        result = DMath::rad2deg(result); \
    else if (Settings::instance()->angleUnit == 'g') \
        result = DMath::rad2gon(result);

static FunctionRepo *s_FunctionRepoInstance = 0;

// FIXME: destructor seems not to be called
static void s_deleteFunctions()
{
    delete s_FunctionRepoInstance;
}

Quantity Function::exec(const Function::ArgumentList &args)
{
    if (!m_ptr)
        return CMath::nan();
    setError(Success);
    Quantity result = (*m_ptr)(this, args);
    if (result.error())
        setError(result.error());
    return result;
}

Quantity function_abs(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::abs(args.at(0));
}

Quantity function_average(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return std::accumulate(args.begin() + 1, args.end(), *args.begin()) / Quantity(args.count());
}

Quantity function_absdev(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    Quantity mean = function_average(f, args);
    if (mean.isNan())
        return mean;   // pass the error along
    Quantity acc = 0;
    for (int i = 0; i < args.count(); ++i)
        acc += DMath::abs(args.at(i) - mean);
    return acc / Quantity(args.count());
}

Quantity function_int(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::integer(args[0]);
}

Quantity function_trunc(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(1, 2);
    Quantity num = args.at(0);
    if (args.count() == 2) {
        Quantity argprec = args.at(1);
        if (argprec != 0) {
            if (!argprec.isInteger()) {
                f->setError(OutOfDomain);
                return DMath::nan();
            }
            int prec = argprec.numericValue().toInt();
            if (prec)
                return DMath::trunc(num, prec);
            // The second parameter exceeds the integer limits.
            if (argprec < 0)
                return Quantity(0);
            return num;
        }
    }
    return DMath::trunc(num);
}

Quantity function_frac(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::frac(args[0]);
}

Quantity function_floor(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::floor(args[0]);
}

Quantity function_ceil(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::ceil(args[0]);
}

Quantity function_gcd(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    for (int i = 0; i < args.count(); ++i)
        if (!args[i].isInteger()) {
            f->setError(OutOfDomain);
            return DMath::nan();
        }
    return std::accumulate(args.begin() + 1, args.end(), args.at(0), DMath::gcd);
}

Quantity function_round(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(1, 2);
    Quantity num = args.at(0);
    if (args.count() == 2) {
        Quantity argPrecision = args.at(1);
        if (argPrecision != 0) {
            if (!argPrecision.isInteger()) {
                f->setError(OutOfDomain);
                return DMath::nan();
            }
            int prec = argPrecision.numericValue().toInt();
            if (prec)
                return DMath::round(num, prec);
            // The second parameter exceeds the integer limits.
            if (argPrecision < 0)
                return Quantity(0);
            return num;
        }
    }
    return DMath::round(num);
}

Quantity function_sqrt(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::sqrt(args[0]);
}

Quantity function_variance(Function *f, const Function::ArgumentList &args)
{
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);

    Quantity mean = function_average(f, args);
    if (mean.isNan())
        return mean;

    Quantity acc(DMath::real(args[0] - mean)*DMath::real(args[0] - mean)
                 + DMath::imag(args[0] - mean)*DMath::imag(args[0] - mean));
    for (int i = 1; i < args.count(); ++i) {
        Quantity q(args[i] - mean);
        acc += DMath::real(q) * DMath::real(q) + DMath::imag(q) * DMath::imag(q);
    }

    return acc / Quantity(args.count());
}

Quantity function_stddev(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return DMath::sqrt(function_variance(f, args));
}

Quantity function_cbrt(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::cbrt(args[0]);
}

Quantity function_exp(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::exp(args[0]);
}

Quantity function_ln(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::ln(args[0]);
}

Quantity function_lg(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::lg(args[0]);
}

Quantity function_lb(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::lb(args[0]);
}

Quantity function_log(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::log(args.at(0), args.at(1));
}

Quantity function_real(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::real(args.at(0));
}

Quantity function_imag(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::imag(args.at(0));
}

Quantity function_conj(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::conj(args.at(0));
}

Quantity function_phase(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = DMath::phase(args.at(0));
    CONVERT_RESULT_ANGLE(angle);
    return angle;
}


Quantity function_sin(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return DMath::sin(angle);
}

Quantity function_cos(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return DMath::cos(angle);
}

Quantity function_tan(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return DMath::tan(angle);
}

Quantity function_cot(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return DMath::cot(angle);
}

Quantity function_sec(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return DMath::sec(angle);
}

Quantity function_csc(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity angle = args.at(0);
    CONVERT_ARGUMENT_ANGLE(angle);
    return DMath::csc(angle);
}

Quantity function_arcsin(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity result;
    result = DMath::arcsin(args.at(0));
    CONVERT_RESULT_ANGLE(result);
    return result;
}

Quantity function_arccos(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity result;
    result = DMath::arccos(args.at(0));
    CONVERT_RESULT_ANGLE(result);
    return result;
}

Quantity function_arctan(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity result;
    result = DMath::arctan(args.at(0));
    CONVERT_RESULT_ANGLE(result);
    return result;
}

Quantity function_arctan2(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(2);
    Quantity result;
    result = DMath::arctan2(args.at(0), args.at(1));
    CONVERT_RESULT_ANGLE(result);
    return result;
}

//add 20200619 arccot(x)
Quantity function_arccot(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    Quantity result;
    result = DMath::arctan(args.at(0));
    result = (DMath::pi() / Quantity(2)) - result;
    CONVERT_RESULT_ANGLE(result);
    return result;
}


Quantity function_sinh(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::sinh(args[0]);
}

Quantity function_cosh(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::cosh(args[0]);
}

Quantity function_tanh(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::tanh(args[0]);
}

Quantity function_arsinh(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::arsinh(args[0]);
}

Quantity function_arcosh(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::arcosh(args[0]);
}

Quantity function_artanh(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::artanh(args[0]);
}

Quantity function_erf(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::erf(args[0]);
}

Quantity function_erfc(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::erfc(args[0]);
}

Quantity function_gamma(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::gamma(args[0]);
}

Quantity function_lngamma(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    ENSURE_REAL_ARGUMENT(0);
    return DMath::lnGamma(args[0]);
}

Quantity function_sgn(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::sgn(args[0]);
}

Quantity function_ncr(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::nCr(args.at(0), args.at(1));
}

Quantity function_npr(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::nPr(args.at(0), args.at(1));
}

Quantity function_degrees(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::rad2deg(args[0]);
}

Quantity function_radians(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::deg2rad(args[0]);
}

Quantity function_gradians(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::rad2gon(args[0]);
}

Quantity function_max(Function *f, const Function::ArgumentList &args)
{
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    ENSURE_REAL_ARGUMENTS()
    ENSURE_SAME_DIMENSION()
    return *std::max_element(args.begin(), args.end());
}

Quantity function_median(Function *f, const Function::ArgumentList &args)
{
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    ENSURE_REAL_ARGUMENTS()
    ENSURE_SAME_DIMENSION()

    Function::ArgumentList sortedArgs = args;
    std::sort(sortedArgs.begin(), sortedArgs.end());

    if ((args.count() & 1) == 1)
        return sortedArgs.at((args.count() - 1) / 2);

    const int centerLeft = args.count() / 2 - 1;
    return (sortedArgs.at(centerLeft) + sortedArgs.at(centerLeft + 1)) / Quantity(2);
}

Quantity function_min(Function *f, const Function::ArgumentList &args)
{
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    ENSURE_REAL_ARGUMENTS()
    ENSURE_SAME_DIMENSION()
    return *std::min_element(args.begin(), args.end());
}

Quantity function_sum(Function *f, const Function::ArgumentList &args)
{
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return std::accumulate(args.begin(), args.end(), Quantity(0));
}

Quantity function_product(Function *f, const Function::ArgumentList &args)
{
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return std::accumulate(args.begin(), args.end(), Quantity(1), std::multiplies<Quantity>());
}

Quantity function_geomean(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);

    Quantity result = std::accumulate(args.begin(), args.end(), Quantity(1),
                                      std::multiplies<Quantity>());

    if (result <= Quantity(0))
        return DMath::nan(OutOfDomain);

    if (args.count() == 1)
        return result;

    if (args.count() == 2)
        return DMath::sqrt(result);

    return  DMath::raise(result, Quantity(1) / Quantity(args.count()));
}

Quantity function_dec(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return Quantity(args.at(0)).setFormat(Quantity::Format::Decimal() + Quantity(args.at(0)).format());
}

Quantity function_hex(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return Quantity(args.at(0)).setFormat(Quantity::Format::Fixed() + Quantity::Format::Hexadecimal() + Quantity(args.at(0)).format());
}

Quantity function_oct(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return Quantity(args.at(0)).setFormat(Quantity::Format::Fixed() + Quantity::Format::Octal() + Quantity(args.at(0)).format());
}

Quantity function_bin(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return Quantity(args.at(0)).setFormat(Quantity::Format::Fixed() + Quantity::Format::Binary() + Quantity(args.at(0)).format());
}

Quantity function_cart(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return Quantity(args.at(0)).setFormat(Quantity::Format::Cartesian() + Quantity(args.at(0)).format());
}

Quantity function_polar(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(1);
    return Quantity(args.at(0)).setFormat(Quantity::Format::Polar() + Quantity(args.at(0)).format());
}

Quantity function_binompmf(Function *f, const Function::ArgumentList &args)
{
    ENSURE_ARGUMENT_COUNT(3);
    return DMath::binomialPmf(args.at(0), args.at(1), args.at(2));
}

Quantity function_binomcdf(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(3);
    return DMath::binomialCdf(args.at(0), args.at(1), args.at(2));
}

Quantity function_binommean(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::binomialMean(args.at(0), args.at(1));
}

Quantity function_binomvar(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::binomialVariance(args.at(0), args.at(1));
}

Quantity function_hyperpmf(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(4);
    return DMath::hypergeometricPmf(args.at(0), args.at(1), args.at(2), args.at(3));
}

Quantity function_hypercdf(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(4);
    return DMath::hypergeometricCdf(args.at(0), args.at(1), args.at(2), args.at(3));
}

Quantity function_hypermean(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(3);
    return DMath::hypergeometricMean(args.at(0), args.at(1), args.at(2));
}

Quantity function_hypervar(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(3);
    return DMath::hypergeometricVariance(args.at(0), args.at(1), args.at(2));
}

Quantity function_poipmf(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::poissonPmf(args.at(0), args.at(1));
}

Quantity function_poicdf(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::poissonCdf(args.at(0), args.at(1));
}

Quantity function_poimean(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::poissonMean(args.at(0));
}

Quantity function_poivar(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::poissonVariance(args.at(0));
}

Quantity function_mask(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::mask(args.at(0), args.at(1));
}

Quantity function_unmask(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::sgnext(args.at(0), args.at(1));
}

Quantity function_not(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return ~args.at(0);
}

Quantity function_and(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return std::accumulate(args.begin(), args.end(), Quantity(-1),
                           std::mem_fun_ref(&Quantity::operator&));
}

Quantity function_or(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return std::accumulate(args.begin(), args.end(), Quantity(0),
                           std::mem_fun_ref(&Quantity::operator|));
}

Quantity function_xor(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_MINIMUM_ARGUMENT_COUNT(2);
    return std::accumulate(args.begin(), args.end(), Quantity(0),
                           std::mem_fun_ref(&Quantity::operator^));
}

Quantity function_shl(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::ashr(args.at(0), -args.at(1));
}

Quantity function_shr(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::ashr(args.at(0), args.at(1));
}

Quantity function_idiv(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::idiv(args.at(0), args.at(1));
}

Quantity function_mod(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return args.at(0) % args.at(1);
}

Quantity function_ieee754_decode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(3, 4);
    if (args.count() == 3) {
        return DMath::decodeIeee754(args.at(0), args.at(1), args.at(2));
    } else {
        return DMath::decodeIeee754(args.at(0), args.at(1), args.at(2), args.at(3));
    }
}

Quantity function_ieee754_encode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_EITHER_ARGUMENT_COUNT(3, 4);
    if (args.count() == 3) {
        return DMath::encodeIeee754(args.at(0), args.at(1), args.at(2));
    } else {
        return DMath::encodeIeee754(args.at(0), args.at(1), args.at(2), args.at(3));
    }
}

Quantity function_ieee754_half_decode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::decodeIeee754(args.at(0), 5, 10);
}

Quantity function_ieee754_half_encode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::encodeIeee754(args.at(0), 5, 10);
}

Quantity function_ieee754_single_decode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::decodeIeee754(args.at(0), 8, 23);
}

Quantity function_ieee754_single_encode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::encodeIeee754(args.at(0), 8, 23);
}

Quantity function_ieee754_double_decode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::decodeIeee754(args.at(0), 11, 52);
}

Quantity function_ieee754_double_encode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::encodeIeee754(args.at(0), 11, 52);
}

Quantity function_ieee754_quad_decode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::decodeIeee754(args.at(0), 15, 112);
}

Quantity function_ieee754_quad_encode(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(1);
    return DMath::encodeIeee754(args.at(0), 15, 112);
}

//add 20200604 yroot(x)
Quantity function_yroot(Function *f, const Function::ArgumentList &args)
{
    /* TODO : complex mode switch for this function */
    ENSURE_ARGUMENT_COUNT(2);
    return DMath::raise(args.at(0), Quantity(1) / args.at(1));
}


void FunctionRepo::createFunctions()
{
    // Analysis.
    FUNCTION_INSERT(abs);
    FUNCTION_INSERT(absdev);
    FUNCTION_INSERT(average);
    FUNCTION_INSERT(bin);
    FUNCTION_INSERT(cbrt);
    FUNCTION_INSERT(ceil);
    FUNCTION_INSERT(dec);
    FUNCTION_INSERT(floor);
    FUNCTION_INSERT(frac);
    FUNCTION_INSERT(gamma);
    FUNCTION_INSERT(geomean);
    FUNCTION_INSERT(hex);
    FUNCTION_INSERT(int);
    FUNCTION_INSERT(lngamma);
    FUNCTION_INSERT(max);
    FUNCTION_INSERT(min);
    FUNCTION_INSERT(oct);
    FUNCTION_INSERT(product);
    FUNCTION_INSERT(round);
    FUNCTION_INSERT(sgn);
    FUNCTION_INSERT(sqrt);
    FUNCTION_INSERT(stddev);
    FUNCTION_INSERT(sum);
    FUNCTION_INSERT(trunc);
    FUNCTION_INSERT(variance);
    FUNCTION_INSERT(yroot); //add 20200604 yroot(x)

    // Complex.
    FUNCTION_INSERT(real);
    FUNCTION_INSERT(imag);
    FUNCTION_INSERT(conj);
    FUNCTION_INSERT(phase);
    FUNCTION_INSERT(polar);
    FUNCTION_INSERT(cart);

    // Discrete.
    FUNCTION_INSERT(gcd);
    FUNCTION_INSERT(ncr);
    FUNCTION_INSERT(npr);

    // Probability.
    FUNCTION_INSERT(binomcdf);
    FUNCTION_INSERT(binommean);
    FUNCTION_INSERT(binompmf);
    FUNCTION_INSERT(binomvar);
    FUNCTION_INSERT(erf);
    FUNCTION_INSERT(erfc);
    FUNCTION_INSERT(hypercdf);
    FUNCTION_INSERT(hypermean);
    FUNCTION_INSERT(hyperpmf);
    FUNCTION_INSERT(hypervar);
    FUNCTION_INSERT(median);
    FUNCTION_INSERT(poicdf);
    FUNCTION_INSERT(poimean);
    FUNCTION_INSERT(poipmf);
    FUNCTION_INSERT(poivar);

    // Trigonometry.
    FUNCTION_INSERT(arccos);
    FUNCTION_INSERT(arcosh);
    FUNCTION_INSERT(arsinh);
    FUNCTION_INSERT(artanh);
    FUNCTION_INSERT(arcsin);
    FUNCTION_INSERT(arctan);
    FUNCTION_INSERT(arctan2);
    FUNCTION_INSERT(arccot);
    FUNCTION_INSERT(cos);
    FUNCTION_INSERT(cosh);
    FUNCTION_INSERT(cot);
    FUNCTION_INSERT(csc);
    FUNCTION_INSERT(degrees);
    FUNCTION_INSERT(exp);
    FUNCTION_INSERT(gradians);
    FUNCTION_INSERT(lb);
    FUNCTION_INSERT(lg);
    FUNCTION_INSERT(ln);
    FUNCTION_INSERT(log);
    FUNCTION_INSERT(radians);
    FUNCTION_INSERT(sec);
    FUNCTION_INSERT(sin);
    FUNCTION_INSERT(sinh);
    FUNCTION_INSERT(tan);
    FUNCTION_INSERT(tanh);

    // Logic.
    FUNCTION_INSERT(mask);
    FUNCTION_INSERT(unmask);
    FUNCTION_INSERT(not);
    FUNCTION_INSERT( and);
    FUNCTION_INSERT( or);
    FUNCTION_INSERT(xor);
    FUNCTION_INSERT(shl);
    FUNCTION_INSERT(shr);
    FUNCTION_INSERT(idiv);
    FUNCTION_INSERT(mod);

    // IEEE-754.
    FUNCTION_INSERT(ieee754_decode);
    FUNCTION_INSERT(ieee754_encode);
    FUNCTION_INSERT(ieee754_half_decode);
    FUNCTION_INSERT(ieee754_half_encode);
    FUNCTION_INSERT(ieee754_single_decode);
    FUNCTION_INSERT(ieee754_single_encode);
    FUNCTION_INSERT(ieee754_double_decode);
    FUNCTION_INSERT(ieee754_double_encode);
    FUNCTION_INSERT(ieee754_quad_decode);
    FUNCTION_INSERT(ieee754_quad_encode);
}

FunctionRepo *FunctionRepo::instance()
{
    if (!s_FunctionRepoInstance) {
        s_FunctionRepoInstance = new FunctionRepo;
        qAddPostRoutine(s_deleteFunctions);
    }
    return s_FunctionRepoInstance;
}

FunctionRepo::FunctionRepo()
{
    createFunctions();
    setNonTranslatableFunctionUsages();
    retranslateText();
}

void FunctionRepo::insert(Function *function)
{
    if (!function)
        return;
    m_functions.insert(function->identifier().toUpper(), function);
}

Function *FunctionRepo::find(const QString &identifier) const
{
    if (identifier.isNull())
        return 0;
    return m_functions.value(identifier.toUpper(), 0);
}

QStringList FunctionRepo::getIdentifiers() const
{
    QStringList result = m_functions.keys();
    std::transform(result.begin(), result.end(), result.begin(), [](const QString & s) { return s.toLower(); });
    return result;
}

void FunctionRepo::setNonTranslatableFunctionUsages()
{
    FUNCTION_USAGE(abs, "x");
    FUNCTION_USAGE(absdev, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(arccos, "x");
    FUNCTION_USAGE( and, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(arcosh, "x");
    FUNCTION_USAGE(arsinh, "x");
    FUNCTION_USAGE(artanh, "x");
    FUNCTION_USAGE(arcsin, "x");
    FUNCTION_USAGE(arctan, "x");
    FUNCTION_USAGE(arctan2, "x; y");
    FUNCTION_USAGE(arccot, "x");
    FUNCTION_USAGE(average, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(bin, "n");
    FUNCTION_USAGE(cart, "x");
    FUNCTION_USAGE(cbrt, "x");
    FUNCTION_USAGE(ceil, "x");
    FUNCTION_USAGE(conj, "x");
    FUNCTION_USAGE(cos, "x");
    FUNCTION_USAGE(cosh, "x");
    FUNCTION_USAGE(cot, "x");
    FUNCTION_USAGE(csc, "x");
    FUNCTION_USAGE(dec, "x");
    FUNCTION_USAGE(degrees, "x");
    FUNCTION_USAGE(erf, "x");
    FUNCTION_USAGE(erfc, "x");
    FUNCTION_USAGE(exp, "x");
    FUNCTION_USAGE(floor, "x");
    FUNCTION_USAGE(frac, "x");
    FUNCTION_USAGE(gamma, "x");
    FUNCTION_USAGE(gcd, "n<sub>1</sub>; n<sub>2</sub>; ...");
    FUNCTION_USAGE(geomean, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(gradians, "x");
    FUNCTION_USAGE(hex, "n");
    FUNCTION_USAGE(ieee754_half_decode, "x");
    FUNCTION_USAGE(ieee754_half_encode, "x");
    FUNCTION_USAGE(ieee754_single_decode, "x");
    FUNCTION_USAGE(ieee754_single_encode, "x");
    FUNCTION_USAGE(ieee754_double_decode, "x");
    FUNCTION_USAGE(ieee754_double_encode, "x");
    FUNCTION_USAGE(ieee754_quad_decode, "x");
    FUNCTION_USAGE(ieee754_quad_encode, "x");
    FUNCTION_USAGE(int, "x");
    FUNCTION_USAGE(imag, "x");
    FUNCTION_USAGE(lb, "x");
    FUNCTION_USAGE(lg, "x");
    FUNCTION_USAGE(ln, "x");
    FUNCTION_USAGE(lngamma, "x");
    FUNCTION_USAGE(max, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(median, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(min, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(ncr, "x<sub>1</sub>; x<sub>2</sub>");
    FUNCTION_USAGE(not, "n");
    FUNCTION_USAGE(npr, "x<sub>1</sub>; x<sub>2</sub>");
    FUNCTION_USAGE(oct, "n");
    FUNCTION_USAGE( or, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(polar, "x");
    FUNCTION_USAGE(product, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(phase, "x");
    FUNCTION_USAGE(radians, "x");
    FUNCTION_USAGE(real, "x");
    FUNCTION_USAGE(sec, "x)");
    FUNCTION_USAGE(sgn, "x");
    FUNCTION_USAGE(sin, "x");
    FUNCTION_USAGE(sinh, "x");
    FUNCTION_USAGE(sqrt, "x");
    FUNCTION_USAGE(stddev, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(sum, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(tan, "x");
    FUNCTION_USAGE(tanh, "x");
    FUNCTION_USAGE(trunc, "x");
    FUNCTION_USAGE(variance, "x<sub>1</sub>; x<sub>2</sub>; ...");
    FUNCTION_USAGE(xor, "x<sub>1</sub>; x<sub>2</sub>; ...");
}

void FunctionRepo::setTranslatableFunctionUsages()
{
    FUNCTION_USAGE_TR(binomcdf, /*tr*/("max; trials; probability"));
    FUNCTION_USAGE_TR(binommean, /*tr*/("trials; probability"));
    FUNCTION_USAGE_TR(binompmf, /*tr*/("hits; trials; probability"));
    FUNCTION_USAGE_TR(binomvar, /*tr*/("trials; probability"));
    FUNCTION_USAGE_TR(hypercdf, /*tr*/("max; total; hits; trials"));
    FUNCTION_USAGE_TR(hypermean, /*tr*/("total; hits; trials"));
    FUNCTION_USAGE_TR(hyperpmf, /*tr*/("count; total; hits; trials"));
    FUNCTION_USAGE_TR(hypervar, /*tr*/("total; hits; trials"));
    FUNCTION_USAGE_TR(idiv, /*tr*/("dividend; divisor"));
    FUNCTION_USAGE_TR(ieee754_decode, /*tr*/("x; exponent_bits; significand_bits [; exponent_bias]"));
    FUNCTION_USAGE_TR(ieee754_encode, /*tr*/("x; exponent_bits; significand_bits [; exponent_bias]"));
    FUNCTION_USAGE_TR(log, /*tr*/("base; x"));
    FUNCTION_USAGE_TR(mask, /*tr*/("n; bits"));
    FUNCTION_USAGE_TR(mod, /*tr*/("value; modulo"));
    FUNCTION_USAGE_TR(poicdf, /*tr*/("events; average_events"));
    FUNCTION_USAGE_TR(poimean, /*tr*/("average_events"));
    FUNCTION_USAGE_TR(poipmf, /*tr*/("events; average_events"));
    FUNCTION_USAGE_TR(poivar, /*tr*/("average_events"));
    FUNCTION_USAGE_TR(round, /*tr*/("x [; precision]"));
    FUNCTION_USAGE_TR(shl, /*tr*/("n; bits"));
    FUNCTION_USAGE_TR(shr, /*tr*/("n; bits"));
    FUNCTION_USAGE_TR(unmask, /*tr*/("n; bits"));
    FUNCTION_USAGE_TR(yroot, ("x; y"));//add 20200604 yroot(x)
}

void FunctionRepo::setFunctionNames()
{
    FUNCTION_NAME(abs, /*tr*/("Absolute Value"));
    FUNCTION_NAME(absdev, /*tr*/("Absolute Deviation"));
    FUNCTION_NAME(arccos, /*tr*/("Arc Cosine"));
    FUNCTION_NAME( and, /*tr*/("Logical AND"));
    FUNCTION_NAME(arcosh, /*tr*/("Area Hyperbolic Cosine"));
    FUNCTION_NAME(arsinh, /*tr*/("Area Hyperbolic Sine"));
    FUNCTION_NAME(artanh, /*tr*/("Area Hyperbolic Tangent"));
    FUNCTION_NAME(arcsin, /*tr*/("Arc Sine"));
    FUNCTION_NAME(arctan, /*tr*/("Arc Tangent"));
    FUNCTION_NAME(arctan2, /*tr*/("Arc Tangent with two Arguments"));
    FUNCTION_NAME(arccot, /*tr*/("Arc Cotangent"));
    FUNCTION_NAME(average, /*tr*/("Average (Arithmetic Mean)"));
    FUNCTION_NAME(bin, /*tr*/("Convert to Binary Representation"));
    FUNCTION_NAME(binomcdf, /*tr*/("Binomial Cumulative Distribution Function"));
    FUNCTION_NAME(binommean, /*tr*/("Binomial Distribution Mean"));
    FUNCTION_NAME(binompmf, /*tr*/("Binomial Probability Mass Function"));
    FUNCTION_NAME(binomvar, /*tr*/("Binomial Distribution Variance"));
    FUNCTION_NAME(cart, /*tr*/("Convert to Cartesian Notation"));
    FUNCTION_NAME(cbrt, /*tr*/("Cube Root"));
    FUNCTION_NAME(ceil, /*tr*/("Ceiling"));
    FUNCTION_NAME(conj, /*tr*/("Complex Conjugate"));
    FUNCTION_NAME(cos, /*tr*/("Cosine"));
    FUNCTION_NAME(cosh, /*tr*/("Hyperbolic Cosine"));
    FUNCTION_NAME(cot, /*tr*/("Cotangent"));
    FUNCTION_NAME(csc, /*tr*/("Cosecant"));
    FUNCTION_NAME(dec, /*tr*/("Convert to Decimal Representation"));
    FUNCTION_NAME(degrees, /*tr*/("Degrees of Arc"));
    FUNCTION_NAME(erf, /*tr*/("Error Function"));
    FUNCTION_NAME(erfc, /*tr*/("Complementary Error Function"));
    FUNCTION_NAME(exp, /*tr*/("Exponential"));
    FUNCTION_NAME(floor, /*tr*/("Floor"));
    FUNCTION_NAME(frac, /*tr*/("Fractional Part"));
    FUNCTION_NAME(gamma, /*tr*/("Extension of Factorials [= (x-1)!]"));
    FUNCTION_NAME(gcd, /*tr*/("Greatest Common Divisor"));
    FUNCTION_NAME(geomean, /*tr*/("Geometric Mean"));
    FUNCTION_NAME(gradians, /*tr*/("Gradians of arc"));
    FUNCTION_NAME(hex, /*tr*/("Convert to Hexadecimal Representation"));
    FUNCTION_NAME(hypercdf, /*tr*/("Hypergeometric Cumulative Distribution Function"));
    FUNCTION_NAME(hypermean, /*tr*/("Hypergeometric Distribution Mean"));
    FUNCTION_NAME(hyperpmf, /*tr*/("Hypergeometric Probability Mass Function"));
    FUNCTION_NAME(hypervar, /*tr*/("Hypergeometric Distribution Variance"));
    FUNCTION_NAME(idiv, /*tr*/("Integer Quotient"));
    FUNCTION_NAME(int, /*tr*/("Integer Part"));
    FUNCTION_NAME(imag, /*tr*/("Imaginary Part"));
    FUNCTION_NAME(ieee754_decode, /*tr*/("Decode IEEE-754 Binary Value"));
    FUNCTION_NAME(ieee754_encode, /*tr*/("Encode IEEE-754 Binary Value"));
    FUNCTION_NAME(ieee754_half_decode, /*tr*/("Decode 16-bit Half-Precision Value"));
    FUNCTION_NAME(ieee754_half_encode, /*tr*/("Encode 16-bit Half-Precision Value"));
    FUNCTION_NAME(ieee754_single_decode, /*tr*/("Decode 32-bit Single-Precision Value"));
    FUNCTION_NAME(ieee754_single_encode, /*tr*/("Encode 32-bit Single-Precision Value"));
    FUNCTION_NAME(ieee754_double_decode, /*tr*/("Decode 64-bit Double-Precision Value"));
    FUNCTION_NAME(ieee754_double_encode, /*tr*/("Encode 64-bit Double-Precision Value"));
    FUNCTION_NAME(ieee754_quad_decode, /*tr*/("Decode 128-bit Quad-Precision Value"));
    FUNCTION_NAME(ieee754_quad_encode, /*tr*/("Encode 128-bit Quad-Precision Value"));
    FUNCTION_NAME(lb, /*tr*/("Binary Logarithm"));
    FUNCTION_NAME(lg, /*tr*/("Common Logarithm"));
    FUNCTION_NAME(ln, /*tr*/("Natural Logarithm"));
    FUNCTION_NAME(lngamma, "ln(abs(Gamma))");
    FUNCTION_NAME(log, /*tr*/("Logarithm to Arbitrary Base"));
    FUNCTION_NAME(mask, /*tr*/("Mask to a bit size"));
    FUNCTION_NAME(max, /*tr*/("Maximum"));
    FUNCTION_NAME(median, /*tr*/("Median Value (50th Percentile)"));
    FUNCTION_NAME(min, /*tr*/("Minimum"));
    FUNCTION_NAME(mod, /*tr*/("Modulo"));
    FUNCTION_NAME(ncr, /*tr*/("Combination (Binomial Coefficient)"));
    FUNCTION_NAME(not, /*tr*/("Logical NOT"));
    FUNCTION_NAME(npr, /*tr*/("Permutation (Arrangement)"));
    FUNCTION_NAME(oct, /*tr*/("Convert to Octal Representation"));
    FUNCTION_NAME( or, /*tr*/("Logical OR"));
    FUNCTION_NAME(phase, /*tr*/("Phase of Complex Number"));
    FUNCTION_NAME(poicdf, /*tr*/("Poissonian Cumulative Distribution Function"));
    FUNCTION_NAME(poimean, /*tr*/("Poissonian Distribution Mean"));
    FUNCTION_NAME(poipmf, /*tr*/("Poissonian Probability Mass Function"));
    FUNCTION_NAME(poivar, /*tr*/("Poissonian Distribution Variance"));
    FUNCTION_NAME(polar, /*tr*/("Convert to Polar Notation"));
    FUNCTION_NAME(product, /*tr*/("Product"));
    FUNCTION_NAME(radians, /*tr*/("Radians"));
    FUNCTION_NAME(real, /*tr*/("Real Part"));
    FUNCTION_NAME(round, /*tr*/("Rounding"));
    FUNCTION_NAME(sec, /*tr*/("Secant"));
    FUNCTION_NAME(shl, /*tr*/("Arithmetic Shift Left"));
    FUNCTION_NAME(shr, /*tr*/("Arithmetic Shift Right"));
    FUNCTION_NAME(sgn, /*tr*/("Signum"));
    FUNCTION_NAME(sin, /*tr*/("Sine"));
    FUNCTION_NAME(sinh, /*tr*/("Hyperbolic Sine"));
    FUNCTION_NAME(sqrt, /*tr*/("Square Root"));
    FUNCTION_NAME(stddev, /*tr*/("Standard Deviation (Square Root of Variance)"));
    FUNCTION_NAME(sum, /*tr*/("Sum"));
    FUNCTION_NAME(tan, /*tr*/("Tangent"));
    FUNCTION_NAME(tanh, /*tr*/("Hyperbolic Tangent"));
    FUNCTION_NAME(trunc, /*tr*/("Truncation"));
    FUNCTION_NAME(unmask, /*tr*/("Sign-extend a value"));
    FUNCTION_NAME(variance, /*tr*/("Variance"));
    FUNCTION_NAME(xor, /*tr*/("Logical XOR"));
    FUNCTION_NAME(yroot, /*tr*/("Y Root"));
}

void FunctionRepo::retranslateText()
{
    setFunctionNames();
    setTranslatableFunctionUsages();
}
