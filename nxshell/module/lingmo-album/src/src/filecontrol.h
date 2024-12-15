// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECONTROL_H
#define FILECONTROL_H

#include "configsetter.h"
#include "unionimage/unionimage_global.h"

#include <QObject>
#include <QFileInfo>
#include <QTimer>
#include <QImage>
#include <QImageReader>
#include <QMap>
#include <QFileSystemWatcher>

#include "imagedata/imagefilewatcher.h"

class OcrInterface;
class QProcess;

class FileControl : public QObject
{
    Q_OBJECT
public:
    explicit FileControl(QObject *parent = nullptr);
    ~FileControl();

    Q_INVOKABLE QString standardPicturesPath() const;

    Q_INVOKABLE void resetImageFiles(const QStringList &filePaths = {});       // 重设当前展示图片列表
    Q_INVOKABLE QStringList getDirImagePath(const QString &path);              // 获得路径下的所有图片路径
    Q_INVOKABLE bool isCurrentWatcherDir(const QUrl &path);                    // 判断是否为当前正在监控的文件路径
    Q_INVOKABLE QString slotGetInfo(const QString &key, const QString &path);  // 获取某项info
    Q_INVOKABLE QString slotGetFileName(const QString &path);                  // 获取文件名
    Q_INVOKABLE QString slotGetFileNameSuffix(const QString &path);            // 获取文件名及其后缀
    Q_INVOKABLE QString getNamePath(const QString &oldPath, const QString &newName);  // 公共接口，获得路径
    Q_INVOKABLE QString slotFileSuffix(const QString &path, bool ret = true);         // 文件后缀

    Q_INVOKABLE void setWallpaper(const QString &imgPath);       // 设置壁纸
    Q_INVOKABLE bool deleteImagePath(const QString &path);       // 删除文件
    Q_INVOKABLE bool displayinFileManager(const QString &path);  // 在文件目录中显示
    Q_INVOKABLE void copyImage(const QString &path);             // 复制图片
    Q_INVOKABLE void copyText(const QString &str);               // 复制文字
    Q_INVOKABLE void ocrImage(const QString &path, int index);   // 进行ocr识别
    Q_INVOKABLE void showPrintDialog(const QString &path);       // 调用打印接口
    Q_INVOKABLE void showPrintDialog(const QStringList &paths);  // 打印接口，多路径
    Q_INVOKABLE QStringList parseCommandlineGetPaths();          // 解析命令行

    Q_INVOKABLE bool isCheckOnly();                               // 判断当前进程是否为唯一看图实例
    Q_INVOKABLE bool isSupportSetWallpaper(const QString &path);  // 是否支持设置壁纸
    Q_INVOKABLE bool isCanSupportOcr(const QString &path);
    Q_INVOKABLE bool isCanRename(const QString &path);
    Q_INVOKABLE bool isCanReadable(const QString &path);
    Q_INVOKABLE bool isRotatable(const QString &path);  // 是否可以被选旋转
    Q_INVOKABLE bool isCanWrite(const QString &path);   // 是否可以被写入
    Q_INVOKABLE bool isCanDelete(const QString &path);  // 是否可以被删除
    Q_INVOKABLE bool isCanDelete(const QStringList &pathList);
    Q_INVOKABLE bool isImage(const QString &path);      // 是否是图片
    Q_INVOKABLE bool isVideo(const QString &path);      // 是否是视频
    Q_INVOKABLE bool isCanPrint(const QVariantList &pathList); // 是否可以打印
    Q_INVOKABLE bool isCanPrint(const QString &path);

    Q_INVOKABLE bool slotFileReName(const QString &name, const QString &filepath, bool isSuffix = false);  // 文件重命名
    Q_INVOKABLE bool isShowToolTip(const QString &oldPath, const QString &name);  // 判断重命名是否显示toolTip

