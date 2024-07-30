/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <cmath>

#include "timeestimator.h"

using namespace Baloo;

TimeEstimator::TimeEstimator() = default;

uint TimeEstimator::calculateTimeLeft(int filesLeft)
{
    if (!m_estimateReady) {
        return 0;
    }

    float totalTime = 0;
    float totalWeight = 0;

    int bufferIndex = m_bufferIndex;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        float weight = sqrt(i + 1);
        totalWeight += weight;

        totalTime += m_batchTimeBuffer[bufferIndex] * weight;
        bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
    }

    float weightedAverage = totalTime / totalWeight;

    return weightedAverage * filesLeft;
}

void TimeEstimator::handleNewBatchTime(uint time, uint batchSize)
{
    // add the current batch time in place of the oldest batch time
    m_batchTimeBuffer[m_bufferIndex] = (float)time / batchSize;

    m_bufferIndex = (m_bufferIndex + 1) % BUFFER_SIZE;

    if (!m_estimateReady && m_bufferIndex == 0) {
        // Buffer has been filled once. We are ready to estimate
        m_estimateReady = true;
    }
}
