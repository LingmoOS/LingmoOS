// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTPREVIEWER_H
#define DFONTPREVIEWER_H

#include <DFrame>

DWIDGET_USE_NAMESPACE

/*************************************************************************
 <Class>         DFontPreviewer
 <Description>   负责控制quickinstallwindow中预览字体显示内容
 <Author>
 <Note>          null
*************************************************************************/
class DFontPreviewer : public QWidget
{
    Q_OBJECT
public:
    explicit DFontPreviewer(QWidget *parent = nullptr);
    //绘制预览字体
    void paintEvent(QPaintEvent *event) override;

    //初始化数据
    void InitData();

    //初始化链接
    void InitConnections();

    //设置当前字体路径
    void setPreviewFontPath(const QString &font);

signals:
    //预览字体变化信号
    void previewFontChanged();

public slots:
    //预览字体变化后的处理
    void onPreviewFontChanged();

private:
    QString m_fontPath;
    QStringList m_previewTexts;
};

#endif // DFONTPREVIEWER_H
