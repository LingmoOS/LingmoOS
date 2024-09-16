// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEMULTIPLECHOICEOPTIONWIDGET_H
#define UT_VNOTEMULTIPLECHOICEOPTIONWIDGET_H

#include <QWidget>
#include "gtest/gtest.h"
#include <QObject>

class VnoteMultipleChoiceOptionWidget;
class UT_VnoteMultipleChoiceOptionWidget : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VnoteMultipleChoiceOptionWidget();
    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    VnoteMultipleChoiceOptionWidget *m_widget {nullptr};
};

#endif // UT_VNOTEMULTIPLECHOICEOPTIONWIDGET_H
