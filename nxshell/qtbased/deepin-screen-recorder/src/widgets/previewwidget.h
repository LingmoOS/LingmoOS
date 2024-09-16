// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCANPAGEWIDGET_H
#define SCANPAGEWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <qglobal.h>

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    //预览位置
    enum PostionStatus {
        RIGHT = 0,
        LEFT,
        INSIDE
    };
    explicit PreviewWidget(const QRect &rect, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    //更新图片
    void updateImage(const QImage &image);
    //标记预览位置在左还是在右，0:右，1：左, 2内部
    void setPreviewWidgetStatusPos(PostionStatus statusPos);
    //计算预览位置或大小
    QRect previewGeomtroy();
    //计算预览位置
    QRect calculatePreviewPosition(int previewWidth, int previewHeight);
    //设置屏幕宽度
    void setScreenInfo(int screenWidth, double screenRatio);
    //初始化预览位置大小
    void initPreviewWidget();

    /**
     * @brief 当捕捉区域发生改变时，用来更新预览图的位置
     * @param rect 捕捉区域的大小及位置
     * 自动调整捕捉区域时，捕捉区域的位置及大小会改变
     */
    void updatePreviewSize(const QRect &rect);

    /**
     * @brief 获取预览框相对于捕捉区域的位置；
     */
    PostionStatus getPreviewPostion();
signals:

public slots:

private:
    QImage m_currentPix;
    QRect m_previewRect; //预览区域
    int m_maxHeight = 0; //预览最大高度
    int m_maxWidth = 0;//预览最大宽度
    PostionStatus m_StatusPos = RIGHT; //位置状态

    int m_recordHeight = 0; //捕捉区域高度
    int m_recordWidth = 0; //捕捉区域宽度
    int m_recordX = 0;//捕捉区域x坐标
    int m_recordY = 0;//捕捉区域y坐标
    int m_screenWidth = 0;//屏幕宽度
    double m_screenRatio = 1.0;//屏幕比率

};

#endif // SCANPAGEWIDGET_H
