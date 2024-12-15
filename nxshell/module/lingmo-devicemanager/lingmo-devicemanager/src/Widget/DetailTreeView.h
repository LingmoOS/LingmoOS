// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILTREEVIEW_H
#define DETAILTREEVIEW_H

#include <QStandardItem>

#include <DTreeView>
#include <DWidget>
#include <DCommandLinkButton>
#include <DTableView>
#include <DTableWidget>

DWIDGET_USE_NAMESPACE

class DetailViewDelegate;
class ButtonDelegate;
class CmdButtonWidget;
class TipsWidget;

/**
 * @brief The BtnWidget class
 * 控件：按钮
 */
class BtnWidget: public DWidget
{
    Q_OBJECT
public:
    BtnWidget();
protected:
    /**
     * @brief enterEvent:鼠标进入事件
     * @param event
     */
    void enterEvent(QEvent *event)override;

    /**
     * @brief leaveEvent:鼠标移出事件
     * @param event
     */
    void leaveEvent(QEvent *event)override;
signals:
    /**
     * @brief enter:信号,鼠标进入
     */
    void enter();

    /**
     * @brief leave:信号,鼠标移出
     */
    void leave();
};

/**
 * @brief The DetailTreeView class
 * 封装的表格
 */
class DetailTreeView: public DTableWidget//DTreeView
{
    Q_OBJECT
public:
    explicit DetailTreeView(DWidget *parent = nullptr);

    /**
     * @brief setColumnAndRow:设置表格行数和列数
     * @param row:表格行数
     * @param column:表格列数
     */
    void setColumnAndRow(int row, int column = 2);

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, QTableWidgetItem *item);

    /**
     * @brief clear : 清空数据
     */
    void clear();

    /**
     * @brief setCommanLinkButton:设置扩展按钮属性，及布局
     * @param row:扩展按钮在表格中的行数
     */
    void setCommanLinkButton(int row);

    /**
     * @brief setTableHeight:设置表格高度
     * @return 重绘矩形的高度
     */
    int setTableHeight(int paintHeight);

    /**
     * @brief hasExpendInfo:是否由详细信息按钮
     * @return 布尔值：true有详细信息按钮，false没有xiangzx详细信息按钮
     */
    bool hasExpendInfo();

    /**
     * @brief setLimitRow:设置表格最多显示行
     * @param row：行数，默认13
     */
    void setLimitRow(int row = 13);

    /**
     * @brief toString:
     * @return
     */
    QString toString();

    /**
     * @brief isCurDeviceEnable:当前设备是否启用
     * @return true:启用，false:禁用
     */
    bool isCurDeviceEnable();

    /**
     * @brief isCurDeviceAvailable 判断设备是否可用，通过设备的驱动是否被加载
     * @return
     */
    bool isCurDeviceAvailable();

    /**
     * @brief setCurDeviceState:设置当前设备状态
     * @param state true:启用;false:禁用
     */
    void setCurDeviceState(bool enable, bool available);

    /**
     * @brief isExpanded 判断是否展开
     * @return 是否展开
     */
    bool isExpanded();

public slots:
    /**
     * @brief expandCommandLinkClicked:展开button点击槽函数
     */
    void expandCommandLinkClicked();

protected:
    /**
     * @brief initUI:初始化界面
     */
    void initUI();

    /**
     * @brief paintEvent
     * @param event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief resizeEvent
     * @param event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief mousePressEvent
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief mouseMoveEvent
     * @param event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief leaveEvent
     * @param event
     */
    void leaveEvent(QEvent *event)override;

private slots:
    /**
     * @brief slotTimeOut
     */
    void slotTimeOut();

    /**
     * @brief slotItemEnterd
     * @param item
     */
    void slotItemEnterd(QTableWidgetItem *item);

    /**
     * @brief slotEnterBtnWidget
     */
    void slotEnterBtnWidget();

    /**
     * @brief slotLeaveBtnWidget
     */
    void slotLeaveBtnWidget();

private:
    /**
     * @brief showTips
     * @param txt
     */
    void showTips(QTableWidgetItem *item);

    /**
     * @brief initBtnWidget
     */
    void initBtnWidget();

private:
    DetailViewDelegate        *mp_ItemDelegate;   // Item自定义代理
    DCommandLinkButton        *mp_CommandBtn;     // 展开命令Button
    int                       m_LimitRow;         // 正常状态下，表格显示的行数
    bool                      m_IsExpand;         // 是否展开
    bool                      m_IsEnable;         // 是否启用
    bool                      m_IsAvailable;
    QTableWidgetItem          *mp_OldItem;
    QTableWidgetItem          *mp_CurItem;
    qint64                    m_TimeStep;
    QTimer                    *mp_Timer;
    QPoint                    mp_Point;
    TipsWidget                *mp_ToolTips;
};

#endif // DETAILTREEVIEW_H
