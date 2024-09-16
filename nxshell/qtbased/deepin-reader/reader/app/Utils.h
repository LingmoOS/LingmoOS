// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <QString>
#include <QIcon>

// gesture 触控板手势
#ifdef OS_BUILD_V23
#define GESTURE_SERVICE          "org.deepin.dde.Gesture"
#define GESTURE_PATH             "/org/deepin/dde/Gesture"
#define GESTURE_INTERFACE        "org.deepin.dde.Gesture"
#else
#define GESTURE_SERVICE          "com.deepin.daemon.Gesture"
#define GESTURE_PATH             "/com/deepin/daemon/Gesture"
#define GESTURE_INTERFACE        "com.deepin.daemon.Gesture"
#endif
#define GESTURE_SIGNAL           "Event"

class Utils : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief getKeyshortcut
     * 组合快捷键
     * @param keyEvent
     * @return
     */
    static QString getKeyshortcut(QKeyEvent *keyEvent);

    /**
     * @brief getInputDataSize
     * 数据单位换算,最大是G
     * @return
     */
    static QString getInputDataSize(const qint64 &);

    /**
     * @brief roundQPixmap
     * 生成带圆角缩略图
     * @param img_in
     * @param radius
     * @return
     */
    static QPixmap roundQPixmap(const QPixmap &img_in, int radius);

    /**
     * @brief copyText
     *复制文本到系统剪切板
     * @param sText
     */
    static void copyText(const QString &sText);

    /**
     * @brief 省略文本
     * @param fontMetrics
     * @param size
     * @param text
     * @param alignment
     * @return
     */
    static QString getElidedText(const QFontMetrics &fontMetrics, const QSize &size, const QString &text, Qt::Alignment alignment);

    /**
     * @brief copyFile
     * 复制文件
     * @param sourcePath
     * @param destinationPath
     * @return
     */
    static bool copyFile(const QString &sourcePath, const QString &destinationPath);

    /**
     * @brief copyImage
     * 复制缩略图
     * @param srcimg
     * @param x
     * @param y
     * @param w
     * @param h
     * @return
     */
    static QImage copyImage(const QImage &srcimg, int x, int y, int w, int h);

    /**
     * @brief getHiglightColorList
     * 注释高亮颜色列表
     * @return
     */
    static QList<QColor> getHiglightColorList();

    /**
     * @brief getCurHiglightColor
     * 当前注释高亮颜色
     * @return
     */
    static QColor getCurHiglightColor();

    /**
     * @brief setHiglightColorIndex
     * 设置全局注释高亮颜色
     * @param index
     */
    static void setHiglightColorIndex(int index);

    /**
     * @brief setObjectNoFocusPolicy
     * 设置对象的焦点策略
     * @param obj
     */
    static void setObjectNoFocusPolicy(QObject *obj);

    /**
     * @brief isWayland
     * 是否wayLand
     * @return
     */
    static bool isWayland();

    /**
     * @return 返回当前桌面环境是否为 Treeland
     */
    static bool isTreeland();

    /**
     * @brief 设置当前操作的文档名称
     * @param currentFilePath
     */
    static void setCurrentFilePath(QString currentFilePath);
private:
    static int m_colorIndex;
    static QString m_currenFilePath;
};

#endif // UTILS_H
