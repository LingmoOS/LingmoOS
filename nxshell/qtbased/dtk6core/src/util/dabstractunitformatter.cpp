// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dabstractunitformatter.h"

DCORE_BEGIN_NAMESPACE

/*!
  @~english
  @class Dtk::Core::DAbstractUnitFormatter
  @ingroup dutil
  @brief DAbstractUnitFormatter is a interface which manages the data with the
  same type

  DAbstractUnitFormatter provides interfaces including: <br>
    1. The maximum value of one unit.
    2. The minumum value of one unit.
    3. The convert rate to next unit.
    4. The string to display one unit.
 */

/*!
  @~english
  @fn int DAbstractUnitFormatter::unitMax() const = 0
  @brief Get the maximum unit in the list
 */
/*!
  @~english
  @fn int DAbstractUnitFormatter::unitMin() const = 0
  @brief Get the minumum unit in the list
 */
/*!
  @~english
  @fn uint DAbstractUnitFormatter::unitConvertRate(int unitId) const = 0
  @brief Get the convert rate to next unit of unitId
  @param[in] unitId the id of one unit
 */
/*!
  @~english
  @fn qreal DAbstractUnitFormatter::unitValueMax(int unitId) const
  @brief Get the maximum value of unitId
  @param[in] unitId the id of one unit
 */
/*!
  @~english
  @fn qreal DAbstractUnitFormatter::unitValueMin(int unitId) const
  @brief Get the minumum value of unitId
  @param[in] unitId the id of one unit
 */
/*!
  @~english
  @fn QString DAbstractUnitFormatter::unitStr(int unitId) const = 0
  @brief Get the display string of unitId
  @param[in] unitId the id of one unit
 */

/*!
  @~english
  @brief Constructor of DAbstractUnitFormatter

 */
DAbstractUnitFormatter::DAbstractUnitFormatter() {}

/*!
  @~english
  @brief Destructor of DAbstractUnitFormatter

 */
DAbstractUnitFormatter::~DAbstractUnitFormatter() {}

/*!
  @~english
  @brief Convert value from unit currentUnit to targetUnit. If currentUnit is
  smaller than targetUnit, the value will zoom out until the unit is equal to
  targetUnit. Instead it will zoom in.

  @param[in] value primitive value
  @param[in] currentUnit current unit of value
  @param[in] targetUnit target unit to convert to
  @return qreal the value with unit targetUnit
 */
qreal DAbstractUnitFormatter::formatAs(qreal value, int currentUnit,
                                       const int targetUnit) const {
  while (currentUnit < targetUnit)
    value /= unitConvertRate(currentUnit++);
  while (currentUnit > targetUnit)
    value *= unitConvertRate(--currentUnit);

  return value;
}

/*!
  @~english
  @brief Convert the value to a proper unit.

  If unit is larger than unitMin() or smaller than unitMax, value will be
  converted to the unit where value is close to the unitValueMin()

  @param[in] value primitive value
  @param[in] unit current unit
  @return QPair<qreal, int> a pair of the converted value and unit
 */
QPair<qreal, int> DAbstractUnitFormatter::format(const qreal value,
                                                 const int unit) const {
  // can convert to smaller unit
  if (unit > unitMin() && value < unitValueMin(unit))
    return format(value * unitConvertRate(unit - 1), unit - 1);

  // can convert to bigger unit
  if (unit < unitMax() && value > unitValueMax(unit))
    return format(value / unitConvertRate(unit), unit + 1);

  return QPair<qreal, int>(value, unit);
}

/*!
  @~english
  @brief A version of format() with all the convert data

  @param[in] value primitive value
  @param[in] unit current unit
  @return QList<QPair<qreal, int>> pairs of all converted value and unit
 */
QList<QPair<qreal, int>>
DAbstractUnitFormatter::formatAsUnitList(const qreal value, int unit) const {
  if (qFuzzyIsNull(value))
    return QList<QPair<qreal, int>>();

  if (value < unitValueMin(unit) || unit == unitMin()) {
    if (unit != unitMin())
      return formatAsUnitList(value * unitConvertRate(unit - 1), unit - 1);
    else
      return std::move(QList<QPair<qreal, int>>()
                       << QPair<qreal, int>(value, unit));
  }

  ulong _value = ulong(value);
  QList<QPair<qreal, int>> ret = formatAsUnitList(value - _value, unit);

  while (_value && unit != unitMax()) {
    const ulong rate = unitConvertRate(unit);
    const ulong r = _value % rate;
    if (r)
      ret.push_front(QPair<qreal, int>(r, unit));

    unit += 1;
    _value /= rate;
  }

  if (_value)
    ret.push_front(QPair<qreal, int>(_value, unit));

  return ret;
}

DCORE_END_NAMESPACE
