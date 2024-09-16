// SPDX-FileCopyrightText: 2011-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "settings.h"
#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <QString>
#include <QImage>
#include <DMainWindow>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusReply>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)      if((p)) { delete (p); (p) = nullptr;}
#endif

#define DEEPIN_THEME        QString("%1share/deepin-editor/themes/deepin.theme").arg(LINGLONG_PREFIX)
#define DEEPIN_DARK_THEME   QString("%1share/deepin-editor/themes/deepin_dark.theme").arg(LINGLONG_PREFIX)
#define DATA_SIZE_1024      1024
#define TEXT_EIDT_MARK_ALL  "MARK_ALL"
#define PROC_MEMINFO_PATH   "/proc/meminfo"
#define COPY_CONSUME_MEMORY_MULTIPLE 9      //复制文本时内存占用系数
#define PASTE_CONSUME_MEMORY_MULTIPLE 7     //粘贴文本时内存占用系数

class Utils
{
public:
    /**
     * @brief 区间交叉类型
     */
    enum RegionIntersectType {
        ELeft,              ///< 活动区间在固定区间左侧 例如 [0, 9] 和 [-5, -1]
        ERight,             ///< 活动区间在固定区间右侧 例如 [0, 9] 和 [10, 15]

        EIntersectLeft,     ///< 活动区间在固定区间左侧存在范围重叠 例如 [0, 9] 和 [-5, 5]
        EIntersectRight,    ///< 活动区间在固定区间右侧存在范围重叠 例如 [0, 9] 和 [5, 15]
        EIntersectOutter,   ///< 活动区间包含固定区间            例如 [0, 9] 和 [-10, 10]
        EIntersectInner,    ///< 活动区间处于固定区间内部         例如 [0, 9] 和 [5, 6]
    };

    /**
     * @brief 当前运行时系统环境
     */
    enum SystemVersion {
        V20,
        V23,
    };

    static QString getQrcPath(const QString &imageName);
    static QString getQssPath(const QString &qssName);
    static QSize getRenderSize(int fontSize, const QString &string);
    static void setFontSize(QPainter &painter, int textSize);
    static void applyQss(QWidget *widget, const QString &qssName);
    static QString getKeyshortcut(QKeyEvent *keyEvent);
    static QString getKeyshortcutFromKeymap(Settings *settings, const QString &keyCategory, const QString &keyName);
    static bool fileExists(const QString &path);
    static bool fileIsWritable(const QString &path);
    static bool fileIsHome(const QString &path);
    static void passInputEvent(int wid);
    static QPixmap dropShadow(const QPixmap &source, qreal radius, const QColor &color, const QPoint &offset);
    static QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color);
    static QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    static QByteArray getEncode(const QByteArray &data);
    static qreal easeInOut(qreal x);
    static qreal easeInQuad(qreal x);
    static qreal easeInQuint(qreal x);
    static qreal easeOutQuad(qreal x);
    static qreal easeOutQuint(qreal x);
    static QVariantMap getThemeMapFromPath(const QString &filepath);
    static bool isMimeTypeSupport(const QString &filepath);
    static bool isDraftFile(const QString &filepath);
    // 返回文件是否为备份文件
    static bool isBackupFile(const QString &filepath);
    static QStringList cleanPath(const QStringList &filePaths);
    // 返回程序使用的本地数据路径(存放临时、备份文件)
    static QString localDataPath();
    static const QStringList getEncodeList();
    static QPixmap renderSVG(const QString &filePath, const QSize &size, bool bIsScale = true);
    static QList<QColor> getHiglightColorList();
    /*******************************************************************************
     1. @函数:    clearChildrenFocus
     2. @作者:    n014361 王培利
     3. @日期:    2020-05-08
     4. @说明:    清空控件内部所有子控件的焦点获取
            安全考虑，不要全局使用．仅在个别控件中使用
    *******************************************************************************/
    static void clearChildrenFocus(QObject *objParent);
    //清除　控件及子控件所以焦点　梁卫东　２０２０－０９－１４　１０：３４：１９
    static void clearChildrenFoucusEx(QWidget *pWidget);
    //设置所有控件焦点 梁卫东　２０２０－０９－１５　１７：５５：１８
    static void setChildrenFocus(QWidget *pWidget, Qt::FocusPolicy policy = Qt::StrongFocus);
    //根据指定名称获取进程数量 秦浩玲　2021-01-26
    static int getProcessCountByName(const char *pstrName);
    //批量结束指定名称的进程 秦浩玲　2021-01-26
    static void killProcessByName(const char *pstrName);
    //计算字符串MD5哈希值 秦浩玲　2021-01-28
    static QString getStringMD5Hash(const QString &input);
    //通过dbus接口从任务栏激活窗口 add by guoshaoyu 2021-04-07
    static bool activeWindowFromDock(quintptr winId);

    //判断是否共享文件夹且只读
    static bool isShareDirAndReadOnly(const QString &filePath);

    static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);

    //return system language
    static QString getSystemLan();
    // 取得系统版本是否为 V23
    static SystemVersion getSystemVersion();

    static bool isWayland();
    static bool isTreeland();

    // 计算换行内容 text: 原始文本内容， nWidth: 一行最大宽度， font:字体大小, nElideRow: 最大显示行数，超出最大行时，中间内容加···省略号显示
    static QString lineFeed(const QString &text, int nWidth, const QFont &font, int nElidedRow = 2);

    // 判断 [x1, y1] 和 [x2, y2] 区间是否存在交集，返回交集类型
    static RegionIntersectType checkRegionIntersect(int x1, int y1, int x2, int y2);
    // 取得当前文本编辑器支持的编码格式，从文件 :/encodes/encodes.ini 中读取
    static QVector<QPair<QString,QStringList>> getSupportEncoding();
    static QStringList getSupportEncodingList();

    /**
    * @brief libPath 动态库路径
    * @param strlib 路径的字符串
    */
    static QString libPath(const QString &strlib);

    // 加载外部定制库，如ZPD定制库的等
    static void loadCustomDLL();
    // 根据传入的文件路径返回是否允许对此文件内容进行剪切或拷贝
    static bool enableClipCopy(const QString &filePath);
    // 文件关闭前记录当前关闭的文件路径
    static void recordCloseFile(const QString &filePath);

    // 发送浮动提示信息，并且字体大小跟随 qApp 应用默认字体而不是父窗口字体
    static void sendFloatMessageFixedFont(QWidget *par, const QIcon &icon, const QString &message);

private:
    static QString m_systemLanguage;
};
