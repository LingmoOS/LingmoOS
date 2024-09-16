// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPICKERMANAGER_H
#define CPICKERMANAGER_H

#include <QWidget>
#include <QMap>

class CPickerManager;

/**
 * @brief CScreenshotWidget 显示当前屏幕的截图；
 * 每个屏幕将拥有一个CScreenshotWidget实例，实现当前屏幕的冻结显示
 */
class CScreenshotWidget: public QWidget
{
    Q_OBJECT
public:
    explicit CScreenshotWidget(CPickerManager *parent);

    /**
     * @brief setPixmap 设置要显示的图像
     */
    void    setPixmap(const QPixmap &pixMap);

    /**
     * @brief pixMap 显示的图像
     */
    QPixmap pixMap();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QPixmap         _scrennshotPixmap;
    CPickerManager *_parentManager = nullptr;
};

/**
 * @brief CPickerManager 管理当前的截图
 * 1.根据当前屏幕的个数创建对应个数的CScreenshotWidget实例，同时设置每个CScreenshotWidget窗口的显示图像
 * 2.实现DBUS调用接口colorPicked，返回取色器所取到的颜色值
 * 3.取色时的界面UI，鼠标样式及取色逻辑
 */
class CPickerManager: public QObject
{
    Q_OBJECT
    /**
     * @brief 声明DBUS名称接口
     */
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Picker")

public:
    CPickerManager();
    ~CPickerManager();

    /**
     * @brief setLanchFlag 设置启动的方式(这个决定了取到颜色后是否放送给对应程序)
     * @param tp取色器启动的方式，ELanchedBySelf表示自启动，ELanchedByOtherApp表示被其他程序调用启动
     * @param appName 当tp为ELanchedByOtherApp时有效，表示调用取色器程序的程序名称
     */
    enum ELanchType {ELanchedBySelf, ELanchedByOtherApp};
    void setLanchFlag(ELanchType tp, const QString &appName = "");

//    /**
//     * @brief caller 调用取色器的程序
//     */
//    QString caller();

//    /**
//     * @brief currentColor 当前的颜色
//     */
//    QColor currentColor() const;

signals:
    /**
     * @brief colorPicked 取色器选定颜色后，将发送颜色到剪切板[通过DBUS交互]
     * @param color 选定的颜色
     * @param colorType 选定的颜色的类型
     */
    void copyColor(QColor color, QString colorType);

signals:
    /**
     * @brief colorPicked 取色器选定颜色后，将发送该信号，发送到appid的应用[通过DBUS交互]
     * @param appName 表示要发送到的程序名
     * @param color 选定的颜色
     */
    Q_SCRIPTABLE void colorPicked(QString appName, QString color);

public slots:
    /**
     * @brief StartPick 设置调用取色器程序的程序名称，调用程序需要通过这个接口设置好名字，
     * 以进行取色后颜色的发送返回交互(colorPicked将使用这个名称)[通过DBUS交互]
     * @param appName 表示调用程序的程序名称
     */
    Q_SCRIPTABLE void StartPick(const QString &appName);

public slots:
    /**
     * @brief onMouseMove 响应鼠标移动
     */
    void onMouseMove(const QPoint &pos);

    /**
     * @brief onMouseMove 响应鼠标点下
     */
    void onMousePress(const QPoint &p, const int flag);

    /**
     * @brief handleMouseMove 更新当前鼠标位置颜色
     */
    void handleMouseMove();

private:
    /**
     * @brief initShotScreenWidgets 初始化显示界面
     */
    void initShotScreenWidgets();

    /**
     * @brief updateCursor 刷新鼠标光标
     */
    void updateCursor(const QPixmap &pixMap, const QPoint &posInPixmap);

    /**
     * @brief ensureDeskTopPixmap 保证桌面图像获取到
     */
    void ensureDeskTopPixmap();

    /**
     * @brief getScreenShotPixmap 获取到某一屏幕的图像
     */
    QPixmap getScreenShotPixmap(QScreen *pScreen);

    /**
     * @brief getDesktopPixmap 获取到桌面图像(多屏时亦保证正确)
     */
    static QPixmap getDesktopPixmap();

    /**
     * @brief isSpcialPlatform 是否当前是特殊平台
     * (特殊平台下需要进行不同的获取桌面图像的方式如wayland，以后考虑枚举方式)
     */
    static bool    isWaylandPlatform();

    void autoUpdate();

private:
    ELanchType                 _isLaunchByDBus = ELanchedBySelf;
    QString                    _appid;
    qreal                      _scaleFactor = 12;
    QMap<QScreen *, CScreenshotWidget *> _widgets;
    QPixmap                    _shadowPixmap;
    QColor                     _curColor;
    bool                       _desktopPixmapDirty = true;
    QPixmap                    _desktopPixmap;
    QTimer                    *_updateScreenshotTimer = nullptr;
    QPoint                     _pos;

    QPixmap                    _cursorPix;
    friend class CScreenshotWidget;
};
#endif // CPICKERMANAGER_H
