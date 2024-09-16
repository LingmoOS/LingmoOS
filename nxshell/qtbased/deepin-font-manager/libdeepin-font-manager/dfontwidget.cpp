// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontwidget.h"

const int FIXED_WIDTH = 1280;
const int FIXED_HEIGHT = 740;

/*************************************************************************
 <Function>      DFontWidget
 <Description>   构造函数-构造一个字体预览类对象
 <Author>
 <Input>
    <param1>     parent          Description:父对象
 <Return>        DFontWidget     Description:返回一个字体预览类对象
 <Note>          null
*************************************************************************/
DFontWidget::DFontWidget(QWidget *parent)
    : QWidget(parent),
      m_layout(new QStackedLayout(this)),
      m_preview(new DFontPreview(this)),
      m_thread(new DFontLoadThread(this)),
      m_spinner(new DSpinner(this)),
      m_errMsg(new QLabel(this))
{
    QLocale locale;
    QString translationFile = QString("/usr/share/deepin-font-manager/translations/deepin-font-manager_%1.qm").arg(locale.name());
    m_translator.load(translationFile);
    qApp->installTranslator(&m_translator);
    QWidget *spinnerPage = new QWidget;
    QVBoxLayout *spinnerLayout = new QVBoxLayout(spinnerPage);
    m_spinner->setFixedSize(50, 50);
    spinnerLayout->addWidget(m_spinner, 0, Qt::AlignCenter);
    spinnerLayout->addWidget(m_errMsg, 0, Qt::AlignCenter);

    m_layout->addWidget(spinnerPage);

    /*增加滚动条功能 UT000539*/
    m_area = new DScrollArea(this);
    m_area->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_area->setWidgetResizable(true);
    m_area->setWidget(m_preview);

    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0x00, 0xff, 0x00, 0x00));
    m_area->setPalette(pal);
    setPalette(pal);

    m_area->setFrameShape(QFrame::Shape::NoFrame);

    m_layout->addWidget(m_area);

    connect(m_thread, &DFontLoadThread::loadFinished, this, &DFontWidget::handleFinished);

    connect(qApp, &DApplication::fontChanged, [this]() {
        m_errMsg->setFont(DApplication::font());
    });
    m_area->setFixedSize(FIXED_WIDTH, FIXED_HEIGHT);
//    m_area->setFixedSize(static_cast<int>(qApp->primaryScreen()->geometry().width() / 1.5),
//                         static_cast<int>(qApp->primaryScreen()->geometry().height() / 1.5 + 20));
}

/*************************************************************************
 <Function>      ~DFontWidget
 <Description>   析构函数-析构DFontWidget类对象
 <Author>
 <Input>
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontWidget::~DFontWidget()
{
    qApp->removeTranslator(&m_translator);
}

/*************************************************************************
 <Function>      setFileUrl
 <Description>   传入字体路径
 <Author>
 <Input>
    <param1>     url             Description:字体路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontWidget::setFileUrl(const QString &url)
{
    m_filePath = url;

    m_layout->setCurrentIndex(0);
    m_spinner->start();

    m_preview->fontDatabase.removeAllApplicationFonts();
    m_thread->quit();
    m_thread->open(url);
    m_thread->start();
}

QSize DFontWidget::sizeHint() const
{
    return QSize(FIXED_WIDTH, FIXED_HEIGHT);
}

/*************************************************************************
 <Function>      handleFinished
 <Description>   显示预览结果
 <Author>
 <Input>
    <param1>     data            Description:输入字节流
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontWidget::handleFinished(const QByteArray &data)
{
    /*UT000539 字体损坏弹出提示隐藏view*/
    if (m_preview->fontDatabase.addApplicationFontFromData(data) == -1) {
        m_spinner->stop();
        m_spinner->hide();
        m_preview->hide();
        QString content = DApplication::translate("DFontWidget", "Broken file");
        m_errMsg->setText(content);
        m_errMsg->show();
        return;
    }
    if (m_errMsg->isVisible())
        m_errMsg->hide();
    m_preview->setFileUrl(m_filePath);
    m_layout->setCurrentIndex(1);
    m_spinner->stop();
    m_preview->show();
    m_area->horizontalScrollBar()->setSliderPosition(0);
}
