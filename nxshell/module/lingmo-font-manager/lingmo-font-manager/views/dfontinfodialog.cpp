// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontinfodialog.h"
#include "dfontpreviewitemdef.h"
#include "utils.h"
#include "fonticontext.h"
#include "dfontinfoscrollarea.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>

#include <QScrollArea>
#include <QKeyEvent>
#include <QBitmap>
#include <QFileInfo>

#define NAME_TITLE_WIDTH 300

/*************************************************************************
 <Function>      DFontInfoDialog
 <Description>   信息页面构造函数
 <Author>
 <Input>
    <param1>     fontInfo        Description:当前字体信息结构体
    <param2>     parent          Description:父控件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontInfoDialog::DFontInfoDialog(DFontPreviewItemData *fontInfo, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_fontInfo(fontInfo)
    , m_fontinfoArea(nullptr)
    , m_scrollArea(nullptr)
{
    connect(m_signalManager, &SignalManager::sizeChange, this, &DFontInfoDialog::autoHeight);
    initUI();
    initConnections();
    this->move(parent->geometry().center() - this->rect().center());
}

/*************************************************************************
 <Function>      AutoFeed
 <Description>   用于信息页面nametitle的宽度和字符串长度判断，
 <Author>        UT000539
 <Input>
    <param1>     text            Description:待处理文本
 <Return>        QString         Description:返回处理过的字符串
 <Note>          null
*************************************************************************/
QString DFontInfoDialog::AutoFeed(QString text)
{
    QFont m_CurrentFont = this->font();
    QFontMetrics fm(m_CurrentFont);
    int n_TextSize = fm.width(text);
    int count  = 0;
    if (n_TextSize > NAME_TITLE_WIDTH) {
        int n_position = 0;
        long n_curSumWidth = 0;
        for (int i = 0; i < text.size(); i++) {
            n_curSumWidth += fm.width(text.at(i));
            if (n_curSumWidth >= NAME_TITLE_WIDTH * (count + 1)) {
                n_position = i;
                text.insert(n_position, "\n");
                count++;
            }
            if (count == 2) {
                QString thirdLine = text.right(text.size() - n_position);
                text.remove(thirdLine);
                thirdLine = adaptiveLengthForNameTitle(fm, thirdLine, NAME_TITLE_WIDTH);
                text.append(thirdLine);
                break;
            }
        }
    }
    return text;
}

/*************************************************************************
 <Function>      adaptiveLengthForNameTitle
 <Description>   用于nametitle的第三行判断，由AutoFeed调用
 <Author>        UT000539
 <Input>
    <param1>     fm            Description:传入当前字体属性信息
    <param2>     thirdLineText Description:第三行待处理的文本
    <param3>     lineWidth     Description:用于判断的行宽参数
 <Return>        QString       Description:返回处理过的第三行字符串
 <Note>          null
*************************************************************************/
QString DFontInfoDialog::adaptiveLengthForNameTitle(QFontMetrics fm, QString thirdLineText, int lineWidth)
{
    if (fm.width(thirdLineText) > lineWidth) {
        QString s = thirdLineText.right(6);
        int width = 0;
        for (int i = 0; i < thirdLineText.size(); i++) {
            width += fm.width(thirdLineText.at(i));
            if (width > (lineWidth - (fm.width(s) + fm.width("...")))) {
                thirdLineText.remove(i, thirdLineText.size());
                thirdLineText.append("...").append(s);
                break;
            }
        }
    } return thirdLineText;
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化信息页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    setLogoVisable(false);

    //setWindowOpacity(0.5); //Debug

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->setContentsMargins(10, 0, 0, 10);
    mainLayout->setSpacing(0);

    m_mainFrame = new QWidget(this);
    //m_mainFrame->setFrameShape(DFrame::Shape::NoFrame);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Font logo
    m_fontLogo = new FontIconText("://font-info-logo.svg", this);
    QFileInfo fileInfo(m_fontInfo->fontInfo.filePath);
    QString suffix = fileInfo.suffix().toLower();
    if (!suffix.compare("ttf")) {
        m_fontLogo->setContent(true);
    } else {
        m_fontLogo->setContent(false);
    }
    DFontSizeManager::instance()->bind(m_fontLogo, DFontSizeManager::T4);
    m_fontLogo->setFontName(m_fontInfo->fontInfo.familyName, m_fontInfo->fontInfo.styleName);

    m_fontFileName = new DLabel(this);
    m_fontFileName->setFixedWidth(280);
    m_fontFileName->adjustSize();
    m_fontFileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fontFileName->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_fontFileName->setMinimumHeight(m_fontFileName->fontMetrics().height());
//    m_fontFileName->setWordWrap(true);

    DFontSizeManager::instance()->bind(m_fontFileName, DFontSizeManager::T8);
    m_FileName = QFileInfo(m_fontInfo->fontInfo.filePath).fileName();
    QString text = AutoFeed(m_FileName);
    m_fontFileName->setText(text);

    // Set color
    DPalette pa = DApplicationHelper::instance()->palette(m_fontFileName);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::ToolTipText));
    m_fontFileName->setPalette(pa);

    m_scrollArea = new QScrollArea(this);
//    scrollArea->setLineWidth(120);
    m_scrollArea->setFixedSize(QSize(290, 375));
