/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsCodingStateMachine_h__
#define nsCodingStateMachine_h__

#include "kencodingprober.h"

#include "kcodecs_export.h"

#include "nsPkgInt.h"
namespace kencodingprober
{
enum {
    eStart = 0,
    eError = 1,
    eItsMe = 2,
};
using nsSMState = int;

#define GETCLASS(c) GETFROMPCK(((unsigned char)(c)), mModel->classTable)

// state machine model
typedef struct {
    nsPkgInt classTable;
    unsigned int classFactor;
    nsPkgInt stateTable;
    const unsigned int *charLenTable;
    const char *name;
} SMModel;

class KCODECS_NO_EXPORT nsCodingStateMachine
{
public:
    nsCodingStateMachine(const SMModel *sm)
    {
        mCurrentState = eStart;
        mModel = sm;
    }
    nsSMState NextState(char c)
    {
        // for each byte we get its class KCODECS_NO_EXPORT , if it is first byte, we also get byte length
        unsigned int byteCls = GETCLASS(c);
        if (mCurrentState == eStart) {
            mCurrentBytePos = 0;
            mCurrentCharLen = mModel->charLenTable[byteCls];
        }
        // from byte's class KCODECS_NO_EXPORT and stateTable, we get its next state
        mCurrentState = GETFROMPCK(mCurrentState * (mModel->classFactor) + byteCls, mModel->stateTable);
        mCurrentBytePos++;
        return mCurrentState;
    }
    unsigned int GetCurrentCharLen(void)
    {
        return mCurrentCharLen;
    }
    void Reset(void)
    {
        mCurrentState = eStart;
    }
    const char *GetCodingStateMachine()
    {
        return mModel->name;
    }
#ifdef DEBUG_PROBE
    const char *DumpCurrentState()
    {
        switch (mCurrentState) {
        case eStart:
            return "eStart";
        case eError:
            return "eError";
        case eItsMe:
            return "eItsMe";
        default:
            return "OK";
        }
    }
#endif

protected:
    int mCurrentState;
    unsigned int mCurrentCharLen;
    unsigned int mCurrentBytePos;

    const SMModel *mModel;
};

extern KCODECS_NO_EXPORT const SMModel UTF8SMModel;
extern KCODECS_NO_EXPORT const SMModel Big5SMModel;
extern KCODECS_NO_EXPORT const SMModel EUCJPSMModel;
extern KCODECS_NO_EXPORT const SMModel EUCKRSMModel;
extern KCODECS_NO_EXPORT const SMModel GB18030SMModel;
extern KCODECS_NO_EXPORT const SMModel SJISSMModel;
extern KCODECS_NO_EXPORT const SMModel UCS2LESMModel;
extern KCODECS_NO_EXPORT const SMModel UCS2BESMModel;

extern KCODECS_NO_EXPORT const SMModel HZSMModel;
extern KCODECS_NO_EXPORT const SMModel ISO2022CNSMModel;
extern KCODECS_NO_EXPORT const SMModel ISO2022JPSMModel;
extern KCODECS_NO_EXPORT const SMModel ISO2022KRSMModel;
}
#endif /* nsCodingStateMachine_h__ */
