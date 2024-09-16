// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontinfoscrollarea.h"

#include <DFontSizeManager>
#include <DApplication>

#include <QTimer>
#include "freetype/ftsnames.h"
#include "freetype/ttnameid.h"
QString convertToUtf8(unsigned char *content, unsigned int len);

#define LAEBL_TEXT_WIDTH   165
#define TITLE_VISIBLE_WIDTH 90
#define INFO_VISIBLE_WIDTH 180
#define IS_NEED_ELLIPSIS 30 //是否需要省略号

/*************************************************************************
 <Function>      dfontinfoscrollarea
 <Description>   构造函数
 <Author>
 <Input>
    <param1>     pData           Description:当前字体信息结构体
    <param2>     parent          Description:父控件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
dfontinfoscrollarea::dfontinfoscrollarea(DFontPreviewItemData *pData,  DWidget *parent)
    : DFrame(parent)
    , m_fontInfo(pData)
{
    installEventFilter(this);

    setFrameShape(QFrame::NoFrame);
    initUi();
}

/*************************************************************************
 <Function>      initUi
 <Description>   初始化信息页面中详情页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void dfontinfoscrollarea::initUi()
{
    QStringList sFontList;
    sFontList << "Bitstream Charter-Regular" << "Bitstream Charter-Bold" << "Bitstream Charter-Italic"
              <<"Bitstream Charter-Bold Italic" << "Courier 10 Pitch-Italic" << "Courier 10 Pitch-Bold Italic"
              << "Courier 10 Pitch-Regular" << "Courier 10 Pitch-Bold";
    auto gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(0, 6, 0, 6);
    gridLayout->setSpacing(3);

    createLabel(gridLayout, 0, DApplication::translate("FontDetailDailog", "Style")
                , m_fontInfo->fontInfo.styleName);
    createLabel(gridLayout, 1, DApplication::translate("FontDetailDailog", "Type")
                , DApplication::translate("FontDetailDailog", m_fontInfo->fontInfo.type.toLatin1()));
    if (m_fontInfo->fontInfo.version.isEmpty()) {
        QString version = "Copyright 2014~2015 Adobe Syste-ms Incorporated (http://www.adob.com/), with Reserved Font Name cc Source.";
        createLabel(gridLayout, 2, DApplication::translate("FontDetailDailog", "Version"), version);
    } else {
        createLabel(gridLayout, 2, DApplication::translate("FontDetailDailog", "Version")
                    , m_fontInfo->fontInfo.version);
    }
    if (m_fontInfo->fontInfo.description.isEmpty()) {
        createLabel(gridLayout, 3, DApplication::translate("FontDetailDailog", "Description")
                    , DApplication::translate("FontDetailDailog", "Unknown"));
    } else {
        createLabel(gridLayout, 3, DApplication::translate("FontDetailDailog", "Description")
                    , m_fontInfo->fontInfo.description);
    }
    createLabel(gridLayout, 4, DApplication::translate("FontDetailDailog", "Full name"), m_fontInfo->fontInfo.fullname);
    createLabel(gridLayout, 5, DApplication::translate("FontDetailDailog", "Ps name"), m_fontInfo->fontInfo.psname);
    createLabel(gridLayout, 6, DApplication::translate("FontDetailDailog", "Trademark"), m_fontInfo->fontInfo.trademark);
    //获取 LICENSE
    FT_Library library = nullptr;
    FT_Init_FreeType(&library);
    FT_Face face = nullptr;
    FT_Error error = FT_New_Face(library,  m_fontInfo->fontInfo.filePath.toUtf8().constData(), 0, &face);
    QString sLicense = "";
    if (error == 0) {
         if (FT_IS_SFNT(face)) {
             FT_SfntName sname;
             const unsigned int count = FT_Get_Sfnt_Name_Count(face);
             for (unsigned int i = 0; i < count; ++i) {
                 if (FT_Get_Sfnt_Name(face, i, &sname) != 0) {
                     continue;
                 }

                 // only handle the unicode names for US langid.
                 if (sname.language_id == 0) {
                     continue;
                 }
                 bool bExitLoop = false;
                 switch (sname.name_id) {
                 // 0
                 case TT_NAME_ID_LICENSE: {
                        sLicense = convertToUtf8(sname.string, sname.string_len).simplified();
                        bExitLoop = true;
                        break;
                     }
                 default:
                     break;
                 }
                 if(bExitLoop) break;
            }
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    if(sFontList.contains(m_fontInfo->fontData.strFontName)) {
        QString version = "Copyright 1989-1992, Bitstream Inc., Cambridge, MA.";
        QString sLicenseDes = "You are hereby granted permission under all Bitstream propriety rights to use, copy, modify, sublicense, sell, and redistribute the 4 Bitstream Charter (r) \
Type 1 outline fonts and the 4 Courier Type 1 outline fonts for any purpose and without restriction; \
provided, that this notice is left intact on all copies of such fonts and that Bitstream's trademark is acknowledged as shown below on all unmodified copies of the 4 Charter Type 1 fonts. \
BITSTREAM CHARTER is a registered trademark of Bitstream Inc.";
        createLabel(gridLayout, 7, DApplication::translate("FontDetailDailog", "Copyright"), m_fontInfo->fontInfo.copyright.isEmpty()?version:m_fontInfo->fontInfo.copyright);
        createLabel(gridLayout, 8, DApplication::translate("FontDetailDailog", "License Description"), sLicense.isEmpty()?sLicenseDes:sLicense);
    } else {
        createLabel(gridLayout, 7, DApplication::translate("FontDetailDailog", "Copyright"), m_fontInfo->fontInfo.copyright.isEmpty()?DApplication::translate("FontDetailDailog", "Unknown"):m_fontInfo->fontInfo.copyright);
        createLabel(gridLayout, 8, DApplication::translate("FontDetailDailog", "License Description"), sLicense.isEmpty()?DApplication::translate("FontDetailDailog", "Unknown"):sLicense);
    }
    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(10, 10, 10, 10);
    basicLabel = new DLabel(DApplication::translate("FontDetailDailog", "Basic info"));
    DFontSizeManager::instance()->bind(basicLabel, DFontSizeManager::T6);

    vLayout->addWidget(basicLabel);
    vLayout->addItem(gridLayout);
    vLayout->addStretch(1);
    this->setLayout(vLayout);
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   事件过滤器-过滤到字体改变事件后更新详情页字体信息与页面高度
 <Author>
 <Input>
    <param1>     obj             Description:事件接受对象
    <param2>     e               Description:事件对象
 <Return>        bool            Description:返回值为是否已处理该事件
 <Note>          null
*************************************************************************/
bool dfontinfoscrollarea::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::FontChange) {
        updateText();
        QTimer::singleShot(0, [ = ] {
            autoHeight();
        });
    }
    return  DFrame::eventFilter(obj, e);
}

