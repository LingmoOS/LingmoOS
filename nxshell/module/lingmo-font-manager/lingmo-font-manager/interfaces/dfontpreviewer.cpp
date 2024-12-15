// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewer.h"
#include "utils.h"

#include <DApplicationHelper>

#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

DFontPreviewer::DFontPreviewer(QWidget *parent)
    : QWidget(parent)
{
    InitData();
    InitConnections();
}

/*************************************************************************
 <Function>      InitData
 <Description>   初始化预览内容数据
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewer::InitData()
{
    m_previewTexts << "汉体书写信息技术标准相容"
                   << "档案下载使用界面简单"
                   << "支援服务升级资讯专业制作"
                   << "创意空间快速无线上网"
                   << "㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩"
                   << "AaBbCc ＡａＢｂＣｃ";
}

/*************************************************************************
 <Function>      InitConnections
 <Description>   初始化链接
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewer::InitConnections()
{
    connect(this, &DFontPreviewer::previewFontChanged,
            this, &DFontPreviewer::onPreviewFontChanged);
}


/*************************************************************************
 <Function>      onPreviewFontChanged
 <Description>   预览字体类型发生变化后的处理
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewer::onPreviewFontChanged()
{
    m_previewTexts.clear();
    InitData();
    foreach (auto it, m_previewTexts) {
        QString text = Utils::convertToPreviewString(m_fontPath, it);
        m_previewTexts.replaceInStrings(it, text);
    }
}

/*************************************************************************
 <Function>      setPreviewFontPath
 <Description>   设置当前预览字体的路径
 <Author>        null
 <Input>
    <param1>     font            Description:当前字体路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewer::setPreviewFontPath(const QString &font)
{
    if (m_fontPath != font) {
        m_fontPath = font;
        Q_EMIT previewFontChanged();
    }
}

/*************************************************************************
 <Function>      paintEvent
 <Description>   绘制预览字体
 <Author>        null
 <Input>
    <param1>     event            Description:绘制事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    //Save pen
    QPen oldPen = painter.pen();

    painter.setRenderHint(QPainter::Antialiasing);
    DPalette pa = DApplicationHelper::instance()->palette(this);
    painter.setBrush(QBrush(pa.color(DPalette::Base)));
    painter.setPen(Qt::transparent);

    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);

    QPainterPath painterPath;
    painterPath.addRoundedRect(rect, 8, 8);
    painter.drawPath(painterPath);

    int topSpace = 0;
    int fontHeight = painter.font().pixelSize();
    int textline = m_previewTexts.size();
    int textSpace = (event->rect().height() - textline * fontHeight - topSpace) / (textline + 1);
    int textHeight = fontHeight + textSpace;

    //Restore the pen
    painter.setPen(oldPen);

    QRect startRect(0, topSpace, event->rect().width(), textHeight);
    foreach (auto it, m_previewTexts) {
        painter.drawText(startRect, Qt::AlignCenter, it);

        startRect.setY(startRect.y() + textHeight);
        startRect.setHeight(textHeight);
    }
}
