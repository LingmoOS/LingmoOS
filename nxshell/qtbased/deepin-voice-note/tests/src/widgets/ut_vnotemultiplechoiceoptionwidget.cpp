// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnotemultiplechoiceoptionwidget.h"
#include "vnotemultiplechoiceoptionwidget.h"

UT_VnoteMultipleChoiceOptionWidget::UT_VnoteMultipleChoiceOptionWidget()
{
}

void UT_VnoteMultipleChoiceOptionWidget::SetUp()
{
    m_widget = new VnoteMultipleChoiceOptionWidget;
}

void UT_VnoteMultipleChoiceOptionWidget::TearDown()
{
    delete m_widget;
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_changeFromThemeType_001)
{
    m_widget->changeFromThemeType();
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_buttonPressed_001)
{
    m_widget->buttonPressed(m_widget->Move);
    EXPECT_FALSE(m_widget->m_moveButton->icon().isNull());
    m_widget->buttonPressed(m_widget->SaveAsTxT);
    EXPECT_FALSE(m_widget->m_saveTextButton->icon().isNull());
    m_widget->buttonPressed(m_widget->SaveAsVoice);
    EXPECT_FALSE(m_widget->m_saveVoiceButton->icon().isNull());
    m_widget->buttonPressed(m_widget->Delete);
    EXPECT_FALSE(m_widget->m_deleteButton->icon().isNull());
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_setSvgColor_001)
{
    QString path(":/icons/deepin/builtin/light/detail_notes_move.svg");
    QString color = QColor(Qt::red).name();
    EXPECT_FALSE(m_widget->setSvgColor(path, color).isNull());
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_trigger_001)
{
    int id = 2;
    m_widget->trigger(id);
    EXPECT_EQ(m_widget->m_moveButton->testAttribute(Qt::WA_UnderMouse), false);
    EXPECT_EQ(m_widget->m_saveTextButton->testAttribute(Qt::WA_UnderMouse), false);
    EXPECT_EQ(m_widget->m_saveVoiceButton->testAttribute(Qt::WA_UnderMouse), false);
    EXPECT_EQ(m_widget->m_deleteButton->testAttribute(Qt::WA_UnderMouse), false);
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_enableButtons_001)
{
    bool txtButtonStatus = false;
    bool voiceButtonStatus = true;
    m_widget->enableButtons(txtButtonStatus, voiceButtonStatus);
    EXPECT_EQ(m_widget->m_saveTextButton->isEnabled(), txtButtonStatus);
    EXPECT_EQ(m_widget->m_saveVoiceButton->isEnabled(), voiceButtonStatus);
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_setNoteNumber_001)
{
    int noteNumber = 2;
    m_widget->m_noteNumber = 1;
    m_widget->setNoteNumber(noteNumber);
    EXPECT_EQ(m_widget->m_noteNumber, noteNumber);
}

TEST_F(UT_VnoteMultipleChoiceOptionWidget, UT_VnoteMultipleChoiceOptionWidget_initConnections_001)
{
    m_widget->initConnections();
}
