// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontpreview.h"

#ifdef DFM_BASE
DFMBASE_BEGIN_NAMESPACE
#else
DFM_BEGIN_NAMESPACE
#endif

#ifdef DFM_BASE
FontPreview::FontPreview(QObject *parent):
    AbstractBasePreview(parent),
#else
FontPreview::FontPreview(QObject *parent):
    DFMFilePreview(parent),
#endif
    m_previewWidget(new DFontWidget)
{
}

FontPreview::~FontPreview()
{
//    https://pms.uniontech.com/bug-view-142781.html
//    插件中不需要释放。释放会导致文管异常退出。
//    if (m_previewWidget)
//        m_previewWidget->deleteLater();
}

/*************************************************************************
 <Function>      setFileUrl
 <Description>   设置url路径
 <Author>        null
 <Input>
    <param1>     url             Description:需要设置的路径
 <Return>        bool                Description:是否设置成功
 <Note>          null
*************************************************************************/
#ifdef DFM_BASE
bool FontPreview::setFileUrl(const QUrl &url)
#else
bool FontPreview::setFileUrl(const DUrl &url)
#endif
{
    if (m_url == url)
        return true;

    if (!url.isLocalFile())
        return false;

    m_url = url;
    qDebug() << __FUNCTION__ << m_url << "m_url";
    m_previewWidget->setFileUrl(url.toLocalFile());
    m_title = url.toString();
    qDebug() << __FUNCTION__ << m_title << url.toLocalFile() << "m_title";

    if (!m_title.split("/").isEmpty()) {
        m_title = m_title.split("/").last();
        qDebug() << __FUNCTION__ << m_title;
    }

    Q_EMIT titleChanged();

    return true;
}

/*************************************************************************
 <Function>      fileUrl
 <Description>   获取文件的url路径
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        DUrl            Description:返回字体文件的url路径
 <Note>          null
*************************************************************************/
#ifdef DFM_BASE
QUrl FontPreview::fileUrl() const
#else
DUrl FontPreview::fileUrl() const
#endif
{
    return m_url;
}

/*************************************************************************
 <Function>      contentWidget
 <Description>   获取预览控件
 <Author>        null
 <Input>
    <param1>     null                 Description:null
 <Return>        QWidget *            Description:预览控件的指针
 <Note>          null
*************************************************************************/
QWidget *FontPreview::contentWidget() const
{
    return m_previewWidget;
}

/*************************************************************************
 <Function>      title
 <Description>   获取标题label显示内容
 <Author>        null
 <Input>
    <param1>     null               Description:null
 <Return>        QString            Description:label显示内容
 <Note>          null
*************************************************************************/
QString FontPreview::title() const
{
    return m_title;
}

#ifdef DFM_BASE
DFMBASE_END_NAMESPACE
#else
DFM_END_NAMESPACE
#endif

