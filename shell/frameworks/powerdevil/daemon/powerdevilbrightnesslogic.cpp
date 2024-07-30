/*
 *   SPDX-FileCopyrightText: 2014 Nikita Skovoroda <chalkerx@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "powerdevilbrightnesslogic.h"

#include <QDebug>

namespace PowerDevil
{
BrightnessLogic::BrightnessLogic()
{
}

void BrightnessLogic::setValue(int value)
{
    m_current.value = value;
}

void BrightnessLogic::setValueRange(int valueMin, int valueMax)
{
    if (valueMax != m_current.valueMax || valueMin != m_current.valueMin) {
        m_current.valueMax = valueMax;
        m_current.valueMin = valueMin;
        m_current.steps = calculateSteps(valueMax);
    }
}

int BrightnessLogic::adjusted(StepAdjustmentAction adjustment) const
{
    switch (adjustment) {
    case Increase:
        return increased();
    case Decrease:
        return decreased();
    case IncreaseSmall:
        return increasedSmall();
    case DecreaseSmall:
        return decreasedSmall();
    }

    return -1; // We shouldn't get here
}

int BrightnessLogic::increased() const
{
    if (m_current.value == m_current.valueMax) {
        return m_current.valueMax; // we are at the maximum already
    }

    // Add 1 and round upwards to the nearest step
    int step = m_current.steps - (m_current.valueMax - m_current.value - 1) * m_current.steps / m_current.valueMax;

    if (m_current.valueMax > 100 && qRound(percentage(stepToValue(step))) <= qRound(percentage(m_current.value))) {
        // When no visible change was made, add 1 step.
        // This can happen only if valueMax > 100, else 1 >= 1%.
        step++;
    }

    return stepToValue(step);
}

int BrightnessLogic::increasedSmall() const
{
    const double newPercent = (std::round(m_current.value * 100 / double(m_current.valueMax)) + 1) / 100.0;
    return std::min<int>(m_current.valueMax, std::round(m_current.valueMax * newPercent));
}

int BrightnessLogic::decreased() const
{
    if (m_current.value == m_current.valueMin) {
        return m_current.valueMin; // we are at the minimum already
    }

    // Subtract 1 and round downwards to the nearest Step
    int step = (m_current.value - 1) * m_current.steps / m_current.valueMax;

    if (m_current.valueMax > 100 && qRound(percentage(stepToValue(step))) >= qRound(percentage(m_current.value))) {
        // When no visible change was made, subtract 1 step.
        // This can happen only if valueMax > 100, else 1 >= 1%.
        step--;
    }

    return stepToValue(step);
}

int BrightnessLogic::decreasedSmall() const
{
    const double newPercent = (std::round(m_current.value * 100 / double(m_current.valueMax)) - 1) / 100.0;
    return std::max<int>(m_current.valueMin, std::round(m_current.valueMax * newPercent));
}

int BrightnessLogic::steps() const
{
    return m_current.steps;
}

float BrightnessLogic::percentage(int value) const
{
    return value * 100.0 / m_current.valueMax;
}

const BrightnessLogic::BrightnessInfo BrightnessLogic::info() const
{
    return m_current;
}

int BrightnessLogic::stepToValue(int step) const
{
    return qBound(m_current.valueMin, qRound(step * 1.0 * m_current.valueMax / m_current.steps), m_current.valueMax);
}

}
