// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AnimationLabel.h"

AnimationLabel::AnimationLabel(QWidget *parent)
    : DLabel(parent)
    , m_CurrentIndex(-1)
{

}

void AnimationLabel::setImageList(const QStringList &iconList)
{
    // 设置图片列表
    m_ImageList << iconList;
}

void AnimationLabel::setcurrentIcon(int iconIndex)
{
    // 设置当前Icon
    QIcon icon(QIcon::fromTheme(m_ImageList[iconIndex]));
    QPixmap pix = icon.pixmap(128, 128);
    this->setPixmap(pix);
    m_CurrentIndex = iconIndex;
}

int AnimationLabel::currentIcon()
{
    return m_CurrentIndex;
}


