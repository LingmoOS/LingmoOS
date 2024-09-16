// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef INFOSHOWWIDGET_H
#define INFOSHOWWIDGET_H

#include "customcontrol/dmtreeviewdelegate.h"

#include <DWidget>
#include <DLabel>
#include <DFrame>
#include <DProgressBar>
#include <DArrowRectangle>

class DmFrameWidget;
class SizeInfoWidget;
class InfoTopFrame;
class PartitionInfoWidget;
class VGSizeInfoWidget;
class VGInfoShowWidget;

DWIDGET_USE_NAMESPACE

/**
 * @class InfoShowWidget
 * @brief 分区信息展示类
 */

class InfoShowWidget : public DFrame
{
    Q_OBJECT
public:
    explicit InfoShowWidget(DWidget *parent = nullptr);

private:

    /**
     * @brief 设置显示容量大小窗口（右侧页面中间部分）
     */
    void midFramSettings();

    /**
     * @brief 设置分区详细信息窗口（右侧页面下面表格部分）
     */
    void bottomFramSettings();

    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化连接
     */
    void initConnection();

private slots:
    /**
     * @brief 当前选择分区信号响应的槽函数
     */
    void onCurSelectChanged();

    /**
     * @brief 主题切换信号响应的槽函数
     */
    void onHandleChangeTheme();

    /**
     * @brief 首页磁盘分区图示进入响应的槽函数
     * @param rect 分区图示坐标
     * @param path 分区路径
     */
    void onEnterWidget(QRectF rect, QString path);

    /**
     * @brief 首页磁盘分区图示离开响应的槽函数
     */
    void onLeaveWidget();

    /**
     * @brief 首页VG信息图示进入响应的槽函数
     */
    void onEnterVGInfoWidget(QRect rect, const QList< QMap<QString, QVariant> > &lstInfo);

private:
    InfoTopFrame *m_infoTopFrame = nullptr;
    DFrame *m_frame = nullptr;
    DFrame *m_frameMid = nullptr;
    DmFrameWidget *m_frameBottom = nullptr;
    SizeInfoWidget *m_infoWidget = nullptr;
    double m_used;
    double m_noused;
    DiskInfoData m_diskInfoData;
    QColor fillcolor;
    QColor fillcolor1;

    DLabel *m_mountpointLabel;
    DLabel *m_freeLabel;
    DLabel *m_usedLabel;
    DLabel *m_typeLabel;
    DLabel *m_capacityLabel;
    DLabel *m_volumeLabel;

    PartitionInfoWidget *m_partitionInfoWidget;
    DArrowRectangle *m_arrowRectangle;
    DLabel *m_pathLabel;

    VGSizeInfoWidget *m_vgSizeInfoWidget;
    VGInfoShowWidget *m_vgInfoShowWidget;
};

#endif // INFOSHOWWIDGET_H
