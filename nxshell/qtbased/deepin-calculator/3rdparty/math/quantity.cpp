// quantity.cpp
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

#include "quantity.h"
#include "../core/settings.h"

#include "rational.h"
#include "units.h"

#include <QStringList>

#define RATIONAL_TOL HNumber("1e-20")

#define ENSURE_DIMENSIONLESS(x) \
    if (!(x).isDimensionless()) \
        return DMath::nan(InvalidDimension);

#define ENSURE_SAME_DIMENSION(x, y) \
    if ((!(x).sameDimension(y))) \
        return DMath::nan(DimensionMismatch);

Quantity operator-(const Quantity &q)
{
    Quantity res(q);
    res.m_numericValue = -res.m_numericValue;
    return res;
}

Quantity operator-(const Quantity &a, const Quantity &b)
{
    Quantity res(a);
    if (!a.sameDimension(b))
        return DMath::nan(DimensionMismatch);
    res.m_numericValue -= b.m_numericValue;
    return res;
}

bool operator>(const Quantity &l, const Quantity &r)
{
    if (l.sameDimension(r))
        return l.m_numericValue > r.m_numericValue;
    return false;
}

bool operator<(const Quantity &l, const Quantity &r)
{
    if (l.sameDimension(r))
        return l.m_numericValue < r.m_numericValue;
    return false;
}

bool operator>=(const Quantity &l, const Quantity &r)
{
    if (l.sameDimension(r))
        return l.m_numericValue >= r.m_numericValue;
    return false;
}

bool operator<=(const Quantity &l, const Quantity &r)
{
    if (l.sameDimension(r))
        return l.m_numericValue <= r.m_numericValue;
    return false;
}

bool operator==(const Quantity &l, const Quantity &r)
{
    if (l.sameDimension(r))
        return l.m_numericValue == r.m_numericValue;
    return false;
}

// Returns TRUE upon dimension mismatch.
bool operator!=(const Quantity &l, const Quantity &r)
{
    if (l.sameDimension(r))
        return l.m_numericValue != r.m_numericValue;
    return true;
}

Quantity operator*(const HNumber &l, const Quantity &r)
{
    return r * l;
}

Quantity operator*(const CNumber &l, const Quantity &r)
{
    return r * l;
}

Quantity operator/(const HNumber &l, const Quantity &r)
{
    return Quantity(l) / r;
}

Quantity operator/(const CNumber &l, const Quantity &r)
{
    return Quantity(l) / r;
}

Quantity::Quantity()
    : m_numericValue(0)
    , m_unit(nullptr)
    , m_unitName("")
{
}

Quantity::Quantity(const Quantity &other)
    : m_numericValue(other.m_numericValue)
    , m_dimension(other.m_dimension)
    , m_unit(nullptr)
    , m_unitName(other.m_unitName)
    , m_format(other.m_format)
{
    if (other.hasUnit())
        this->m_unit = new CNumber(other.unit());
    cleanDimension();
}

Quantity::Quantity(int i)
    : Quantity(CNumber(i))
{
}

Quantity::Quantity(const QJsonObject &json)
    : Quantity()
{
    *this = deSerialize(json);
}

Quantity::Quantity(const HNumber &h)
    : Quantity(CNumber(h))
{
}

Quantity::Quantity(const CNumber &c)
    : Quantity()
{
    this->m_numericValue = c;
}

Quantity::~Quantity()
{
    delete m_unit;
}

bool Quantity::isNan() const
{
    return m_numericValue.isNan();
}

bool Quantity::isZero() const
{
    return m_numericValue.isZero();
}

bool Quantity::isReal() const
{
    return m_numericValue.isReal();
}

bool Quantity::isPositive() const
{
    return m_numericValue.isPositive();
}

bool Quantity::isNegative() const
{
    return m_numericValue.isNegative();
}

bool Quantity::isInteger() const
{
    return (!this->hasDimension() && !this->hasUnit()) && m_numericValue.isInteger();
}

bool Quantity::isEven() const
{
    return this->isInteger() && (m_numericValue % CNumber(2) == 0);
}

bool Quantity::hasUnit() const
{
    return this->m_unit != NULL;
}

