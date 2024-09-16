// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbutton.h"
#include "../utils/baseutils.h"
#include "../utils/configsettings.h"

TextButton::TextButton(int num, DWidget *parent)
    : DPushButton(parent)
{
    setObjectName("TextButton");
    m_fontsize = num;
    setText(QString("%1").arg(m_fontsize));

    setFixedSize(24, 26);
    setCheckable(true);

    connect(this, &TextButton::clicked, this, [ = ] {
        if (this->isChecked())
        {
            ConfigSettings::instance()->setValue("text", "fontsize", m_fontsize);
        }
    });
}

TextButton::~TextButton() {}
