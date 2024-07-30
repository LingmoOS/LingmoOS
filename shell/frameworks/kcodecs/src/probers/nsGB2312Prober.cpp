/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

// for S-JIS encoding, observe characteristic:
// 1, kana character (or hankaku?) often have high frequency of appearance
// 2, kana character often exist in group
// 3, certain combination of kana is never used in japanese language

#include "nsGB2312Prober.h"

namespace kencodingprober
{
void nsGB18030Prober::Reset(void)
{
    mCodingSM->Reset();
    mState = eDetecting;
    mDistributionAnalyser.Reset();
    // mContextAnalyser.Reset();
}

nsProbingState nsGB18030Prober::HandleData(const char *aBuf, unsigned int aLen)
{
    if (aLen == 0) {
        return mState;
    }

    for (unsigned int i = 0; i < aLen; i++) {
        const nsSMState codingState = mCodingSM->NextState(aBuf[i]);
        if (codingState == eError) {
            mState = eNotMe;
            break;
        }
        if (codingState == eItsMe) {
            mState = eFoundIt;
            break;
        }
        if (codingState == eStart) {
            unsigned int charLen = mCodingSM->GetCurrentCharLen();

            if (i == 0) {
                mLastChar[1] = aBuf[0];
                mDistributionAnalyser.HandleOneChar(mLastChar, charLen);
            } else {
                mDistributionAnalyser.HandleOneChar(aBuf + i - 1, charLen);
            }
        }
    }

    mLastChar[0] = aBuf[aLen - 1];

    if (mState == eDetecting) {
        if (mDistributionAnalyser.GotEnoughData() && GetConfidence() > SHORTCUT_THRESHOLD) {
            mState = eFoundIt;
        }
    }
    //    else
    //      mDistributionAnalyser.HandleData(aBuf, aLen);

    return mState;
}

float nsGB18030Prober::GetConfidence(void)
{
    float distribCf = mDistributionAnalyser.GetConfidence();

    return (float)distribCf;
}
}
