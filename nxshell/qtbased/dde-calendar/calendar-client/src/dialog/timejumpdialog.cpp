// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "timejumpdialog.h"
#include "cschedulebasewidget.h"
#include <QHBoxLayout>
#include <QLineEdit>

TimeJumpDialog::TimeJumpDialog(ArrowDirection direction, QWidget *parent) : DArrowRectangle(direction, DArrowRectangle::FloatWindow, parent)
{
    setWindowFlags(Qt::Popup);
    initView();
    initConnect();
}

void TimeJumpDialog::initView()
{
    setRadius(16);  //设置圆角大小
    setArrowWidth(60);  //设置箭头宽度
    setArrowHeight(30); //设置箭头高度
    setRadiusArrowStyleEnable(true);    //设置箭头圆角样式
    setBackgroundColor(DBlurEffectWidget::AutoColor);   //设置背景样式
    setMargin(10);      //设置边距

    m_yearEdit = new CTimeLineEdit(EditYear, this);
    m_monthEdit = new CTimeLineEdit(EditMonth, this);
    m_dayEdit = new CTimeLineEdit(EditDay, this);
    m_jumpButton = new DSuggestButton(tr("Go", "button"), this);

    m_yearEdit->setFixedSize(98, 36);
    m_monthEdit->setFixedSize(88, 36);
    m_dayEdit->setFixedSize(88, 36);
    m_jumpButton->setFixedSize(82, 36);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->setSpacing(10);

    hLayout->addWidget(m_yearEdit);
    hLayout->addWidget(m_monthEdit);
    hLayout->addWidget(m_dayEdit);
    hLayout->addWidget(m_jumpButton);

    QWidget *wgt = new QWidget();
    wgt->setLayout(hLayout);
    setContent(wgt);

}

void TimeJumpDialog::initConnect()
{
    connect(m_yearEdit, &CTimeLineEdit::signalNumChange, this, &TimeJumpDialog::slotEditNumChange);
    connect(m_monthEdit, &CTimeLineEdit::signalNumChange, this, &TimeJumpDialog::slotEditNumChange);
    connect(m_dayEdit, &CTimeLineEdit::signalNumChange, this, &TimeJumpDialog::slotEditNumChange);
    connect(m_yearEdit, &CTimeLineEdit::signalDateJump, this, &TimeJumpDialog::slotDateJump);
    connect(m_monthEdit, &CTimeLineEdit::signalDateJump, this, &TimeJumpDialog::slotDateJump);
    connect(m_dayEdit, &CTimeLineEdit::signalDateJump, this, &TimeJumpDialog::slotDateJump);
    connect(m_jumpButton, &QPushButton::clicked, this, &TimeJumpDialog::slotJumpButtonClicked);
}

void TimeJumpDialog::initData()
{
    m_yearEdit->setRange(1900, 2100);
    m_monthEdit->setRange(1, 12);
    //设置天编辑器限制范围
    m_dayEdit->setRange(1, getMaxDayNum());
    m_yearEdit->setNum(m_date.year());
    m_monthEdit->setNum(m_date.month());
    m_dayEdit->setNum(m_date.day());
    resetEdietStepEnable();
}

/**
 * @brief TimeJumpDialog::showPopup
 * 显示函数
 * @param date 初始显示的日期
 * @param pos 显示位置
 */
void TimeJumpDialog::showPopup(const QDate &date, const QPoint &pos)
{
    showPopup(date, pos.x(), pos.y());
}

/**
 * @brief TimeJumpDialog::showPopup
 * 显示函数
 * @param date 初始显示事件
 * @param x 显示位置x坐标
 * @param y 显示位置y坐标
 */
void TimeJumpDialog::showPopup(const QDate &date, int x, int y)
{
    //保存时间
    m_date = date;
    //初始化数据
    initData();
    //将焦点转移到年编辑器
    m_yearEdit->clearFocus();
    m_monthEdit->clearFocus();
    m_dayEdit->clearFocus();
    //m_yearEdit->selectAll();
    //显示
    show(x, y);
}

/**
 * @brief TimeJumpDialog::getMaxDayNum
 * 获取当月最大天数
 */