    // 设置/获取配置文件相关值
    Q_INVOKABLE QVariant getConfigValue(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    Q_INVOKABLE void setConfigValue(const QString &group, const QString &key, const QVariant &value);
    Q_INVOKABLE void saveSetting();
    Q_INVOKABLE int getlastWidth();
    Q_INVOKABLE int getlastHeight();
    Q_INVOKABLE void setSettingWidth(int width);
    Q_INVOKABLE void setSettingHeight(int height);
    Q_INVOKABLE void setEnableNavigation(bool b);  // 设置是否允许展示导航窗口
    Q_INVOKABLE bool isEnableNavigation();

    Q_INVOKABLE QUrl getCompanyLogo();  // 获取公司 Logo 图标地址

    Q_INVOKABLE void showShortcutPanel(int windowCenterX, int windowCenterY);  // 显示快捷键面板
    Q_INVOKABLE void terminateShortcutPanelProcess();                          // 结束快捷键面板进程

    Q_INVOKABLE void copyImage(const QStringList &paths);// 复制多个
    Q_INVOKABLE bool isRotatable(const QStringList &pathList);
    Q_INVOKABLE QString getDirPath(const QString &path); // 获得路径下的dir路径
    Q_INVOKABLE bool pathExists(const QString &path);    // 路径是否存在
    Q_INVOKABLE bool haveImage(const QVariantList &urls);// 是否存在图片
    Q_INVOKABLE bool haveVideo(const QVariantList &urls);// 是否存在视频
    Q_INVOKABLE bool isFile(const QString &path);        // 是否是文件
    //旋转文件 pathList：要旋转的图片路径链表；rotateAngel: > 0 顺时针旋转， < 0 逆时针旋转
    Q_INVOKABLE bool rotateFile(const QVariantList &pathList, const int &rotateAngel);
    Q_INVOKABLE bool rotateFile(const QString &path, const int &rotateAngel);
    Q_INVOKABLE void slotRotatePixCurrent(bool bNotifyExternal = false); //旋转
    Q_INVOKABLE void setViewerType(imageViewerSpace::ImgViewerType type);      // 设置图片查看类型
    Q_INVOKABLE bool isAlbum();                                                // 提供接口，程序是否是相册模式
    Q_INVOKABLE bool checkMimeUrls(const QList<QUrl> &urls);                   // 检查是否可以接受当前的拖拽导入行为

    // 打开文件，触发 OpenImageWidget.qml:openImageFile 处理，fileName需为url路径
    Q_SIGNAL void openImageFile(const QString &fileName);
    // 文件被重命名
    Q_SIGNAL void imageRenamed(const QUrl &oldName, const QUrl &newName);
    // 文件变更通知信号，文件被移动、删除、覆盖等操作时触发
    Q_SIGNAL void imageFileChanged(const QString &fileName);

signals:
    // 通知相册刷新缩略图内容
    void callSavePicDone(const QString &path);
    // 通知打开了非图片/视频文件的格式
    void invalidFormat();

private:
    // 生成用于快捷键面板的字符串
    QString createShortcutString();

private:
    OcrInterface *m_ocrInterface;

    QString m_shortcutString;  // 快捷键字符串，将采用懒加载模式，需要通过createShortcutString()函数使用
    QProcess *m_shortcutViewProcess;               // 快捷键面板进程
    QStringList listsupportWallPaper;              // 支持设置壁纸的图片后缀类型
    ImageFileWatcher *imageFileWatcher = nullptr;  // 图片文件变更监控

    QString m_currentPath;                    // 当前操作的旋转图片路径
    QMap<QString, QString> m_currentAllInfo;  // 当前图片详细信息

    LibConfigSetter *m_config;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    int m_lastSaveWidth = 0;
    int m_lastSaveHeight = 0;
    int m_rotateAngel = 0;             // 旋转角度
    QTimer *m_tSaveSetting = nullptr;  // 保存配置信息定时器，在指定时间内只保存一次
    QTimer *m_tSaveImage = nullptr;    // 保存旋转图片定时器，在指定时间内只旋转一次

    QImageReader *m_currentReader = nullptr;
    QHash<QString, QString>     m_cacheFileInfo;    // 缓存的图片信息，用于判断图片信息是否变更 QHash<完整路径, url信息>
    QHash<QString, QString>     m_removedFile;      // 缓存被移除的文件信息(FileWatcher在文件删除/移动后将不会继续观察)
    QFileSystemWatcher          *m_pFileWathcer;    // 文件观察类，用于提示文件变更
    QString fileRenamed;                            // 文件重命名缓存，用于阻止文件变更操作
    imageViewerSpace::ImgViewerType m_viewerType { imageViewerSpace::ImgViewerTypeLocal };
};

#endif  // FILECONTROL_H
