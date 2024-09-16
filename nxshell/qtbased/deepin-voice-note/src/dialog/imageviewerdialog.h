// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEEPIN_MANUAL_VIEW_WIDGETS_IMAGE_VIEWER_H
#define DEEPIN_MANUAL_VIEW_WIDGETS_IMAGE_VIEWER_H

#include <DLabel>
#include <DDialogCloseButton>

#include <QDialog>

DWIDGET_USE_NAMESPACE

/**
 * @brief ImageViewer::ImageViewer
 * @param parent
 * 点击图片后展示全屏的图片
 */
class ImageViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewerDialog(QWidget *parent = nullptr);
    ~ImageViewerDialog() override;

public slots:
    /**
     * @brief 加载图片并显示
     * @param filepath 图片路径
     */
    void open(const QString &filepath);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief 初始化ui
     */
    void initUI();

    using QDialog::open;

private:
    DLabel *m_imgLabel = nullptr; //图片显示控件
    DDialogCloseButton *m_closeButton; //关闭按钮控件
    int m_imgWidth = 0; //图片实际宽度
    int m_imgHeight = 0; //图片实际高度
};

#endif // DEEPIN_MANUAL_VIEW_WIDGETS_IMAGE_VIEWER_H