CNumber Quantity::unit() const
{
    if (this->hasUnit())
        return CNumber(*(this->m_unit));
    return CNumber(1);
}

QString Quantity::unitName() const
{
    if (this->hasUnit())
        return m_unitName;
    return "";
}

CNumber Quantity::numericValue() const
{
    return m_numericValue;
}

Quantity &Quantity::setDisplayUnit(const CNumber unit, const QString &name)
{
    if (unit.isNan())
        *this = DMath::nan(InvalidDimension);
    else {
        stripUnits();
        m_unit = new CNumber(unit);
        m_unitName = name;
    }
    return *this;
}

Quantity &Quantity::setFormat(Format c)
{
    m_format = c;
    return *this;
}

void Quantity::stripUnits()
{
    delete m_unit;
    m_unit = nullptr;
    m_unitName = "";
}

bool Quantity::hasDimension() const
{
    return !this->m_dimension.empty();
}

/*
 * Unlike hasDimension(), this does a clean up first, i.e. it
 * checks for redundant exponents.
 */
bool Quantity::isDimensionless() const
{
    Quantity temp(*this);
    temp.cleanDimension();
    return m_dimension.empty();
}

QMap<QString, Rational> Quantity::getDimension() const
{
    Quantity temp(*this);
    temp.cleanDimension();
    return temp.m_dimension;
}

void Quantity::modifyDimension(const QString &key, const Rational &exponent)
{
    if (exponent.isZero())
        m_dimension.remove(key);
    else
        m_dimension.insert(key, exponent);
}

void Quantity::copyDimension(const Quantity &other)
{
    clearDimension();
    this->m_dimension = other.m_dimension;
}

void Quantity::clearDimension()
{
    this->m_dimension.clear();
}

// Note: Does NOT clean the dimension vector first.
// The calling function must do so on its own.
bool Quantity::sameDimension(const Quantity &other) const
{
    return this->m_dimension == other.m_dimension;
}

void Quantity::cleanDimension()
{
    auto i = m_dimension.begin();
    while (i != m_dimension.end()) {
        if (i.value().isZero())
            i = m_dimension.erase(i);
        else
            ++i;
    }
}

void Quantity::serialize(QJsonObject &json) const
{
    QJsonObject nom_json;
    m_numericValue.serialize(nom_json);
    json["numeric_value"] = nom_json;

    if (hasDimension()) {
        QJsonObject dim_json;
        auto i = m_dimension.constBegin();
        while (i != m_dimension.constEnd()) {
            const auto &exp = i.value();
            const auto &name = i.key();
            dim_json[name] = exp.toString();
            ++i;
        }
        json["dimension"] = dim_json;
    }

    if (hasUnit()) {
        QJsonObject unit_json;
        m_unit->serialize(unit_json);
        json["unit"] = unit_json;
        json["unit_name"] = m_unitName;
    }

    if (!m_format.isNull()) {
        QJsonObject format_json;
        m_format.serialize(format_json);
        json["format"] = format_json;
    }
}

Quantity Quantity::deSerialize(const QJsonObject &json)
{
    Quantity result;
    if (json.contains("numeric_value")) {
        QJsonObject num_json = json["numeric_value"].toObject();
        result.m_numericValue = CNumber(num_json);
    }
    result.stripUnits();
    if (json.contains("unit")) {
        QJsonObject unit_json = json["unit"].toObject();
        result.m_unit = new CNumber(unit_json);
    }
    if (json.contains("unit_name"))
        result.m_unitName = json["unit_name"].toString();

    if (json.contains("dimension")) {
        QJsonObject dim_json = json["dimension"].toObject();
        for (int i = 0; i < dim_json.count(); ++i) {
            auto key = dim_json.keys().at(i);
            Rational val(dim_json[key].toString());
            result.modifyDimension(key, val);
        }
    }
    if (json.contains("format")) {
        QJsonObject format_json = json["format"].toObject();
        result.m_format = Quantity::Format::deSerialize(format_json);
    }
    return result;
}

Error Quantity::error() const
{
    return m_numericValue.error();
}

