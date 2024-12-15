// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "seperatorline.h"
#include "pagecontext.h"

#ifdef USE_DTK
#include <DGuiApplicationHelper>
#endif

SeperatorLine::SeperatorLine(QWidget *parent)
    : DLabel(parent)
{
    setFixedSize(12, 24);

    updateTheme();

#ifdef USE_DTK
    //绑定主题发生变化的信号
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &SeperatorLine::updateTheme);
#endif

    setAlignment(Qt::AlignCenter);
}

SeperatorLine::~SeperatorLine()
{

}

void SeperatorLine::updateTheme()
{
#ifdef USE_DTK
    QPixmap seprtatorLine;
    if (DGuiApplicationHelper::instance()->themeType() == 1) {
        seprtatorLine = QPixmap(QString(":/theme/common/line.svg"));
    } else {
        seprtatorLine = QPixmap(QString(":/theme/common/linedark.svg"));
    }
    this->setPixmap(seprtatorLine);
#endif
}
