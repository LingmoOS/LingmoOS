// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERMISSIONCONFIG_H
#define PERMISSIONCONFIG_H

#include <QObject>
#include <QJsonObject>

#include <dtkwidget_config.h>

// 在 5.6.9 版本后提供
#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
#include <DWaterMarkHelper>
// 较低版本(<5.5.50)不提供插件相关接口
#include <DPrintPreviewDialog>

DWIDGET_USE_NAMESPACE
#endif  // DTKWIDGET_CLASS_DWaterMarkHelper

class PermissionConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int printCount READ printCount NOTIFY printCountChanged)

    explicit PermissionConfig(QObject *parent = nullptr);
    ~PermissionConfig() override;

public:
    static PermissionConfig *instance();
    bool isValid() const;
    bool isCurrentIsTargetImage() const;

    enum Authorise {
        NoAuth = 0,
        EnableEdit = 0x1,  // 编辑权限，旋转图片是否允许写回文件
        EnableCopy = 0x2,
        EnableDelete = 0x4,
        EnableRename = 0x8,
        EnableSwitch = 0x10,  // 是否允许切换文件
        EnableWallpaper = 0x20,

        EnableReadWaterMark = 0x1000,
        EnablePrintWaterMark = 0x2000,
    };
    Q_DECLARE_FLAGS(Authorises, Authorise)

    // 权限/配置查询接口
    bool checkAuthFlag(Authorise authFlag, const QString &fileName = QString()) const;
    bool isPrintable(const QString &fileName = QString()) const;
    bool hasReadWaterMark() const;
    bool hasPrintWaterMark() const;

    enum TidType {
        TidOpen = 1000200050,
        TidEdit = 1000200051,
        TidCopy = 1000200052,
        TidPrint = 1000200053,
        TidClose = 1000200054,
        TidSwitch = 1000200055,
        TidSetWallpaper = 1000200056,
        TidDelete = 1000200057,
        TidRename = 1000200058,
    };

    // 动作触发通知接口
    void triggerAction(TidType tid, const QString &fileName);
    void triggerPrint(const QString &fileName);
    Q_SIGNAL void authoriseNotify(const QJsonObject &data);

    int printCount() const;
    bool isUnlimitPrint() const;
    Q_SIGNAL void printCountChanged();

    void setCurrentImagePath(const QString &fileName);
    Q_SIGNAL void currentImagePathChanged(const QString &fileName, bool isTargetImage);
    QString targetImage() const;

#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
    // 阅读/打印水印
    WaterMarkData readWaterMarkData() const;
    WaterMarkData printWaterMarkData() const;

    bool installFilterPrintDialog(DPrintPreviewDialog *dialog);
    bool setDialogPrintWatermark(DPrintPreviewDialog *dialog) const;
#endif  // DTKWIDGET_CLASS_DWaterMarkHelper

    Q_SLOT void activateProcess(qint64 pid);
    void initFromArguments(const QStringList &arguments);

private:
    // 解析配置
    bool parseConfigOption(const QStringList &arguments, QString &configParam, QStringList &imageList) const;
    void initAuthorise(const QJsonObject &param);

#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
    // 用于兼容主线/定制线的通用水印定义，适配不同接口
    struct AdapterWaterMarkData
    {
        enum AWaterMarkType { None, Text, Image };  // 水印类型
        enum AWaterMarkLayout { Center, Tiled };    // 水印布局

        AWaterMarkType type = None;        // 水印类型
        AWaterMarkLayout layout = Center;  // 水印布局
        qreal scaleFactor = 1.0;           // 整体大小缩放系数
        int spacing = 0;                   // 间距
        int lineSpacing = 0;               // 行间距
        QString text;                      // 文本水印内容
        QFont font;                        // 文本水印字体
        QColor color;                      // 文本水印颜色
        qreal rotation = 0;                // 水印旋转角度(0~360)
        qreal opacity = 1;                 // 水印透明度(0~1)
        QImage image;                      // 图片水印中的图片
        bool grayScale = true;             // 是否灰度化图片
    };
    WaterMarkData convertAdapterWaterMarkData(const AdapterWaterMarkData &adptData) const;

    void initReadWaterMark(const QJsonObject &param);
    void initPrintWaterMark(const QJsonObject &param);
    void detectWaterMarkPluginExists();
    bool initWaterMarkPluginEnvironment();
#endif  // DTKWIDGET_CLASS_DWaterMarkHelper

    bool checkAuthInvalid(const QString &fileName = QString()) const;
    void reduceOnePrintCount();
    void triggerNotify(const QJsonObject &data);
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum Status { NotOpen, Open, Close };

    QString currentImagePath;  // 当前展示的图片文件路径
    QString targetImagePath;   // 权限控制指向的文件路径
    bool valid = false;
    int printLimitCount = 0;  // 打印记数，-1表示无限制
    Status status = NotOpen;  // 被控制权限图片的状态
    Authorises authFlags = NoAuth;

    // 内部拓展参数
    bool ignoreDevicePixelRatio = false;  // 过滤设备显示比率，按照原生像素计算
    bool useWaterMarkPlugin = false;      // 是否使用水印插件
    bool breakPrintSpacingLimit = false;  // 打破打印间距限制
    qreal printRowSpacing = 0.0;
    qreal printColumnSpacing = 0.0;

#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
    AdapterWaterMarkData readAdapterWaterMark;  // 水印数据，不区分DTK版本
    AdapterWaterMarkData printAdapterWaterMark;

    WaterMarkData readWaterMark;  // 水印数据，根据不同DTK版本支持接口不同
    WaterMarkData printWaterMark;
#endif  // DTKWIDGET_CLASS_DWaterMarkHelper

    Q_DISABLE_COPY(PermissionConfig)
};

#endif  // PERMISSIONCONFIG_H