Quantity &Quantity::operator=(const Quantity &other)
{
    m_numericValue = other.m_numericValue;
    m_dimension = other.m_dimension;
    m_format = other.m_format;
    stripUnits();
    if (other.hasUnit()) {
        m_unit = new CNumber(*other.m_unit);
        m_unitName = other.m_unitName;
    }
    cleanDimension();
    return *this;
}

Quantity Quantity::operator+(const Quantity &other) const
{
    if (!this->sameDimension(other))
        return DMath::nan(DimensionMismatch);
    Quantity result(*this);
    result.m_numericValue += other.m_numericValue;
    return result;
}

Quantity &Quantity::operator+=(const Quantity &other)
{
    if (!this->sameDimension(other))
        *this = DMath::nan(DimensionMismatch);
    else
        this->m_numericValue += other.m_numericValue;
    return *this;
}

Quantity &Quantity::operator-=(const Quantity &other)
{
    return operator=(*this - other);
}

Quantity Quantity::operator*(const Quantity &other) const
{
    Quantity result(*this);
    result.m_numericValue *= other.m_numericValue;
    if (!other.isDimensionless()) {
        result.stripUnits();
        auto i = other.m_dimension.constBegin();
        while (i != other.m_dimension.constEnd()) {
            const auto &exp = i.value();
            const auto &name = i.key();
            if (!result.m_dimension.contains(name))
                result.m_dimension[name] = Rational(0);
            result.m_dimension[name] += exp;
            ++i;
        }
        result.cleanDimension();
    }
    return result;
}

Quantity Quantity::operator*(const CNumber &other) const
{
    Quantity result(*this);
    result.m_numericValue *= other;
    return result;
}

Quantity Quantity::operator*(const HNumber &other) const
{
    return operator*(CNumber(other));
}

Quantity &Quantity::operator*=(const Quantity &other)
{
    return operator=(*this * other);
}

Quantity Quantity::operator/(const Quantity &other) const
{
    Quantity result(*this);
    result.m_numericValue /= other.m_numericValue;
    if (!other.isDimensionless()) {
        result.stripUnits();
        auto i = other.m_dimension.constBegin();
        while (i != other.m_dimension.constEnd()) {
            const auto &exp = i.value();
            const auto &name = i.key();
            if (!result.m_dimension.contains(name))
                result.m_dimension[name] = Rational(0);
            result.m_dimension[name] -= exp;
            ++i;
        }
        result.cleanDimension();
    }
    return result;
}

Quantity Quantity::operator/(const HNumber &other) const
{
    return operator/(CNumber(other));
}

Quantity Quantity::operator/(const CNumber &other) const
{
    Quantity result(*this);
    result.m_numericValue /= other;
    result.cleanDimension();
    return result;
}

Quantity &Quantity::operator/=(const Quantity &other)
{
    return operator=(*this / other);
}

Quantity Quantity::operator%(const Quantity &other) const
{
    Quantity result(*this);
    result.m_numericValue = result.m_numericValue % other.m_numericValue;
    return result;
}

Quantity Quantity::operator&(const Quantity &other) const
{
    ENSURE_DIMENSIONLESS(*this);
    ENSURE_DIMENSIONLESS(other);
    Quantity result(*this);
    result.m_numericValue &= other.m_numericValue;
    return result;
}

Quantity &Quantity::operator&=(const Quantity &other)
{
    return operator=(*this & other);
}

Quantity Quantity::operator|(const Quantity &other) const
{
    ENSURE_DIMENSIONLESS(*this);
    ENSURE_DIMENSIONLESS(other);
    Quantity result(*this);
    result.m_numericValue |= other.m_numericValue;
    return result;
}

Quantity &Quantity::operator|=(const Quantity &other)
{
    return operator=(*this | other);
}

Quantity Quantity::operator^(const Quantity &other) const
{
    ENSURE_DIMENSIONLESS(*this);
    ENSURE_DIMENSIONLESS(other);
    Quantity result(*this);
    result.m_numericValue ^= other.m_numericValue;
    return result;
}

Quantity &Quantity::operator^=(const Quantity &other)
{
    return operator=(*this ^ other);
}

