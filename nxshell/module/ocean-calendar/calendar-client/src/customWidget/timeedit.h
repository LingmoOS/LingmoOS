// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "ccustomtimeedit.h"

#include <DComboBox>
#include <DLineEdit>
#include <DListWidget>

#include <QTime>
#include <QTimeEdit>

DWIDGET_USE_NAMESPACE

class CTimeEdit : public DComboBox
{
    Q_OBJECT
public:
    explicit CTimeEdit(QWidget *parent = nullptr);
    ~CTimeEdit() override;

    /**
     * @brief setMineTime   设置下拉列表顶端时间
     * @param mineTime
     */
    void setMineTime(const QTime &mineTime);

    /**
     * @brief setTime       设置显示时间
     * @param time
     */
    void setTime(const QTime &time);

    /**
     * @brief getTime       获取时间
     * @return
     */
    QTime getTime();

    /**
     * @brief updateListItem        更新列表项
     * @param isShowTimeInterval     是否显示时间间隔
     */
    void updateListItem(bool isShowTimeInterval = false);

    /**
     * @brief setSelectItem     设置选中项目和scroll定位位置
     * @param time
     */
    void setSelectItem(const QTime &time);
public slots:
    void setTimeFormat(int value);
    //焦点获取效果绘制
    void slotFocusDraw(bool showFocus);


    /**
     * @brief slotSetPlainText      设置当前编辑框显示文本
     * @param arg
     */
    void slotSetPlainText(const QString &arg);

    /**
     * @brief slotActivated         Combobox选择项处理槽
     * @param arg
     */
    void slotActivated(const QString &arg);

    /**
     * @brief slotEditingFinished       编辑框编辑结束时，处理槽
     */
    void slotEditingFinished();
signals:
    void signalFocusOut();
    void signaleditingFinished();
private:
    void initUI();
    void initConnection();
protected:
    void showPopup() override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
private:
    QString m_timeFormat = "hh:mm";
    QTime m_time;
    CCustomTimeEdit *m_timeEdit = nullptr;
    bool m_hasFocus;
    QTime m_miniTime;
    bool m_isShowTimeInterval = false;
    QModelIndex scrollPosition;     //下拉列表需要滚动到的位置
};
