/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#include "nsSBCharSetProber.h"

#include <stdio.h>

namespace kencodingprober
{
nsProbingState nsSingleByteCharSetProber::HandleData(const char *aBuf, unsigned int aLen)
{
    for (unsigned int i = 0; i < aLen; i++) {
        const unsigned char order = mModel->charToOrderMap[(unsigned char)aBuf[i]];

        if (order < SYMBOL_CAT_ORDER) {
            mTotalChar++;
        }
        if (order < SAMPLE_SIZE) {
            mFreqChar++;

            if (mLastOrder < SAMPLE_SIZE) {
                mTotalSeqs++;
                if (!mReversed) {
                    ++(mSeqCounters[(int)mModel->precedenceMatrix[mLastOrder * SAMPLE_SIZE + order]]);
                } else { // reverse the order of the letters in the lookup
                    ++(mSeqCounters[(int)mModel->precedenceMatrix[order * SAMPLE_SIZE + mLastOrder]]);
                }
            }
        }
        mLastOrder = order;
    }

    if (mState == eDetecting) {
        if (mTotalSeqs > SB_ENOUGH_REL_THRESHOLD) {
            float cf = GetConfidence();
            if (cf > POSITIVE_SHORTCUT_THRESHOLD) {
                mState = eFoundIt;
            } else if (cf < NEGATIVE_SHORTCUT_THRESHOLD) {
                mState = eNotMe;
            }
        }
    }

    return mState;
}

void nsSingleByteCharSetProber::Reset(void)
{
    mState = eDetecting;
    mLastOrder = 255;
    for (unsigned int i = 0; i < NUMBER_OF_SEQ_CAT; i++) {
        mSeqCounters[i] = 0;
    }
    mTotalSeqs = 0;
    mTotalChar = 0;
    mFreqChar = 0;
}

//#define NEGATIVE_APPROACH 1

float nsSingleByteCharSetProber::GetConfidence(void)
{
#ifdef NEGATIVE_APPROACH
    if (mTotalSeqs > 0)
        if (mTotalSeqs > mSeqCounters[NEGATIVE_CAT] * 10) {
            return ((float)(mTotalSeqs - mSeqCounters[NEGATIVE_CAT] * 10)) / mTotalSeqs * mFreqChar / mTotalChar;
        }
    return (float)0.01;
#else // POSITIVE_APPROACH
    float r;

    if (mTotalSeqs > 0) {
        r = ((float)1.0) * mSeqCounters[POSITIVE_CAT] / mTotalSeqs / mModel->mTypicalPositiveRatio;
        r = r * mFreqChar / mTotalChar;
        if (r >= (float)1.00) {
            r = (float)0.99;
        }
        return r;
    }
    return (float)0.01;
#endif
}

const char *nsSingleByteCharSetProber::GetCharSetName()
{
    if (!mNameProber) {
        return mModel->charsetName;
    }
    return mNameProber->GetCharSetName();
}

#ifdef DEBUG_PROBE
void nsSingleByteCharSetProber::DumpStatus()
{
    printf("  SBCS: %1.3f [%s]\r\n", GetConfidence(), GetCharSetName());
}
#endif
}