int TimeJumpDialog::getMaxDayNum()
{
    static int dayNum[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = dayNum[m_date.month() - 1];
    if (m_date.month() == 2  && QDate::isLeapYear(m_date.year())) {
        maxDay = 29;
    }
    return maxDay;
}

/**
 * @brief TimeJumpDialog::resetEdietStepEnable
 * 重置编辑器状态
 */
void TimeJumpDialog::resetEdietStepEnable()
{
    //将所有编辑器调整为可上下编辑状态
    m_yearEdit->setStepEnabled(CTimeLineEdit::StepUpEnabled | CTimeLineEdit::StepDownEnabled);
    m_monthEdit->setStepEnabled(CTimeLineEdit::StepUpEnabled | CTimeLineEdit::StepDownEnabled);
    m_dayEdit->setStepEnabled(CTimeLineEdit::StepUpEnabled | CTimeLineEdit::StepDownEnabled);

    //根据每一级当前值设置上下编辑状态
    if (m_yearEdit->value() == m_yearEdit->minimum()) {
        m_yearEdit->setStepEnabled(CTimeLineEdit::StepUpEnabled);
        if (m_monthEdit->value() == m_monthEdit->minimum()) {
            m_monthEdit->setStepEnabled(CTimeLineEdit::StepUpEnabled);
            if (m_dayEdit->value() == m_dayEdit->minimum()) {
                m_dayEdit->setStepEnabled(CTimeLineEdit::StepUpEnabled);
            }
        }
    } else if (m_yearEdit->value() == m_yearEdit->maximum()) {
        m_yearEdit->setStepEnabled(CTimeLineEdit::StepDownEnabled);
        if (m_monthEdit->value() == m_monthEdit->maximum()) {
            m_monthEdit->setStepEnabled(CTimeLineEdit::StepDownEnabled);
            if (m_dayEdit->value() == m_dayEdit->maximum()) {
                m_dayEdit->setStepEnabled(CTimeLineEdit::StepDownEnabled);
            }
        }
    }
}

/**
 * @brief TimeJumpDialog::slotEditNumChange
 * 编辑器数字改变事件
 * @param id    编辑器id
 * @param num   编辑器内容改变后的数字
 */
void TimeJumpDialog::slotEditNumChange(int id, int num)
{
    switch (id) {
    case EditYear:
        //更改年
    {
        //因直接设置一年的年月日可能会造成更改月或年后该日期不存在导致的日期错误，
        //所以变化年或者月时先将日变为1，后续再根据实际情况更改日日期
        int day = m_date.day();
        m_date.setDate(num, m_date.month(), 1);
        int maxDay = getMaxDayNum();
        day = day > maxDay ? maxDay : day;
        m_date.setDate(m_date.year(), m_date.month(), day);
        m_dayEdit->setRange(1, maxDay);
    }
    break;
    case EditMonth:
        //更改月
    {
        int day = m_date.day();
        m_date.setDate(m_date.year(), num, 1);
        int maxDay = getMaxDayNum();
        day = day > maxDay ? maxDay : day;
        m_date.setDate(m_date.year(), m_date.month(), day);
        m_dayEdit->setRange(1, maxDay);
    }
    break;
    case EditDay:
        //更改日
        m_date.setDate(m_date.year(), m_date.month(), num);
        break;
    }
    resetEdietStepEnable();
}

/**
 * @brief TimeJumpDialog::slotDateJump
 * 编辑器事件跳转事件
 * @param id 编辑器id
 * @param num 跳转量
 */
void TimeJumpDialog::slotDateJump(int id, int num)
{
    switch (id) {
    case EditYear:
        m_date = m_date.addYears(num);
        break;
    case EditMonth:
        m_date = m_date.addMonths(num);
        break;
    case EditDay:
        m_date = m_date.addDays(num);
        break;
    }
    //初始化数据
    initData();
}

/**
 * @brief TimeJumpDialog::slotJumpButtonClicked
 * 跳转按钮点击事件
 */
void TimeJumpDialog::slotJumpButtonClicked()
{
    //将焦点转移到button控件上，使在跳转时间前正在编辑的edit控件触发finished事件
    m_jumpButton->setFocus();
    //设置全局时间
    CScheduleBaseWidget::setSelectDate(m_date);
    //隐藏控件
    hide();
}
