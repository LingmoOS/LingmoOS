// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef CYLINDERINFOWIDGET_H
#define CYLINDERINFOWIDGET_H

#include <DFrame>
#include <DArrowRectangle>
#include <DScrollBar>

#include <QWidget>
#include <QSettings>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class QGridLayout;

/**
 * @class CylinderInfoWidget
 * @brief 柱面检测信息展示窗口类
 */

class CylinderInfoWidget : public DFrame
{
    Q_OBJECT
public:
    explicit CylinderInfoWidget(int cylNumber, QWidget *parent = nullptr);

    /**
     * @brief 设置将要检测的柱面个数
     * @param cylNumber 柱面个数
     */
    void setCylinderNumber(int cylNumber);

    /**
     * @brief 当前检测坏道信息
     * @param cylinderNumber 柱面号
     * @param cylinderTimeConsuming 柱面耗时
     * @param cylinderStatus 柱面状态
     * @param cylinderErrorInfo 当前检测错误信息
     */
    void setCurCheckBadBlocksInfo(const QString &LBANumber, const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo);

    /**
     * @brief 重新检测
     * @param cylNumber 柱面个数
     */
    void againVerify(int cylNumber);

    /**
     * @brief 复位
     * @param cylNumber 柱面个数
     */
    void reset(int cylNumber);

    /**
     * @brief 当前修复坏道信息
     * @param cylinderNumber 柱面号
     */
    void setCurRepairBadBlocksInfo(const QString &cylinderNumber);

    /**
     * @brief 设置检测状态
     * @param isCheck 是否正在检测
     */
    void setChecked(bool isCheck);

signals:
    /**
     * @brief 检测完成信号
     * @param badSectorsCount 坏道个数
     */
//    void checkCoomplete(int badSectorsCount);

public slots:

protected:
    /**
     * @brief 滚轮滑动响应事件
     */
    void wheelEvent(QWheelEvent *event);

private slots:
    /**
     * @brief 鼠标进入柱面方块响应的槽函数
     */
    void enterSlot();

    /**
     * @brief 鼠标离开柱面方块响应的槽函数
     */
    void leaveSlot();

    /**
     * @brief 滚动条数值改变响应的槽函数
     * @param value 滚动条当前数值
     */
    void onScrollBarValueChanged(int value);

private:
    /**
     * @brief 初始化界面
     */
    void initUI();

    /**
     * @brief 初始化信号连接
     */
    void initConnections();

    /**
     * @brief 更新柱面状态以及检测信息
     * @param number 当前需要更新的方块号
     * @param LBANumber 柱面LBA名称
     * @param cylinderNumber 柱面号
     * @param cylinderTimeConsuming 柱面耗时
     * @param cylinderStatus 柱面状态
     * @param cylinderErrorInfo 柱面检测错误信息
     * @param repair 是否是已修复，1代表已修复
     */
    void updateCylinderInfo(int number, const QString &LBANumber, const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo, const QString &repair);

private:
    int m_cylNumber;
    QWidget *m_widget;
    QGridLayout *m_gridLayout;
    DArrowRectangle *m_arrowRectangle;
    QLabel *m_LBALabel;
    QLabel *m_cylNumberLabel;
    QLabel *m_curErrorInfoLabel;
    QLabel *m_repairLabel;
    QLabel *m_elapsedTimeLabel;
    DScrollBar *m_scrollBar;
    QString m_initColor;
    QString m_excellentColor;
    QString m_damagedColor;
    QString m_unknownColor;
    int m_curCheckCount;
    int m_badSectorsCount;
    QSettings *m_settings;
    bool m_isChanged = false;
    bool m_isCheck = false;
    QStringList m_checkData;
    QStringList m_cylNumberData;
    int m_startCylinder;
    int m_endCylinder;
};

#endif // CYLINDERINFOWIDGET_H