Quantity Quantity::operator~() const
{
    ENSURE_DIMENSIONLESS(*this);
    Quantity result(*this);
    result.m_numericValue = ~result.m_numericValue;
    return result;
}

Quantity Quantity::operator>>(const Quantity &other) const
{
    ENSURE_DIMENSIONLESS(*this);
    ENSURE_DIMENSIONLESS(other);
    Quantity result(*this);
    result.m_numericValue = result.m_numericValue >> other.m_numericValue;
    return result;
}

Quantity Quantity::operator<<(const Quantity &other) const
{
    ENSURE_DIMENSIONLESS(*this);
    ENSURE_DIMENSIONLESS(other);
    Quantity result(*this);
    result.m_numericValue = result.m_numericValue << other.m_numericValue;
    return result;
}

Quantity::Format::Format()
    : CNumber::Format()
{
}

Quantity::Format::Format(const CNumber::Format &other)
    : CNumber::Format(other)
{
}

Quantity::Format::Format(const HNumber::Format &other)
    : CNumber::Format(other)
{
}

Quantity::Format Quantity::Format::operator+(const Quantity::Format &other) const
{
    return Quantity::Format(CNumber::Format::operator+(static_cast<const CNumber::Format &>(other)));
}

void Quantity::Format::serialize(QJsonObject &json) const
{
    switch (mode) {
    case Mode::General:
        json["mode"] = QStringLiteral("General");
        break;
    case Mode::Fixed:
        json["mode"] = QStringLiteral("Fixed");
        break;
    case Mode::Scientific:
        json["mode"] = QStringLiteral("Scientific");
        break;
    case Mode::Engineering:
        json["mode"] = QStringLiteral("Engineering");
        break;
    case Mode::Complement:
        json["mode"] = QStringLiteral("Complement");
        break;
    case Mode::Null:
        break;
    }

    switch (base) {
    case Base::Binary:
        json["base"] = QStringLiteral("Binary");
        break;
    case Base::Octal:
        json["base"] = QStringLiteral("Octal");
        break;
    case Base::Hexadecimal:
        json["base"] = QStringLiteral("Hexadecimal");
        break;
    case Base::Decimal:
        json["base"] = QStringLiteral("Decimal");
        break;
    case Base::Null:
        break;
    }

    switch (notation) {
    case Notation::Cartesian:
        json["form"] = QStringLiteral("Cartesian");
        break;
    case Notation::Polar:
        json["form"] = QStringLiteral("Polar");
        break;
    case Notation::Null:
    default:
        break;
    }

    if (precision != PrecisionNull)
        json["precision"] = precision;
}

Quantity::Format Quantity::Format::deSerialize(const QJsonObject &json)
{
    Format result;
    if (json.contains("mode")) {
        auto strMode = json["mode"].toString();
        if (strMode == "General")
            result.mode = Mode::General;
        else if (strMode == "Fixed")
            result.mode = Mode::Fixed;
        else if (strMode == "Scientific")
            result.mode = Mode::Scientific;
        else if (strMode == "Engineering")
            result.mode = Mode::Engineering;
        else
            result.mode = Mode::Null;
    } else
        result.mode = Mode::Null;

    if (json.contains("base")) {
        auto strBase = json["base"].toString();
        if (strBase == "Binary")
            result.base = Base::Binary;
        else if (strBase == "Octal")
            result.base = Base::Octal;
        else if (strBase == "Decimal")
            result.base = Base::Decimal;
        else if (strBase == "Hexadecimal")
            result.base = Base::Hexadecimal;
        else
            result.base = Base::Null;
    } else
        result.base = Base::Null;

    if (json.contains("form")) {
        auto strNotation = json["form"].toString();
        if (strNotation == "Cartesian")
            result.notation = Notation::Cartesian;
        else if (strNotation == "Polar")
            result.notation = Notation::Polar;
        else
            result.notation = Notation::Null;
    } else
        result.notation = Notation::Null;


    result.precision = json.contains("precision") ? json["precision"].toInt() : PrecisionNull;
    return result;
}

bool Quantity::Format::isNull() const
{
    return (mode == Mode::Null && base == Base::Null && precision == PrecisionNull && notation == Notation::Null);
}