/*************************************************************************
 <Function>      createLabel
 <Description>   初始话信息页面详情中信息部分标签内容
 <Author>        UT000539
 <Input>
    <param1>     layout          Description:父布局
    <param2>     index           Description:行参数
    <param3>     objName         Description:标签逻辑名称
    <param4>     sData           Description:标签文本内容
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void dfontinfoscrollarea::createLabel(QGridLayout *layout, const int &index, const QString &objName, const QString &sData)
{
    QString str = objName;
    DLabel *label = new DLabel(elideText(str), this);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T8);
    label->setAlignment(Qt::AlignTop);

    label->setFixedWidth(100);
    layout->addWidget(label, index, 0);
    if (pTitleMap.find(label) == pTitleMap.end()) {
        pTitleMap.insert(std::pair<QLabel *, QString>(label, objName));
    }
    if (sData == "") {
        DLabel *labelText = new DLabel(this);
        DFontSizeManager::instance()->bind(labelText, DFontSizeManager::T8);
        labelText->setText(DApplication::translate("FontDetailDailog", "Unknown"));
        labelText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(labelText, index, 1);
    } else {
        DFrame *widgets = addTitleFrame(sData, objName);
        widgets->setFrameShape(QFrame::NoFrame);
        layout->addWidget(widgets, index, 1);
    }
}

/*************************************************************************
 <Function>      addTitleFrame
 <Description>   初始话信息页面详情中标题部分标签内容
 <Author>
 <Input>
    <param1>     sData            Description:标签文本内容
    <param2>     objName          Description:标签逻辑名称
 <Return>        DFrame           Description:返回一个信息页面布局
 <Note>          null
*************************************************************************/
DFrame *dfontinfoscrollarea::addTitleFrame(const QString &sData, const QString &objName)
{
    Q_UNUSED(objName);
    DFrame *m_textShowFrame = new DFrame(this);
    QString ts = elideText(sData, this->font(), INFO_VISIBLE_WIDTH);

    QVBoxLayout *vLayout = new QVBoxLayout;

    QLabel *label = new QLabel(ts, m_textShowFrame);
    label->setFixedWidth(INFO_VISIBLE_WIDTH);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T8);
    if (pLabelMap.find(label) == pLabelMap.end()) {
        pLabelMap.insert(std::pair<QLabel *, QString>(label, sData));
    }

    label->setAlignment(Qt::AlignLeft);
    label->adjustSize();

    vLayout->addWidget(label);

    vLayout->addStretch(1);
    vLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *textShowLayout = new QHBoxLayout;
    textShowLayout->setContentsMargins(0, 0, 0, 0);
    textShowLayout->setSpacing(0);
    textShowLayout->addLayout(vLayout);
    m_textShowFrame->setLayout(textShowLayout);
    vLayout->setContentsMargins(0, 0, 0, 0);

    textShowLayout->addStretch(1);

    m_textShowFrame->setContentsMargins(0, 0, 0, 0);

    return m_textShowFrame;
}

