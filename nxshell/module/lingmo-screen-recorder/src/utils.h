// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H
//#include <dwindowmanager.h>
#include <DPushButton>
#include <DImageButton>

#include <QObject>
#include <QPainter>
#include <QAction>
#include <QString>
#include <QList>
#include <QScreen>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class Utils : public QObject
{
    Q_OBJECT
public:
    struct ScreenInfo {
        int x;
        int y;
        int width;
        int height;
        QString name;
        ~ScreenInfo() {}
        QString toString()
        {
            return QString("ScreenName: %1 ,width: %2 ,height: %3 ,x: %4 ,y: %5").arg(name).arg(width).arg(height).arg(x).arg(y);
        }
    };
public:
    static QSize getRenderSize(int fontSize, QString string);
    static QString getQrcPath(QString imageName);
//    static void blurRect(DWindowManager *windowManager, int widgetId, QRectF rect);
//    static void blurRects(DWindowManager *windowManager, int widgetId, QList<QRectF> rects);
//    static void clearBlur(DWindowManager *windowManager, int widgetId);
    static void drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity = 0.4);
    static void drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect);
    static void passInputEvent(int wid);
    static void setFontSize(QPainter &painter, int textSize);
//    static void warnNoComposite();
//    static void notSupportWarn();
//    static QString getRecordingSaveDirectory();
    static void setAccessibility(DPushButton *button, const QString name);
    static void setAccessibility(DImageButton *button, const QString name);
    static void setAccessibility(QAction *action, const QString name);
    static bool is3rdInterfaceStart;
    static bool isTabletEnvironment;
    static bool isWaylandMode;
    static QString appName;
    /**
     * @brief 本机是否存在ffmpeg相关库 true:存在 false:不存在
     */
    static bool isFFmpegEnv;
    static bool isRootUser;
    /**
     * @brief 当前屏幕缩放比列
     */
    static qreal pixelRatio;
    static int themeType;
    /**
     * @brief 不支持截图录屏时的警告
     */
    static void notSupportWarn();

    /**
     * @brief 传入屏幕上理论未经缩放的点，获取缩放后实际的点
     * @param 理论未经缩放的点
     * @return 缩放后实际的点
     */
    static QPoint getPosWithScreen(QPoint pos);

    /**
     * @brief 传入屏幕上已经缩放后的点，获取理论上实际的点
     * @param 缩放后实际的点
     * @return 理论未经缩放的点
     */
    static QPoint getPosWithScreenP(QPoint pos);

    /**
     * @brief 传入屏幕上已经缩放后的点，获取理论上实际的点
     * @param 缩放后实际的点
     * @return 理论未经缩放的点
     */
    static QList<ScreenInfo> getScreensInfo();

    /**
     * @brief 对目标区域做穿透处理
     * @param 窗口id
     * @param 区域位置x坐标
     * @param 区域位置y坐标
     * @param 区域宽
     * @param 区域高
     */
    static void getInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height);
    /**
     * @brief 取消对目标区域的穿透处理
     * @param wid  窗口id
     * @param x  区域位置x坐标
     * @param y  区域位置y坐标
     * @param width  区域宽
     * @param height  区域高
     */
    static void cancelInputEvent(const int wid, const short x, const short y, const unsigned short width, const unsigned short height);

    /**
     * @brief 取消对目标区域的穿透处理
     * @param wid  窗口id
     * @param x  区域位置x坐标
     * @param y  区域位置y坐标
     * @param width  区域宽
     * @param height  区域高
     */
    static void cancelInputEvent1(const int wid, const short x, const short y, const unsigned short width, const unsigned short height);

    /**
     * @brief isSysHighVersion1040 判断当前系统版本是否高于1040
     * @return
     */
    static bool isSysHighVersion1040();

    /**
     * @brief showCurrentSys 显示
     */
    static void showCurrentSys();

    /**
     * @brief 使能XGrabButton抓取所有的鼠标点击事件
     */
    static void enableXGrabButton();

    /**
     * @brief 失能XGrabButton抓取所有的鼠标点击事件
     */
    static void disableXGrabButton();

    static void getAllWindowInfo(const quint32 winId, const int width, const int height, QList<QRect> &windowRects, QList<QString> &windowNames);
    static bool checkCpuIsZhaoxin();

    /**
     * @brief 获取处理器名称
     * @return 处理器名称
     */
    static QString getCpuModelName();

    static QString getCurrentAudioChannel();

    /**
     * @brief 通过键盘移动光标
     * @param currentCursor:当前光标的位置
     * @param keyEvent:键盘事件
     */
    static void cursorMove(QPoint currentCursor, QKeyEvent *keyEvent);

};

#endif //UTILS_H
