// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNCOMPRESSPAGE_H
#define UNCOMPRESSPAGE_H

#include "commonstruct.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class UnCompressView;
class CustomCommandLinkButton;
class CustomPushButton;
struct UnCompressParameter;

// 解压界面（列表）
class UnCompressPage : public DWidget
{
    Q_OBJECT
public:
    explicit UnCompressPage(QWidget *parent = nullptr);
    ~UnCompressPage() override;

    /**
     * @brief setArchiveFullPath    设置压缩包全路径
     * @param strArchiveFullPath    压缩包全路径
     * @param unCompressPar         压缩参数
     */
    void setArchiveFullPath(const QString &strArchiveFullPath, UnCompressParameter &unCompressPar);

    /**
     * @brief archiveFullPath   获取压缩包全路径
     * @return
     */
    QString archiveFullPath();

    /**
     * @brief setDefaultUncompressPath  设置默认解压路径
     * @param strPath   解压路径
     */
    void setDefaultUncompressPath(const QString &strPath);

    /**
     * @brief refreshArchiveData    加载结束刷新数据
     */
    void refreshArchiveData();
    /**
     * @brief refreshDataByCurrentPathChanged 刷新删除后的显示数据
     */
    void refreshDataByCurrentPathChanged();

    /**
     * @brief addNewFiles   追加新文件
     * @param listFiles     新文件
     */
    void addNewFiles(const QStringList &listFiles);

    /**
     * @brief getCurPath    获取当前层级路径
     * @return
     */
    QString getCurPath();

    /**
     * @brief clear 清空数据
     */
    void clear();

    UnCompressView *getUnCompressView() const;

    CustomPushButton *getUnCompressBtn() const;

    CustomCommandLinkButton *getUncompressPathBtn() const;

protected:
    /**
     * @brief resizeEvent   刷新解压路径显示
     * @param e
     */
    void resizeEvent(QResizeEvent *e) override;

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief elidedExtractPath     对解压路径字符串进行截取（防止太长）
     * @param strPath               解压路径
     * @return
     */
    QString elidedExtractPath(const QString &strPath);

Q_SIGNALS:
    /**
     * @brief signalFileChoose  选择文件信号
     */
    void signalFileChoose();

    /**
     * @brief signalUncompress  点击解压按钮
     * @param strTargetPath     解压目标路径
     */
    void signalUncompress(const QString &strTargetPath);

    /**
     * @brief signalExtract2Path    提取压缩包中文件
     * @param listSelEntry          当前选中的文件
     * @param stOptions             提取参数
     */
    void signalExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief signalDelFiels    删除压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        删除文件总大小
     */
    void signalDelFiles(const QList<FileEntry> &listSelEntry, qint64 qTotalSize);

    /**
     * @brief signalDelFiels    重命名压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        重命名文件总大小
     */
    void signalRenameFile(const FileEntry &SelEntry, qint64 qTotalSize);

    /**
     * @brief signalOpenFile    打开压缩包中文件
     * @param entry             待打开的文件
     * @param strProgram        应用程序名（为空时，用默认应用程序打开）
     */
    void signalOpenFile(const FileEntry &entry, const QString &strProgram = "");

    /**
     * @brief signalAddFiles2Archive    向压缩包中添加文件
     * @param listFiles                 待添加的文件
     * @param strPassword               密码
     */
    void signalAddFiles2Archive(const QStringList &listFiles, const QString &strPassword);
    /**重命名信号
      */
    void sigRenameFile();

private Q_SLOTS:
    /**
     * @brief slotUncompressClicked 解压按钮点击槽函数
     */
    void slotUncompressClicked();

    /**
     * @brief slotUnCompressPathClicked     设置解压路径
     */
    void slotUnCompressPathClicked();

    /**
     * @brief signalFileChoose  选择文件信号
     */
    void slotFileChoose();

private:
    UnCompressView *m_pUnCompressView;    // 压缩列表
    CustomCommandLinkButton *m_pUncompressPathBtn; //解压目标路径按钮
    CustomPushButton *m_pUnCompressBtn;   //解压按钮

    QString m_strArchiveFullPath;       // 压缩包名称
    QString m_strUnCompressPath;    // 解压路径
};

#endif // UNCOMPRESSPAGE_H