/*************************************************************************
 <Function>      elideText
 <Description>   用于信息页面title名称长度判断
 <Author>
 <Input>
    <param1>     titleName       Description:标题名称文本
 <Return>        QString         Description:返回处理过的文本信息
 <Note>          null
*************************************************************************/
QString dfontinfoscrollarea::elideText(QString &titleName) const
{
    QFont font = this->font();
    QFontMetrics fontMetric(font);
    QString finalTitle = "";
    QString m_curTitle = "";
    int curWidth = 0;

    for (auto str : titleName) {
        if (str == "\t") {
            curWidth  += fontMetric.width("a");
        } else {
            curWidth  += fontMetric.width(str);
        }
        m_curTitle += str;
        if (curWidth > TITLE_VISIBLE_WIDTH) {
            if (m_curTitle == titleName) {
                finalTitle = titleName;
                break;
            } else {
                if (fontMetric.width("...") > IS_NEED_ELLIPSIS) {
                    finalTitle = m_curTitle;
                } else {
                    finalTitle =   m_curTitle.append("...");
                }
                break;
            }
        } else {
            finalTitle = titleName;
        }
    }
    return finalTitle;
}

//用于信息页面详细信息部分换行判断/*539*/
/*************************************************************************
 <Function>      elideText
 <Description>   用于信息页面详情信息部分长度判断
 <Author>
 <Input>
    <param1>     text       Description:详情文本内容
    <param1>     font       Description:当前页面字体信息
    <param1>     nLabelSize Description:页面标签宽度
 <Return>        QString    Description:返回处理过的文本信息
 <Note>          null
*************************************************************************/
QString dfontinfoscrollarea::elideText(const QString &text, const QFont &font, int nLabelSize)
{
    QFontMetrics fm(font);
    QString strText = text;
    int n_TextSize = fm.width(strText);
    int count  = 0;
    if (n_TextSize > nLabelSize) {
        int n_position = 0;
        long n_curSumWidth = 0;
        for (int i = 0; i < strText.size(); i++) {
            n_curSumWidth += fm.width(strText.at(i));
            if (n_curSumWidth > nLabelSize * (count + 1)) {
                n_position = i;
                strText.insert(n_position, "\n");
                count++;
            }
        }
    }
    return strText;
}

/*************************************************************************
 <Function>      updateText
 <Description>   在字体变化后重绘信息页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void dfontinfoscrollarea::updateText()
{
    for (auto pTltle : pTitleMap) {
        if (!pTltle.first) {
            continue;
        }
        QString text = pTltle.second;
        QString newtext = elideText(text);
        pTltle.first->setText(newtext);
    }
    for (auto plabeliter : pLabelMap) {
        if (!plabeliter.first) {
            continue;
        }
        QString text = plabeliter.second;
        QString newtext = elideText(text, this->font(), INFO_VISIBLE_WIDTH);
        plabeliter.first->setText(newtext);
    }
}

/*************************************************************************
 <Function>      autoHeight
 <Description>   重新计算信息详情页面高度参数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void dfontinfoscrollarea::autoHeight()
{
    int totalHeight = 0;
    for (auto plabeliter : pLabelMap) {
        if (!plabeliter.first) {
            continue;
        }
        totalHeight = totalHeight + plabeliter.first->height();
    }
    emit m_signalManager->sizeChange(totalHeight + 76 + basicLabel->height());
}
