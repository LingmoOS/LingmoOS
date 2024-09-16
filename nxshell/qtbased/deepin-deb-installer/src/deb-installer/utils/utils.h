// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QHash>
#include <QWidget>

#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QDir>
#include <DDialog>
#include <QPushButton>
#include <DPushButton>
#define dApp (static_cast<DApplication *>(QCoreApplication::instance()))

DWIDGET_USE_NAMESPACE

class DebApplicationHelper : public DGuiApplicationHelper
{
    Q_OBJECT

public:
    static DebApplicationHelper *instance();

    DPalette standardPalette(DGuiApplicationHelper::ColorType type) const;
    DPalette palette(const QWidget *widget, const QPalette &base = QPalette()) const;
    void setPalette(QWidget *widget, const DPalette &palette);
    void resetPalette(QWidget *widget);
};

class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);

    enum FontType { SourceHanSansMedium, SourceHanSansNormal, DefautFont };

    enum VerifyResultCode {
        OtherError = -1,     // 其他错误
        VerifySuccess,       // 验证成功
        DebfileInexistence,  // 解压deb文件用的临时目录不存在
        ExtractDebFail,      // 提取deb包内容时出错
        DebVerifyFail        // deb包验证失败
    };

    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;

    static QString loadFontFamilyByType(FontType fontType);                                         // 加载字体类型
    static QFont loadFontBySizeAndWeight(const QString &fontFamily, int fontSize, int fontWeight);  // 设置字体大小等
    static void
    bindFontBySizeAndWeight(QWidget *widget, const QString &fontFamily, int fontSize, int fontWeight);  // 绑定字体大小等
    static QString fromSpecialEncoding(const QString &inputStr);                                        // 字体编码处理
    static QString holdTextInRect(const QFont &font, const QString &srcText, const QSize &size);  // 针对字体截断处理函数
    static QString holdTextInRect(const QFont &font, const QString &srcText, const int &width);  // 针对配置界面字体截断处理函数
    static int returnfileIsempty(QString strfilepath, QString strfilename);                      // 返回文件是否存在
    static VerifyResultCode Digital_Verify(const QString &filepath_name);                        // 验证deb数字签名
    static bool Return_Digital_Verify(const QString &strfilepath, const QString &strfilename);  // 返回验证工具是否存在
    static bool checkPackageReadable(const QString &packagePath);  // 返回软件包是否允许读取

    static int compareVersion(const QString &v1, const QString &v2);

    static bool isDevelopMode();  // Check if develop mode (root)
};

#endif
