// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SecurityDialog.h"

#include <DFontSizeManager>
#include <DLabel>
#include <DApplicationHelper>

#include <QTextOption>
#include <QPainter>

NewStr autoCutText(const QString &text, DLabel *pDesLbl)
{

    if (text.isEmpty() || nullptr == pDesLbl) {
        return NewStr();
    }

    QFont font; // 应用使用字体对象
    QFontMetrics font_label(font);
    QString strText = text;
    int titlewidth = font_label.width(strText);
    QString str;
    NewStr newstr;
    int width = pDesLbl->width();
    if (titlewidth < width) {
        newstr.strList.append(strText);
        newstr.resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (font_label.width(str) > width) { //根据label宽度调整每行字符数
                str.remove(str.count() - 1, 1);
                newstr.strList.append(str);
                newstr.resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        newstr.strList.append(str);
        newstr.resultStr += str;
    }
    newstr.fontHeifht = font_label.height();
    return newstr;
}

SecurityDialog::SecurityDialog(const QString &urlstr, QWidget *parent)
    : DDialog(parent)
{
    setFixedWidth(380);
    setMinimumHeight(180);
    setIcon(QIcon::fromTheme("deepin-reader"));
    QString str1(tr("This document is trying to connect to:"));
    QString str2(urlstr + QLatin1String(" ") + tr("If you trust the website, click Allow, otherwise click Block."));
    m_strDesText = str2;

    addButton(tr("Block", "button"));
    addButton(tr("Allow", "button"), true, ButtonRecommend);

    // 标题
    NameLabel = new DLabel(this);
    NameLabel->setFixedWidth(340);
    NameLabel->setAlignment(Qt::AlignCenter);
    NameLabel->setText(str1);
    NameLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(NameLabel, DFontSizeManager::T6, QFont::Medium);
    setLabelColor(NameLabel, 1.0);

    // 提示
    ContentLabel = new DLabel(this);
    ContentLabel->setFixedWidth(340);
    ContentLabel->setAlignment(Qt::AlignCenter);
    ContentLabel->setText(str2);
    DFontSizeManager::instance()->bind(ContentLabel, DFontSizeManager::T6, QFont::Normal);
    setLabelColor(ContentLabel, 0.7);

    addContent(NameLabel, Qt::AlignHCenter);
    addContent(ContentLabel, Qt::AlignHCenter);

    autoFeed();
}

void SecurityDialog::autoFeed()
{
    if (nullptr == NameLabel || nullptr == ContentLabel) {
        return;
    }

    NewStr newstr = autoCutText(m_strDesText, ContentLabel);
    ContentLabel->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    ContentLabel->setFixedHeight(height_lable);

    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        m_iDialogOldHeight = height();
        m_iLabelOld1Height = NameLabel->height();
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight - m_iLabelOld1Height
                       + height_lable + NameLabel->height()); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
}

void SecurityDialog::setLabelColor(DLabel *label, qreal alphaF)
{
    if (nullptr == label) {
        return;
    }

    // 根据UI要求使用对应的颜色并设置透明度
    DPalette pamessageDetail = DApplicationHelper::instance()->palette(label);
    QColor pamessageDetailColor = pamessageDetail.color(DPalette::Active, DPalette::BrightText);
    pamessageDetailColor.setAlphaF(alphaF);
    pamessageDetail.setColor(DPalette::Active, DPalette::WindowText, pamessageDetailColor);
    DApplicationHelper::instance()->setPalette(label, pamessageDetail);
}

void SecurityDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        autoFeed();
    } else if (QEvent::ThemeChange == event->type()) { // 根据主题变化重新设置颜色
        setLabelColor(NameLabel, 1.0);
        setLabelColor(ContentLabel, 0.7);
    }

    DDialog::changeEvent(event);
}
