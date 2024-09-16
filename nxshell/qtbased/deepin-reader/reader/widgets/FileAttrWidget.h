// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEATTRWIDGET_H
#define FILEATTRWIDGET_H

#include <DAbstractDialog>
#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class DocSheet;
class QVBoxLayout;
class ImageWidget;

/**
 * @brief The FileAttrWidget class
 * 文件信息窗口
 */
class FileAttrWidget : public DAbstractDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(FileAttrWidget)

public:
    explicit FileAttrWidget(DWidget *parent = nullptr);

    ~FileAttrWidget() override;

    /**
     * @brief setFileAttr
     * 设置文档对象
     * @param sheet
     */
    void setFileAttr(DocSheet *sheet);

    /**
     * @brief showScreenCenter
     * 居中显示
     */
    void showScreenCenter();

private:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

    /**
     * @brief initCloseBtn
     * 初始化关闭按钮
     */
    void initCloseBtn();

    /**
     * @brief initImageLabel
     * 初始化图片显示控件
     */
    void initImageLabel();

    /**
     * @brief addTitleFrame
     * 添加标题控件
     */
    void addTitleFrame(const QString &);

private:
    QVBoxLayout *m_pVBoxLayout = nullptr;
    ImageWidget *frameImage = nullptr;
};

#endif  // FILEATTRWIDGET_H