//    scrollArea->setContentsMargins(0,0,30,0);

    QPixmap bmp(QSize(280, 375));
    bmp.fill();
    QPainter p(&bmp);
    bmp.setDevicePixelRatio(0);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(), 12, 12);
    p.setRenderHint(QPainter::Antialiasing);

    m_scrollArea->viewport()->setMask(bmp);
    m_scrollArea->setFrameShape(QFrame::Shape::NoFrame);

    m_fontinfoArea = new dfontinfoscrollarea(m_fontInfo, this);
    m_fontinfoArea->setFixedWidth(290);
    m_scrollArea->setWidget(m_fontinfoArea);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    // Add childs to main layout
    mainLayout->addWidget(m_fontLogo, 0, Qt::AlignHCenter);
//    mainLayout->addSpacing(6);
    mainLayout->addWidget(m_fontFileName);
//    mainLayout->addSpacing(42);
    mainLayout->addStretch(8);
    mainLayout->addWidget(m_scrollArea);
    m_mainFrame->setLayout(mainLayout);
    addContent(m_mainFrame);

    if (DApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette paFrame = DApplicationHelper::instance()->palette(m_scrollArea->viewport());
        QColor colorFrame = paFrame.textLively().color();
        colorFrame.setAlphaF(0.05);
        paFrame.setColor(DPalette::Base, colorFrame);
        DApplicationHelper::instance()->setPalette(m_scrollArea->viewport(), paFrame);

    } else if (DApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette paFrame = DApplicationHelper::instance()->palette(m_scrollArea->viewport());
        QColor colorFrame = paFrame.textLively().color();
        colorFrame.setAlphaF(0.70);
        paFrame.setColor(DPalette::Base, colorFrame);
        DApplicationHelper::instance()->setPalette(m_scrollArea->viewport(), paFrame);
    }
    m_fontinfoArea->autoHeight();
    m_scrollArea->setFocus(Qt::MouseFocusReason);
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化信号与槽连接处理函数-主题变化信号和槽
 <Author>        UT000539
 <Input>         Null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoDialog::initConnections()
{
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ] {
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

        if (DApplicationHelper::DarkType == themeType)
        {
            DPalette paFrame = DApplicationHelper::instance()->palette(m_scrollArea->viewport());
            QColor colorFrame = paFrame.textLively().color();
            colorFrame.setAlphaF(0.05);
            paFrame.setColor(DPalette::Base, colorFrame);
            DApplicationHelper::instance()->setPalette(m_scrollArea->viewport(), paFrame);
            //通过m_fontFileName->setPalette(pa)未实现更新字体颜色，在此根据主题变化同步更新字体名称标签的颜色属性
            DApplicationHelper::instance()->setPalette(m_fontFileName, paFrame);

        } else if (DApplicationHelper::LightType == themeType)
        {
            DPalette paFrame = DApplicationHelper::instance()->palette(m_scrollArea->viewport());
            //使用构造函数中有关设置，会导致颜色重叠的异常的情况，故在此使用固定颜色，与textLively中#FFFFFF同色号
            QColor colorFrame(255, 255, 255);
            colorFrame.setAlphaF(0.70);
            paFrame.setColor(DPalette::Base, colorFrame);
            DApplicationHelper::instance()->setPalette(m_scrollArea->viewport(), paFrame);
            //同时更新字体名称标签的颜色属性,跟随主题，原因同上
            DApplicationHelper::instance()->setPalette(m_fontFileName, paFrame);
        }
    });
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   重新实现键盘press事件-信息页面打开快捷键关闭本窗口
 <Author>
 <Input>
    <param1>     ev              Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoDialog::keyPressEvent(QKeyEvent *ev)
{
    QDialog::keyPressEvent(ev);
    if (QApplication::keyboardModifiers()
            == Qt::ControlModifier && ev->key() == Qt::Key_I) {
        this->close();
        deleteLater();
    }
}

/*************************************************************************
 <Function>      autoHeight
 <Description>   自适应变化信息详情页面高度
 <Author>        UT000442
 <Input>
    <param1>     height          Description:用于判断信息详情页面高度的参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoDialog::autoHeight(int height)
{
    //repaint m_fontFileName/*UT000539*/
    m_fontFileName->setText(AutoFeed(m_FileName));

    if (height * 1.1 + 280 < DEFAULT_WINDOW_H) {
        this->setFixedHeight(static_cast<int>(height * 1.1 + 280));
        QPixmap bmp(QSize(280, (static_cast<int>(height * 1.1 + 10))));
        bmp.fill();
        QPainter p(&bmp);
        bmp.setDevicePixelRatio(0);
        p.setBrush(Qt::black);
        p.drawRoundedRect(bmp.rect(), 12, 12);
        p.setRenderHint(QPainter::Antialiasing);
        m_scrollArea->viewport()->setMask(bmp);
        m_scrollArea->viewport()->setFixedHeight(static_cast<int>(height * 1.1 + 10));
        m_scrollArea->setFixedHeight(static_cast<int>(height * 1.1 + 10));
    } else {
        this->setFixedHeight(DEFAULT_WINDOW_H);
        QPixmap bmp(QSize(280, 375));
        bmp.fill();
        QPainter p(&bmp);
        bmp.setDevicePixelRatio(0);
        p.setBrush(Qt::black);
        p.drawRoundedRect(bmp.rect(), 12, 12);
        p.setRenderHint(QPainter::Antialiasing);
        m_scrollArea->viewport()->setMask(bmp);
        m_scrollArea->viewport()->setFixedHeight(375);
        m_scrollArea->setFixedHeight(375);
    }
}
