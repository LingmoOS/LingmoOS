// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTWIDGET_H
#define DFONTWIDGET_H

#include "dspinner.h"
#include "dfontpreview.h"
#include "dfontloadthread.h"

#include <DScrollArea>
#include <DApplication>

#include <QWidget>
#include <QStackedLayout>
#include <QScrollBar>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

/*************************************************************************
 <Class>         DFontWidget
 <Description>   字体预览类
 <Author>
 <Note>          null
*************************************************************************/
class DFontWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DFontWidget(QWidget *parent = nullptr);
    ~DFontWidget() override;
    //传入字体路径
    void setFileUrl(const QString &url);
    QSize sizeHint() const override;
protected:

private:
    //显示预览结果
    void handleFinished(const QByteArray &data);

private:
    QStackedLayout *m_layout;
    DFontPreview *m_preview;
    DScrollArea *m_area;
    DFontLoadThread *m_thread;
    DSpinner *m_spinner;
    QString m_filePath;
    QLabel *m_errMsg;
    QTranslator m_translator;
};

#endif
