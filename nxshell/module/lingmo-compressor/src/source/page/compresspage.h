// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPRESSPAGE_H
#define COMPRESSPAGE_H

#include <DWidget>
#include "commonstruct.h"

DWIDGET_USE_NAMESPACE

class CompressView;
class CustomPushButton;
class QShortcut;

// 压缩界面（列表）
class CompressPage : public DWidget
{
    Q_OBJECT
public:
    explicit CompressPage(QWidget *parent = nullptr);
    ~CompressPage() override;

    /**
     * @brief addCompressFiles  添加压缩文件
     * @param listFiles     添加的文件
     */
    void addCompressFiles(const QStringList &listFiles);

    /**
     * @brief compressFiles     获取所有待压缩的文件
     * @return 所有待压缩的文件
     */
    QStringList compressFiles();

    /**
     * @brief getEntrys  获取待压缩的文件信息
     * @return      待压缩的文件信息
     */
    QList<FileEntry> getEntrys();
    /**
     * @brief refreshCompressedFiles    刷新压缩文件
     * @param bChanged                  是否有文件改变
     * @param strFileName               改变的文件名
     */
    void refreshCompressedFiles(bool bChanged = false, const QString &strFileName = "");

    /**
     * @brief clear 清空数据
     */
    void clear();

    /**
     * @brief getCompressView   // 获取压缩列表
     * @return
     */
    CompressView *getCompressView();

    /**
     * @brief getNextBtn    获取“下一步”按钮
     * @return
     */
    CustomPushButton *getNextBtn() const;

private:
    /**
    * @brief initUI    初始化界面
    */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

signals:
    /**
     * @brief signalFileChoose  选择文件信号
     */
    void signalFileChoose();

    /**
     * @brief signalCompressNext    压缩下一步信号
     */
    void signalCompressNextClicked();

    /**
     * @brief signalLevelChanged    目录层级变化
     * @param bRootIndex    是否是根目录
     */
    void signalLevelChanged(bool bRootIndex);

private slots:
    /**
     * @brief slotCompressNext  按钮点击下一步
     */
    void slotCompressNextClicked();

    /**
     * @brief compressLevelChanged  处理压缩层级变化
     * @param bRootIndex    是否是根目录
     */
    void slotCompressLevelChanged(bool bRootIndex);

    /**
     * @brief signalFileChoose  选择文件信号
     */
    void slotFileChoose();
private:
    CompressView *m_pCompressView = nullptr;    // 压缩列表
    CustomPushButton *m_pNextBtn = nullptr;       // 下一步
    bool m_bRootIndex = true;                  // 是否为根目录
};

#endif // COMPRESSPAGE_H
