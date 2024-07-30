/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#include "CharDistribution.h"

#include "tables/Big5Freq.tab"
#include "tables/EUCKRFreq.tab"
#include "tables/GB2312Freq.tab"
#include "tables/JISFreq.tab"

#define SURE_YES 0.99f
#define SURE_NO 0.01f

namespace kencodingprober
{
// return confidence base on received data
float CharDistributionAnalysis::GetConfidence()
{
    // if we didn't receive any character in our consideration range, return negative answer
    if (mTotalChars == 0) {
        return SURE_NO;
    }

    if (mTotalChars != mFreqChars) {
        float r = mFreqChars / ((mTotalChars - mFreqChars) * mTypicalDistributionRatio);

        if (r < SURE_YES) {
            return r;
        }
    }
    // normalize confidence, (we don't want to be 100% sure)
    return SURE_YES;
}

EUCKRDistributionAnalysis::EUCKRDistributionAnalysis()
{
    mCharToFreqOrder = EUCKRCharToFreqOrder;
    mTableSize = EUCKR_TABLE_SIZE;
    mTypicalDistributionRatio = EUCKR_TYPICAL_DISTRIBUTION_RATIO;
}

GB2312DistributionAnalysis::GB2312DistributionAnalysis()
{
    mCharToFreqOrder = GB2312CharToFreqOrder;
    mTableSize = GB2312_TABLE_SIZE;
    mTypicalDistributionRatio = GB2312_TYPICAL_DISTRIBUTION_RATIO;
}

Big5DistributionAnalysis::Big5DistributionAnalysis()
{
    mCharToFreqOrder = Big5CharToFreqOrder;
    mTableSize = BIG5_TABLE_SIZE;
    mTypicalDistributionRatio = BIG5_TYPICAL_DISTRIBUTION_RATIO;
}

SJISDistributionAnalysis::SJISDistributionAnalysis()
{
    mCharToFreqOrder = JISCharToFreqOrder;
    mTableSize = JIS_TABLE_SIZE;
    mTypicalDistributionRatio = JIS_TYPICAL_DISTRIBUTION_RATIO;
}

EUCJPDistributionAnalysis::EUCJPDistributionAnalysis()
{
    mCharToFreqOrder = JISCharToFreqOrder;
    mTableSize = JIS_TABLE_SIZE;
    mTypicalDistributionRatio = JIS_TYPICAL_DISTRIBUTION_RATIO;
}
}