// DMath
// =====

bool DMath::complexMode = Settings::instance()->complexNumbers; // 关闭复数处理

#define COMPLEX_WRAP_1(fct, arg) \
    (DMath::complexMode ? CMath::fct(arg) : CNumber(HMath::fct(arg.real)))

#define COMPLEX_WRAP_2(fct, arg1, arg2) \
    (DMath::complexMode ? CMath::fct(arg1, arg2) : CNumber(HMath::fct(arg1.real, arg2.real)))

#define COMPLEX_WRAP_3(fct, arg1, arg2, arg3) \
    (DMath::complexMode ? CMath::fct(arg1, arg2, arg3) : CNumber(HMath::fct(arg1.real, arg2.real, arg3.real)))

#define COMPLEX_WRAP_4(fct, arg1, arg2, arg3, arg4) \
    (DMath::complexMode ? CMath::fct(arg1, arg2, arg3, arg4) \
     : CNumber(HMath::fct(arg1.real, arg2.real, arg3.real, arg4.real)))

//  Wrappers for functions that are only defined for dimensionless arguments

// Mo argument.
#define WRAPPER_DMATH_0(fct) \
    Quantity DMath::fct() \
    { \
        return Quantity(CMath::fct()); \
    } \

// One argument.
#define WRAPPER_DMATH_1(fct) \
    Quantity DMath::fct(const Quantity& arg1) \
    { \
        ENSURE_DIMENSIONLESS(arg1); \
        return Quantity(COMPLEX_WRAP_1(fct, arg1.m_numericValue)); \
    }

// Two arguments.
#define WRAPPER_DMATH_2(fct) \
    Quantity DMath::fct(const Quantity& arg1, const Quantity& arg2) \
    { \
        ENSURE_DIMENSIONLESS(arg1); \
        ENSURE_DIMENSIONLESS(arg2); \
        return Quantity(COMPLEX_WRAP_2(fct, arg1.m_numericValue, arg2.m_numericValue)); \
    }

// Three arguments.
#define WRAPPER_DMATH_3(fct) \
    Quantity DMath::fct(const Quantity& arg1, const Quantity& arg2, const Quantity& arg3) \
    { \
        ENSURE_DIMENSIONLESS(arg1); \
        ENSURE_DIMENSIONLESS(arg2); \
        ENSURE_DIMENSIONLESS(arg3); \
        return Quantity(COMPLEX_WRAP_3(fct, arg1.m_numericValue, arg2.m_numericValue, arg3.m_numericValue)); \
    }

// Four arguments.
#define WRAPPER_DMATH_4(fct) \
    Quantity DMath::fct(const Quantity& arg1, const Quantity& arg2, const Quantity& arg3, const Quantity& arg4) \
    { \
        ENSURE_DIMENSIONLESS(arg1); \
        ENSURE_DIMENSIONLESS(arg2); \
        ENSURE_DIMENSIONLESS(arg3); \
        ENSURE_DIMENSIONLESS(arg4); \
        return Quantity(COMPLEX_WRAP_4(fct, arg1.m_numericValue, arg2.m_numericValue, arg3.m_numericValue, \
                                       arg4.m_numericValue)); \
    }

WRAPPER_DMATH_0(e)
WRAPPER_DMATH_0(pi)
WRAPPER_DMATH_0(phi)
WRAPPER_DMATH_0(i)

Quantity DMath::nan(Error error)
{
    return Quantity(CMath::nan(error));
}

