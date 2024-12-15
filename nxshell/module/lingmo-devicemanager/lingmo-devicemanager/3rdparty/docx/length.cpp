// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "length.h"

#include <math.h>

namespace Docx {
//

const int EMUS_PER_INCH = 914400;
const int EMUS_PER_CM = 360000;
const int EMUS_PER_MM = 36000;
const int EMUS_PER_PX = 12700;
const int EMUS_PER_TWIP = 635;

const int UNITS_PER_POINT = 100;

Length::Length(): m_value()
{

}

Length::Length(int emu) : m_value(emu)
{
    m_isEmpty = false;
}

float Length::cm() const
{
    return (float)m_value / (float)EMUS_PER_CM;
}

int Length::emu() const
{
    return m_value;
}

float Length::inches() const
{
    return (float)m_value / (float)EMUS_PER_INCH;
}

float Length::mm() const
{
    return (float)m_value / (float)EMUS_PER_MM;
}

int Length::px() const
{
    return (int)(round((float)m_value / (float)EMUS_PER_PX) + 0.1);
}

int Length::twips() const
{
    return (int)(round((float)m_value / (float)EMUS_PER_TWIP));
}

bool Length::isEmpty() const
{
    return m_isEmpty;
}

/*!
 * \brief Inches长度
 *
 * eg width = Inches(0.5)
 * \param inches
 * \return
 */
Length Inches::emus(float inches)
{
    int emu = int(inches * EMUS_PER_INCH);
    return Length(emu);
}

/*!
 * \brief centimetres 长度
 *
 * eg height = Cm(12)
 * \param inches
 * \return
 */
Length Cm::emus(float cm)
{
    int emu = int(cm * EMUS_PER_CM);
    return Length(emu);
}

/*!
 * \brief English Metric Units
 *
 * eg width = Emu(457200)
 * \param emu
 * \return
 */
Length Emu::emus(float emu)
{
    return Length(emu);
}

/*!
 * \brief millimeters 长度
 * \param cm
 * \return
 */
Length Mm::emus(float mm)
{
    int emu = int(mm * EMUS_PER_MM);
    return Length(emu);
}

/*!
 * \brief  setting font sizes in points
 * \param pts
 * \return
 */
Length Pt::emus(float pts)
{
    int units = int(pts * UNITS_PER_POINT);
    return Length(units);
}

/*!
 * \brief pixels
 * \param px
 * \return
 */
Length Px::emus(float px)
{
    int emu = int(px * EMUS_PER_PX);
    return Length(emu);
}

/*!
 * \brief Twips
 *
 *  eg width = Twips(42)
 *  A twip is a twentieth of a point, 635 EMU
 * \param px
 * \return
 */
Length Twips::emus(float twips)
{
    int emu = int(twips * EMUS_PER_TWIP);
    return Length(emu);
}
}

