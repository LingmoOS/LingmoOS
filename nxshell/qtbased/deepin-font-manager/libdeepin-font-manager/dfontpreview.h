// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTPREVIEW_H
#define DFONTPREVIEW_H

#include <QWidget>
#include <QLabel>
#include <QFontDatabase>

#include <ft2build.h>

#include FT_FREETYPE_H

/*************************************************************************
 <Class>         DFontPreview
 <Description>   字体预览视图类
 <Author>
 <Note>          null
*************************************************************************/
class DFontPreview : public QWidget
{
    Q_OBJECT

public:
    explicit DFontPreview(QWidget *parent = nullptr);
    ~DFontPreview();
    //传入当前预览字体的路径
    void setFileUrl(const QString &url);
    //检查字体库中是否有当前字符信息
    static bool checkFontContainText(FT_Face face, const QString &text);
    //构建预览字符串
    static QString buildCharlistForFace(FT_Face face, int length);

    QFontDatabase fontDatabase;

    bool m_needScroll = false;
    int viewWidth = 1204;
    int currentMaxWidth = 1;
protected:
    //重写绘图事件-实现预览效果
    void paintEvent(QPaintEvent *);
    //获取预览绘制的起始点
    QPoint adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const;
    //判断是否需要出现底部水平滚动条
    void isNeedScroll(const int width);

private:
    //初始化预览文本内容
    void initContents();
    //获取预览文本-获取小写英文文本
    QString getSampleString();
    //获得当前语言下可预览字符串
    QString getLanguageSampleString(const QString &language);

private:
    FT_Library m_library;
    FT_Face m_face;
    FT_Error m_error = 1;
    QHash<QString, QString> m_contents;
};

#endif  // DFONTPREVIEW_H
