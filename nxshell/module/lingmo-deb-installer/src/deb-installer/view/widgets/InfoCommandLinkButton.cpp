// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InfoCommandLinkButton.h"
#include "utils/utils.h"

#include <QKeyEvent>

InfoCommandLinkButton::InfoCommandLinkButton(QString text, QWidget *parent)
    : DCommandLinkButton(text, parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(this, fontFamily, 12, QFont::ExtraLight);  // 设置字体大小

    this->setFocusPolicy(Qt::TabFocus);  // 获取焦点
}

void InfoCommandLinkButton::keyPressEvent(QKeyEvent *event)
{
    // 添加回车，换行，空格键盘事件的响应
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) {
        if (this->hasFocus()) {  // 只有有焦点时才响应
            this->clicked();
        }
    }
}
