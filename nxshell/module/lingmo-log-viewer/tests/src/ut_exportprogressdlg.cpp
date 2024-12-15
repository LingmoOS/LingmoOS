// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exportprogressdlg.h"

#include <QDebug>

#include <gtest/gtest.h>
#include <stub.h>

TEST(ExportProgressDlg_Constructor_UT, ExportProgressDlg_Constructor_UT)
{
    ExportProgressDlg *p = new ExportProgressDlg(nullptr);
    EXPECT_NE(p, nullptr);
    delete p;
}

class ExportProgressDlg_setProgressBarRange_UT_Param
{
public:
    ExportProgressDlg_setProgressBarRange_UT_Param(bool iIsProgressBarExists, int iMinValue, int iMaxValue)
    {
        m_isProgressBarExists = iIsProgressBarExists;
        m_minValue = iMinValue;
        m_maxValue = iMaxValue;
    }
    bool m_isProgressBarExists;
    int m_minValue;
    int m_maxValue;
};

class ExportProgressDlg_setProgressBarRange_UT : public ::testing::TestWithParam<ExportProgressDlg_setProgressBarRange_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(DisplayContent, ExportProgressDlg_setProgressBarRange_UT, ::testing::Values(ExportProgressDlg_setProgressBarRange_UT_Param(true, 0, 100), ExportProgressDlg_setProgressBarRange_UT_Param(false, 0, 100), ExportProgressDlg_setProgressBarRange_UT_Param(true, 100, 0)));

TEST_P(ExportProgressDlg_setProgressBarRange_UT, ExportProgressDlg_setProgressBarRange_UT_001)
{
    ExportProgressDlg *p = new ExportProgressDlg(nullptr);
    EXPECT_NE(p, nullptr);
    ExportProgressDlg_setProgressBarRange_UT_Param param = GetParam();
    qInfo() << "ExportProgressDlg_setProgressBarRange_UT" << param.m_isProgressBarExists << param.m_minValue << param.m_maxValue;
    if (!param.m_isProgressBarExists) {
        delete p->m_pExportProgressBar;
        p->m_pExportProgressBar = nullptr;
        p->setProgressBarRange(param.m_minValue, param.m_maxValue);
    } else {
        p->setProgressBarRange(param.m_minValue, param.m_maxValue);
        if (param.m_minValue < param.m_maxValue) {
            EXPECT_EQ(p->m_pExportProgressBar->minimum(), param.m_minValue) << "check the status after setProgressBarRange()_001";
            EXPECT_EQ(p->m_pExportProgressBar->maximum(), param.m_maxValue) << "check the status after setProgressBarRange()_001";
        }
    }

    p->deleteLater();
}

TEST_P(ExportProgressDlg_setProgressBarRange_UT, ExportProgressDlg_setProgressBarRange_UT_002)
{
    ExportProgressDlg *p = new ExportProgressDlg(nullptr);
    EXPECT_NE(p, nullptr);
    ExportProgressDlg_setProgressBarRange_UT_Param param = GetParam();
    qInfo() << "ExportProgressDlg_setProgressBarRange_UT" << param.m_isProgressBarExists << param.m_minValue << param.m_maxValue;
    if (!param.m_isProgressBarExists) {
        delete p->m_pExportProgressBar;
        p->m_pExportProgressBar = nullptr;
        p->setProgressBarRange(param.m_minValue, param.m_maxValue);
    } else {
        p->setProgressBarRange(param.m_minValue, param.m_maxValue);
        if (param.m_minValue >= param.m_maxValue) {
            EXPECT_NE(p->m_pExportProgressBar->minimum(), param.m_minValue) << "check the status after setProgressBarRange()_002";
            EXPECT_NE(p->m_pExportProgressBar->maximum(), param.m_maxValue) << "check the status after setProgressBarRange()_002";
        }
    }
    p->deleteLater();
}

class ExportProgressDlg_updateProgressBarValue_UT_Param
{
public:
    ExportProgressDlg_updateProgressBarValue_UT_Param(bool iIsProgressBarExists, int iValue)
    {
        m_isProgressBarExists = iIsProgressBarExists;
        m_Value = iValue;
    }
    bool m_isProgressBarExists;
    int m_Value;
};

class ExportProgressDlg_updateProgressBarValue_UT : public ::testing::TestWithParam<ExportProgressDlg_updateProgressBarValue_UT_Param>
{
};

INSTANTIATE_TEST_CASE_P(ExportProgressDlg, ExportProgressDlg_updateProgressBarValue_UT, ::testing::Values(ExportProgressDlg_updateProgressBarValue_UT_Param(true, 50), ExportProgressDlg_updateProgressBarValue_UT_Param(false, 50)));

TEST_P(ExportProgressDlg_updateProgressBarValue_UT, ExportProgressDlg_updateProgressBarValue_UT)
{
    ExportProgressDlg *p = new ExportProgressDlg(nullptr);
    EXPECT_NE(p, nullptr);
    ExportProgressDlg_updateProgressBarValue_UT_Param param = GetParam();
    if (!param.m_isProgressBarExists) {
        delete p->m_pExportProgressBar;
        p->m_pExportProgressBar = nullptr;
        p->updateProgressBarValue(param.m_Value);
    } else {
        p->updateProgressBarValue(param.m_Value);
        EXPECT_EQ(p->m_pExportProgressBar->value(), param.m_Value);
    }
    p->deleteLater();
}

TEST(ExportProgressDlg_closeEvent_UT, ExportProgressDlg_closeEvent_UT)
{
    ExportProgressDlg *p = new ExportProgressDlg(nullptr);
    EXPECT_NE(p, nullptr);

    p->closeEvent(nullptr);
    EXPECT_EQ(p->m_pExportProgressBar->value(), p->m_pExportProgressBar->minimum());
    p->deleteLater();
}
