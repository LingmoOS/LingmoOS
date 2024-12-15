// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINTOOLWIDGET_H
#define MAINTOOLWIDGET_H

#include "toolbutton.h"
#include "filter.h"

#include <DStackedWidget>
#include <DLabel>
#include <DWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class MainToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit MainToolWidget(DWidget *parent = nullptr);
    ~MainToolWidget();

    void initWidget();
    //录屏截屏控件按钮初始化
    void initMainLabel();
    void installTipHint(QWidget *w, const QString &hintstr);
    void installHint(QWidget *w, QWidget *hint);

signals:
    /**
     * @brief 发射截图功能或录屏功能被点击的信号
     * @param checked : 点击与否
     * @param type : "record" or "shot"
     */
    void buttonChecked(bool checked, QString type);
public slots:
//    void switchContent(QString shapeType);
    void setRecordButtonOut();
    void setRecordLauchMode(const unsigned int funType);


private:
    QHBoxLayout *m_baseLayout;
    DLabel *m_mainTool;
    bool m_isChecked;
    ToolButton *m_recordBtn;
    ToolButton *m_shotBtn;
    HintFilter *hintFilter = nullptr;
};

#endif // MAINTOOLWIDGET_H
