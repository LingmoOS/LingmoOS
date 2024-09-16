// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UT_STUB_H
#define UT_STUB_H

#include "darchivemanager.h"
#include "gtest/gtest.h"
#include "libzipplugin.h"
#include "uitools.h"
#include "pluginmanager.h"
#include "commonstruct.h"
#include "singlejob.h"
#include "batchjob.h"
#include <stub.h>

DCOMPRESSOR_USE_NAMESPACE

ReadOnlyArchiveInterface *g_UiTools_createInterface_result = nullptr;
bool g_batchExtractJob_setArchiveFiles_result = false;
ReadOnlyArchiveInterface *uiTools_createInterface_stub(const QString &, bool, UiTools::AssignPluginType)
{
    return g_UiTools_createInterface_result;
}

void job_start_stub()
{
    return;
}

bool batchExtractJob_setArchiveFiles_stub(const QStringList &)
{
    return g_batchExtractJob_setArchiveFiles_result;
}

bool stub_batchExtractJob_setArchiveFiles(Stub &stub, bool bOk)
{
    g_batchExtractJob_setArchiveFiles_result = bOk;
    stub.set(ADDR(BatchExtractJob, setArchiveFiles), batchExtractJob_setArchiveFiles_stub);
}

void stub_UiTools_createInterface(Stub &stub, ReadOnlyArchiveInterface *pInterface)
{
    g_UiTools_createInterface_result = pInterface;
    stub.set((ReadOnlyArchiveInterface * (*)(const QString &, bool bWrite, UiTools::AssignPluginType)) ADDR(UiTools, createInterface), uiTools_createInterface_stub);
}

void stub_SingleJob_start(Stub &stub)
{
    typedef void (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::start);
    stub.set(A_foo, job_start_stub);
}

void stub_StepExtractJob_start(Stub &stub)
{
    typedef void (*fptr)(StepExtractJob *);
    fptr A_foo = (fptr)(&StepExtractJob::start);
    stub.set(A_foo, job_start_stub);
}

void stub_UpdateJob_start(Stub &stub)
{
    typedef void (*fptr)(UpdateJob *);
    fptr A_foo = (fptr)(&UpdateJob::start);
    stub.set(A_foo, job_start_stub);
}

void stub_ConvertJob_start(Stub &stub)
{
    typedef void (*fptr)(ConvertJob *);
    fptr A_foo = (fptr)(&ConvertJob::start);
    stub.set(A_foo, job_start_stub);
}

#endif