WRAPPER_DMATH_1(rad2deg)
WRAPPER_DMATH_1(deg2rad)
WRAPPER_DMATH_1(rad2gon)
WRAPPER_DMATH_1(gon2rad)
WRAPPER_DMATH_1(integer)
WRAPPER_DMATH_1(frac)
WRAPPER_DMATH_1(floor)
WRAPPER_DMATH_1(ceil)
WRAPPER_DMATH_1(exp)
WRAPPER_DMATH_1(ln)
WRAPPER_DMATH_1(lg)
WRAPPER_DMATH_1(lb)
WRAPPER_DMATH_2(log)
WRAPPER_DMATH_1(sinh)
WRAPPER_DMATH_1(cosh)
WRAPPER_DMATH_1(tanh)
WRAPPER_DMATH_1(arsinh)
WRAPPER_DMATH_1(arcosh)
WRAPPER_DMATH_1(artanh)
WRAPPER_DMATH_1(sin)
WRAPPER_DMATH_1(cos)
WRAPPER_DMATH_1(tan)
WRAPPER_DMATH_1(cot)
WRAPPER_DMATH_1(sec)
WRAPPER_DMATH_1(csc)
WRAPPER_DMATH_1(arcsin)
WRAPPER_DMATH_1(arccos)
WRAPPER_DMATH_1(arctan)
WRAPPER_DMATH_2(arctan2)

WRAPPER_DMATH_2(factorial)
WRAPPER_DMATH_1(gamma)
WRAPPER_DMATH_1(lnGamma)
WRAPPER_DMATH_1(erf)
WRAPPER_DMATH_1(erfc)

WRAPPER_DMATH_2(gcd)
WRAPPER_DMATH_2(idiv)

Quantity DMath::round(const Quantity &n, int prec)
{
    ENSURE_DIMENSIONLESS(n);
    return DMath::complexMode ?
           CMath::round(n.numericValue(), prec) :
           CNumber(HMath::round(n.numericValue().real, prec));
}

Quantity DMath::trunc(const Quantity &n, int prec)
{
    ENSURE_DIMENSIONLESS(n);
    return DMath::complexMode ?
           CMath::trunc(n.numericValue(), prec)
           : CNumber(HMath::trunc(n.numericValue().real, prec));
}

WRAPPER_DMATH_2(nCr)
WRAPPER_DMATH_2(nPr)
WRAPPER_DMATH_3(binomialPmf)
WRAPPER_DMATH_3(binomialCdf)
WRAPPER_DMATH_2(binomialMean)
WRAPPER_DMATH_2(binomialVariance)
WRAPPER_DMATH_4(hypergeometricPmf)
WRAPPER_DMATH_4(hypergeometricCdf)
WRAPPER_DMATH_3(hypergeometricMean)
WRAPPER_DMATH_3(hypergeometricVariance)
WRAPPER_DMATH_2(poissonPmf)
WRAPPER_DMATH_2(poissonCdf)
WRAPPER_DMATH_1(poissonMean)
WRAPPER_DMATH_1(poissonVariance)

WRAPPER_DMATH_2(mask)
WRAPPER_DMATH_2(sgnext)
WRAPPER_DMATH_2(ashr)
WRAPPER_DMATH_2(lshr)
WRAPPER_DMATH_2(rosh)
WRAPPER_DMATH_2(rcsh)


WRAPPER_DMATH_3(decodeIeee754)
WRAPPER_DMATH_4(decodeIeee754)


QString DMath::format(Quantity q, Quantity::Format format)
{
    format = q.format() + format;  // Left hand side oerator takes priority.

    // Handle units.
    if (!q.hasUnit() && !q.isDimensionless()) {
        q.cleanDimension();
        Units::findUnit(q);
    }
    QString unit_name = ' ' + q.unitName();
    CNumber unit = q.unit();
    CNumber number = q.m_numericValue;

    number /= unit;

    QString result = CMath::format(number, format);

    if (!number.real.isZero() && !number.imag.isZero() && unit_name != " ")
        result = "(" + result + ")";

    if (unit_name != " ")
        result.append(unit_name);

    return result;
}

Quantity DMath::real(const Quantity &x)
{
    Quantity result(x);
    result.m_numericValue = result.m_numericValue.real;
    return result;
}

Quantity DMath::imag(const Quantity &x)
{
    Quantity result(x);
    result.m_numericValue = result.m_numericValue.imag;
    return result;
}

Quantity DMath::conj(const Quantity &n)
{
    Quantity result = Quantity(n);
    // If in Real mode, just strip the imaginary part.
    result.m_numericValue = complexMode ?
                            CMath::conj(result.m_numericValue)
                            : CMath::real(result.m_numericValue);

    return result;
}

