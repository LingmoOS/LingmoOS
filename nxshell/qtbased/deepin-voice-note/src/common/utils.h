// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DTextEdit>

#include <QString>
#include <QDateTime>
#include <QPixmap>
#include <QTextDocumentFragment>
#include <QTextDocument>

DWIDGET_USE_NAMESPACE

class VNoteBlock;

class Utils
{
public:
    Utils();
    //格式化时间
    static QString convertDateTime(const QDateTime &dateTime);
    //加载图标
    static QPixmap renderSVG(const QString &filePath, const QSize &size, DApplication *pApp);
    //加载图标
    static QPixmap loadSVG(const QString &fileName, bool fCommon = false);
    //编辑器内容高亮
    static int highTextEdit(QTextDocument *textDoc, const QString &searchKey,
                            const QColor &highColor, bool undo = false);
    //格式化录音时长
    static QString formatMillisecond(qint64 millisecond, qint64 minValue = 1);
    //内存数据同步编辑器显示内容
    static void documentToBlock(VNoteBlock *block, const QTextDocument *doc);
    //编辑器同步内存数据
    static void blockToDocument(const VNoteBlock *block, QTextDocument *doc);
    //设置编辑器字体颜色
    static void setDefaultColor(QTextDocument *srcDoc, const QColor &color);
    //设置标题栏tab焦点
    static void setTitleBarTabFocus(QKeyEvent *event);
    //图片转base64
    static bool pictureToBase64(QString imgPath, QString &base64);
    //判断是否龙芯平台
    static bool isLoongsonPlatform();
    //过滤文件名内容
    static QString filteredFileName(QString fileName, const QString &defaultName = "");
    //判断是否wayland
    static bool isWayland();
};
