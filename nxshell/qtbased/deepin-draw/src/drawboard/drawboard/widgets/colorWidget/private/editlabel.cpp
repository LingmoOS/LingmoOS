// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editlabel.h"

#include <QHBoxLayout>
#include <QFontMetrics>
#include <QDebug>

#include "utils/global.h"

const QSize LINEEDIT_SIZE = QSize(55, 36);

EditLabel::EditLabel(QWidget *parent)
    : DLineEdit(parent)
{
    this->setFixedSize(LINEEDIT_SIZE);
    this->setClearButtonEnabled(false);
#ifdef USE_DTK
    this->lineEdit()->setReadOnly(true);
#else
    setReadOnly(true);
#endif

    QFont font = this->font();
    font.setPixelSize(14);
    this->setFont(font);
}


EditLabel::~EditLabel() {}
