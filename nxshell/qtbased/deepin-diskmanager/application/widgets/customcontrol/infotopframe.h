// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef INFOTOPFRAME_H
#define INFOTOPFRAME_H

#include <DFrame>
#include <DWidget>
#include <DLabel>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

/**
 * @class InfoTopFrame
 * @brief 分区信息上部分实现类
 */

class InfoTopFrame : public DFrame
{
    Q_OBJECT
public:
    explicit InfoTopFrame(DWidget *parent = nullptr);

public:
    /**
     * @brief 更新界面显示
     */
    void updateDiskInfo();

protected:
    /**
     * @brief 字号变更事件
     */
    void resizeEvent(QResizeEvent *event);

private:
    /**
     * @brief 初始化左侧信息
     */
    void initLeftInfo();

    /**
     * @brief 初始化右侧信息
     */
    void initRightInfo();

private:
    QHBoxLayout *m_mainLayout = nullptr;
    DLabel *m_pictureLabel = nullptr;
    DLabel *m_nameLabel = nullptr;
    DLabel *m_typeLabel = nullptr;
    DLabel *m_allNameLabel = nullptr;
    DLabel *m_allMemoryLabel = nullptr;
};

#endif // INFOTOPFRAME_H