Quantity DMath::abs(const Quantity &n)
{
    Quantity result(n);
    result.m_numericValue = COMPLEX_WRAP_1(abs, n.m_numericValue);
    return result;
}

Quantity DMath::phase(const Quantity &n)
{
    return CMath::phase(n.numericValue());
}

Quantity DMath::sqrt(const Quantity &n)
{
    Quantity result(COMPLEX_WRAP_1(sqrt, n.m_numericValue));
    auto i = n.m_dimension.constBegin();
    while (i != n.m_dimension.constEnd()) {
        auto &exp = i.value();
        auto &name = i.key();
        result.modifyDimension(name, exp * Rational(1, 2));
        ++i;
    }
    return result;
}

Quantity DMath::cbrt(const Quantity &n)
{
    Quantity result(COMPLEX_WRAP_1(cbrt, n.m_numericValue));
    auto i = n.m_dimension.constBegin();
    while (i != n.m_dimension.constEnd()) {
        auto &exp = i.value();
        auto &name = i.key();
        result.modifyDimension(name, exp * Rational(1, 3));
        ++i;
    }
    return result;
}

Quantity DMath::raise(const Quantity &n1, int n)
{
    Quantity result;
    result.m_numericValue = complexMode ?
                            CMath::raise(n1.m_numericValue, n)
                            : CNumber(HMath::raise(n1.m_numericValue.real, n));
    auto i = n1.m_dimension.constBegin();
    while (i != n1.m_dimension.constEnd()) {
        auto &exp = i.value();
        auto &name = i.key();
        result.modifyDimension(name, exp * n);
        ++i;
    }
    return result;
}

Quantity DMath::raise(const Quantity &n1, const Quantity &n2)
{
    if (!n2.isDimensionless() || (!n1.isDimensionless() && !n2.isReal() && complexMode))
        return DMath::nan(InvalidDimension);

    // First get the new numeric value.
    Quantity result(COMPLEX_WRAP_2(raise, n1.m_numericValue, n2.m_numericValue));

    if (n1.isDimensionless())
        return result;

    // We can now assume that n1 has a dimension, but n2 is real.
    // Compute the new dimension: try to convert n2 to a Rational. If n2 is not
    // rational, return NaN.

    // For negative bases only allow odd denominators.
    Rational exponent(n2.m_numericValue.real);
    if (abs(exponent.toHNumber() - n2.m_numericValue.real) >= RATIONAL_TOL
            || (n1.isNegative() && exponent.denominator() % 2 == 0))
        return DMath::nan(OutOfDomain);

    // Compute new dimension.
    auto i = n1.m_dimension.constBegin();
    while (i != n1.m_dimension.constEnd()) {
        result.modifyDimension(i.key(), i.value()*exponent);
        ++i;
    }
    return result;
}

Quantity DMath::sgn(const Quantity &x)
{
    return Quantity(CMath::sgn(x.m_numericValue));
}

Quantity DMath::encodeIeee754(const Quantity &val, const Quantity &exp_bits, const Quantity &significand_bits)
{
    ENSURE_DIMENSIONLESS(val);
    ENSURE_DIMENSIONLESS(exp_bits);
    ENSURE_DIMENSIONLESS(significand_bits);

    Quantity result(CMath::encodeIeee754(val.numericValue(), exp_bits.numericValue(), significand_bits.numericValue()));
    result.m_format = result.m_format + Quantity::Format::Fixed() + Quantity::Format::Hexadecimal();
    return result;
}

Quantity DMath::encodeIeee754(const Quantity &val, const Quantity &exp_bits, const Quantity &significand_bits,
                              const Quantity &exp_bias)
{
    ENSURE_DIMENSIONLESS(val);
    ENSURE_DIMENSIONLESS(exp_bits);
    ENSURE_DIMENSIONLESS(significand_bits);
    ENSURE_DIMENSIONLESS(exp_bias);

    Quantity result(CMath::encodeIeee754(val.numericValue(), exp_bits.numericValue(), significand_bits.numericValue(),
                                         exp_bias.numericValue()));
    result.m_format = result.m_format + Quantity::Format::Fixed() + Quantity::Format::Hexadecimal();
    return result;
}
