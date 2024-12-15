// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeflodarea.h"
#include "leftareaoftextedit.h"


CodeFlodArea::CodeFlodArea(LeftAreaTextEdit *leftAreaWidget)
{
    m_pLeftAreaWidget = leftAreaWidget;
}

CodeFlodArea::~CodeFlodArea()
{

}

void CodeFlodArea::paintEvent(QPaintEvent *e)
{
    m_pLeftAreaWidget->codeFlodAreaPaintEvent(e);
}

