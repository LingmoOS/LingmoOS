// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscheduleoperation.h"
#include "commondef.h"

#include "schedulectrldlg.h"
#include "configsettings.h"
#include "dcalendarddialog.h"
#include "cdynamicicon.h"
#include "accountmanager.h"
#include "lunarmanager.h"
#include "doanetworkdbus.h"


CScheduleOperation::CScheduleOperation(const AccountItem::Ptr &accountItem, QWidget *parent)
    : QObject(parent)
    , m_accountItem(accountItem)
    , m_widget(parent)
{
    //如果为空默认设置为本地帐户
    if (m_accountItem.isNull()) {
        m_accountItem = gAccountManager->getLocalAccountItem();
    }
}

CScheduleOperation::CScheduleOperation(const QString &scheduleTypeID, QWidget *parent)
    : QObject(parent)
    , m_accountItem(gAccountManager->getAccountItemByScheduleTypeId(scheduleTypeID))
    , m_widget(parent)
{
    //如果为空默认设置为本地帐户
    if (m_accountItem.isNull()) {
        qCWarning(ClientLogger) << "Cannot get account by schedule type,scheduleTypeID:" << scheduleTypeID;
        m_accountItem = gAccountManager->getLocalAccountItem();
    }
}

/**
 * @brief CScheduleOperation::createSchedule        创建日程
 * @param scheduleInfo
 */
bool CScheduleOperation::createSchedule(const DSchedule::Ptr &scheduleInfo)
{
    //如果为农历且重复类型为每年
    if (scheduleInfo->lunnar() && scheduleInfo->getRRuleType() == DSchedule::RRule_Year) {
        lunarMessageDialogShow(scheduleInfo);
    }
    m_accountItem->createSchedule(scheduleInfo);
    return true;
}

/**
 * @brief CScheduleOperation::changeSchedule        修改日程
 * @param newInfo
 * @param oldInfo
 */
bool CScheduleOperation::changeSchedule(const DSchedule::Ptr &newInfo, const DSchedule::Ptr &oldInfo)
{
    bool _result {false};

    if (newInfo->getRRuleType() == DSchedule::RRule_None && newInfo->getRRuleType() == oldInfo->getRRuleType()) {
        //如果为普通日程且没有修改重复类型则更新日程
        if(newInfo->allDay() != oldInfo->allDay()) {
            if(newInfo->allDay()) {
                newInfo->setAlarmType(DSchedule::AlarmType::Alarm_15Hour_Front);
            } else {
                newInfo->setAlarmType(DSchedule::AlarmType::Alarm_15Min_Front);
            }
        }
        m_accountItem->updateSchedule(newInfo);
        _result = true;
    } else {
        //如果切换了全天状态则提醒是否修改全部
        if (newInfo->allDay() != oldInfo->allDay()) {
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(
                tr("All occurrences of a repeating event must have the same all-day status."));
            msgBox.setInformativeText(tr("Do you want to change all occurrences?"));
            msgBox.addPushButton(tr("Cancel", "button"), true);
            msgBox.addWaringButton(tr("Change All"), true);
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                _result = false;
            } else if (msgBox.clickButton() == 1) {
                //更新日程
                showLunarMessageDialog(newInfo, oldInfo);
                m_accountItem->updateSchedule(newInfo);
                _result = true;
            }
        } else if (newInfo->getRRuleType() != oldInfo->getRRuleType()) {
            //修改重复规则
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(tr("You are changing the repeating rule of this event."));
            msgBox.setInformativeText(tr("Do you want to change all occurrences?"));
            msgBox.addPushButton(tr("Cancel", "button"), true);
            msgBox.addWaringButton(tr("Change All"), true);
            msgBox.exec();
            if (msgBox.clickButton() == 0) {
                _result = false;
            } else if (msgBox.clickButton() == 1) {
                //更新日程
                showLunarMessageDialog(newInfo, oldInfo);
                m_accountItem->updateSchedule(newInfo);
                _result = true;
            }
        } else {
            _result = changeRecurInfo(newInfo, oldInfo);
        }
    }
    return  _result;
}

/**
 * @brief CScheduleOperation::deleteSchedule        删除日程
 * @param scheduleInfo
 */
bool CScheduleOperation::deleteSchedule(const DSchedule::Ptr &scheduleInfo)
{
    bool _restuleBool {false};
    //如果为普通日程
    if (scheduleInfo->getRRuleType() == DSchedule::RRule_None) {
        CScheduleCtrlDlg msgBox(m_widget);
        msgBox.setText(tr("You are deleting an event."));
        msgBox.setInformativeText(tr("Are you sure you want to delete this event?"));
        msgBox.addPushButton(tr("Cancel", "button"), true);
        msgBox.addWaringButton(tr("Delete", "button"), true);
        msgBox.exec();
        if (msgBox.clickButton() == 0) {
            return false;
        } else if (msgBox.clickButton() == 1) {
            //            m_DBusManager->DeleteJob(scheduleInfo.getID());
            m_accountItem->deleteScheduleByID(scheduleInfo->uid());
            _restuleBool = true;
        }
    } else {
        //获取原始日程
        DSchedule::Ptr primevalSchedule = m_accountItem->getScheduleByScheduleID(scheduleInfo->uid());
        //如果为重复日程的第一个
        int num = DSchedule::numberOfRepetitions(primevalSchedule, scheduleInfo->dtStart()) ;
        if (num == 1) {
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete all occurrences of this event, or only the selected occurrence?"));
            msgBox.addPushButton(tr("Cancel", "button"));
            msgBox.addPushButton(tr("Delete All"));
            msgBox.addWaringButton(tr("Delete Only This Event"));
            msgBox.exec();
            if (msgBox.clickButton() == 0) {
                return false;
            } else if (msgBox.clickButton() == 1) {
                //删除所有日程
                m_accountItem->deleteScheduleByID(scheduleInfo->uid());
                _restuleBool = true;
            } else if (msgBox.clickButton() == 2) {
                //仅删除此日程
                primevalSchedule->recurrence()->addExDateTime(scheduleInfo->dtStart());
                m_accountItem->updateSchedule(primevalSchedule);
                _restuleBool = true;
            }
        } else {
            CScheduleCtrlDlg msgBox(m_widget);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete this and all future occurrences of this event, or only the selected occurrence?"));
            msgBox.addPushButton(tr("Cancel", "button"));
            msgBox.addPushButton(tr("Delete All Future Events"));
            msgBox.addWaringButton(tr("Delete Only This Event"));
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                return false;
            } else if (msgBox.clickButton() == 1) {
                //删除选中日程及之后的日程
                //修改重复规则
                QList<QDateTime> &&exDt = primevalSchedule->recurrence()->exDateTimes();
                changeRepetitionRule(primevalSchedule, scheduleInfo);
                //如果修改后的日程为普通日程且忽略列表内包含日程开始时间则删除该日程
                if (primevalSchedule->getRRuleType() == DSchedule::RRule_None
                        && exDt.contains(primevalSchedule->dtStart())) {
                    //删除日程
                    m_accountItem->deleteScheduleByID(primevalSchedule->uid());
                } else {
                    //更新日程
                    m_accountItem->updateSchedule(primevalSchedule);
                }

                _restuleBool = true;
            } else if (msgBox.clickButton() == 2) {
                primevalSchedule->recurrence()->addExDateTime(scheduleInfo->dtStart());
                m_accountItem->updateSchedule(primevalSchedule);
                _restuleBool = true;
            }
        }
    }
    return _restuleBool;
}

/**
 * @brief CScheduleOperation::deleteOnlyInfo        删除日程 仅删除此日程 不弹框提醒
 * @param scheduleInfo
 */
void CScheduleOperation::deleteOnlyInfo(const DSchedule::Ptr &scheduleInfo)
{
    //如果为纪念日或节日则不处理
    if (isFestival(scheduleInfo))
        return;
    //如果为普通日程则删除
    if (scheduleInfo->recurs()) {
        //仅删除此日程
        DSchedule::Ptr newschedule = m_accountItem->getScheduleByScheduleID(scheduleInfo->uid());
        newschedule->recurrence()->addExDateTime(scheduleInfo->dtStart());
        m_accountItem->updateSchedule(newschedule);
    } else {
        m_accountItem->deleteScheduleByID(scheduleInfo->uid());
    }
}

/**
 * @brief CScheduleOperation::ChangeRecurInfo       修改重复日程
 * @param newinfo
 * @param oldinfo
 */
bool CScheduleOperation::changeRecurInfo(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo)
{
    bool _result{false};
    //如果为重复类型第一个
    // 获取原始数据
    DSchedule::Ptr primevalScheduleData = m_accountItem->getScheduleByScheduleID(oldinfo->uid());
    int primevalDuration = primevalScheduleData->recurrence()->duration();
    int num = DSchedule::numberOfRepetitions(primevalScheduleData, oldinfo->dtStart());
    if (num == 1) {
        CScheduleCtrlDlg msgBox(m_widget);
        msgBox.setText(tr("You are changing a repeating event."));
        msgBox.setInformativeText(
            tr("Do you want to change only this occurrence of the event, or all "
               "occurrences?"));
        msgBox.addPushButton(tr("Cancel", "button"));
        msgBox.addPushButton(tr("All"));
        msgBox.addsuggestButton(tr("Only This Event"));
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            _result = false;
        } else if (msgBox.clickButton() == 1) {
            //修改所有日程
            //如果此重复日程只有它一个则将修改为普通日程
            DSchedule::Ptr _scheduleDataInfo(newinfo->clone());
            //更新日程
            showLunarMessageDialog(_scheduleDataInfo, oldinfo);
            //如果为非全天且结束于日期则更新结束时间
            if(!_scheduleDataInfo->allDay() && _scheduleDataInfo->recurrence()->duration() ==0){
                QDateTime dtEnd(_scheduleDataInfo->recurrence()->endDateTime().date(),_scheduleDataInfo->dtStart().time());
                _scheduleDataInfo->recurrence()->setEndDateTime(dtEnd);
            }
            m_accountItem->updateSchedule(_scheduleDataInfo);
            _result = true;
        } else if (msgBox.clickButton() == 2) {
            //仅修改此日程
            _result = changeOnlyInfo(newinfo, oldinfo);
        }
    } else {
        CScheduleCtrlDlg msgBox(m_widget);
        msgBox.setText(tr("You are changing a repeating event."));
        msgBox.setInformativeText(
            tr("Do you want to change only this occurrence of the event, or this and "
               "all future occurrences?"));
        msgBox.addPushButton(tr("Cancel", "button"));
        msgBox.addPushButton(tr("All Future Events"));
        msgBox.addsuggestButton(tr("Only This Event"));
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            _result = false;
        } else if (msgBox.clickButton() == 1) {
            // 根据id获取日程并修改
            //修改重复规则

            QList<QDateTime> &&exDt = primevalScheduleData->recurrence()->exDateTimes();
            changeRepetitionRule(primevalScheduleData, oldinfo);
            //如果修改后的日程为普通日程且忽略列表内包含日程开始时间则删除该日程
            if (primevalScheduleData->getRRuleType() == DSchedule::RRule_None
                    &&exDt.contains(primevalScheduleData->dtStart())) {
                //删除日程
                m_accountItem->deleteScheduleByID(primevalScheduleData->uid());
            } else {
                //更新日程
                m_accountItem->updateSchedule(primevalScheduleData);
            }

            //创建日程
            DSchedule::Ptr newschedule(newinfo->clone());

            if (primevalDuration > 0) {
                //如果结束与次数
                int newDuration = primevalDuration - num + 1;
                //如果重复次数大于1
                if (newDuration > 1) {
                    newschedule->recurrence()->setDuration(newDuration);
                } else {
                    newschedule->setRRuleType(DSchedule::RRule_None);
                }
                newschedule->setRecurrenceId(QDateTime());
            } else if (primevalDuration ==0) {
                //结束于日期
                QDateTime dtEnd(newschedule->recurrence()->endDateTime().date(),newschedule->dtStart().time());
                newschedule->recurrence()->setEndDateTime(dtEnd);
            }
            //如果结束为永不和时间则不需要修改

            //创建新日程
            //如果为农历且重复类型为每年
            if (newschedule->lunnar() && DSchedule::RRule_Year == newschedule->getRRuleType()) {
                lunarMessageDialogShow(newschedule);
            }
            m_accountItem->createSchedule(newschedule);
            _result = true;
        } else if (msgBox.clickButton() == 2) {
            _result = changeOnlyInfo(newinfo, oldinfo);
        }
    }
    return _result;
}

/**
 * @brief CScheduleOperation::ChangeOnlyInfo        修改重复日程,仅修改此日程
 * @param newinfo
 * @param oldinfo
 */
bool CScheduleOperation::changeOnlyInfo(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo)
{
    //修改日程
    DSchedule::Ptr newschedule(newinfo->clone());
    newschedule->setRRuleType(DSchedule::RRule_None);
    newschedule->setRecurrenceId(QDateTime());
    //创建日程
    m_accountItem->createSchedule(newschedule);

    //获取原始信息
    DSchedule::Ptr updatescheduleData = m_accountItem->getScheduleByScheduleID(oldinfo->uid());
    updatescheduleData->recurrence()->addExDateTime(oldinfo->dtStart());
    //更新原始信息
    m_accountItem->updateSchedule(updatescheduleData);
    return true;
}

/**
 * @brief CScheduleOperation::changeRepetitionRule      修改日程重复规则
 * @param newinfo
 * @param oldinfo
 */
void CScheduleOperation::changeRepetitionRule(DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo)
{
    int num = DSchedule::numberOfRepetitions(newinfo, oldinfo->dtStart());
    //修改重复规则
    if (newinfo->recurrence()->duration() > 0) {
        //如果为结束与次数则修改结束次数
        int duration = num - 1;
        if (duration > 1) {
            newinfo->recurrence()->setDuration(duration);
        } else {
            //结束次数小于等于0表示不重复，设置为普通日程
            newinfo->setRRuleType(DSchedule::RRule_None);
        }
    } else {
        //如果该日程结束类型为永不和结束于日期则修改结束日期
        newinfo->recurrence()->setDuration(0);
        QDateTime dtEnd(oldinfo->dtStart().addDays(-1));
        newinfo->recurrence()->setEndDateTime(dtEnd);
    }
}

void CScheduleOperation::lunarMessageDialogShow(const DSchedule::Ptr &newinfo)
{
    //如果该日程为闰月日程，因为对应的闰月需要间隔好多年，所以添加对应的提示信息
    CaHuangLiDayInfo huangLiInfo = gLunarManager->getHuangLiDay(newinfo->dtStart().date());;
    if (huangLiInfo.mLunarMonthName.contains("闰")) {
        DCalendarDDialog prompt(m_widget);
        prompt.setIcon(QIcon(CDynamicIcon::getInstance()->getPixmap()));
        prompt.setDisplayPosition(DAbstractDialog::Center);
        prompt.setMessage(tr("You have selected a leap month, and will be reminded according to the rules of the lunar calendar."));
        prompt.addButton(tr("OK", "button"), true, DDialog::ButtonNormal);
        if (m_widget) {
            //获取父窗口的中心坐标
            const QPoint global = m_widget->mapToGlobal(m_widget->rect().center());
            //相对父窗口居中显示
            prompt.move(global.x() - prompt.width() / 2, global.y() - prompt.height() / 2);
        }
        prompt.exec();
    }
}

void CScheduleOperation::showLunarMessageDialog(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo)
{
    //在阴历每年重复情况下如果修改了开始时间或重复规则
    //农历日程重复每年闰月是否提示
    if (newinfo->lunnar() && DSchedule::RRule_Year == newinfo->getRRuleType()) {
        if (oldinfo->dtStart().date() != newinfo->dtStart().date()
            || oldinfo->getRRuleType() != newinfo->getRRuleType()
            || oldinfo->lunnar() != newinfo->lunnar()) {
            //判断是否为闰月
            lunarMessageDialogShow(newinfo);
        }
    }
}


bool CScheduleOperation::isFestival(const DSchedule::Ptr &schedule)
{
    //判断是否为节假日日程
    AccountItem::Ptr account = gAccountManager->getAccountItemByScheduleTypeId(schedule->scheduleTypeID());
    if (account.isNull()) {
        qCWarning(ClientLogger) << "Cannot get account by schedule type,scheduleTypeID:" << schedule->scheduleTypeID();
        return false;
    }
    DScheduleType::Ptr scheduleType = gAccountManager->getScheduleTypeByScheduleTypeId(schedule->scheduleTypeID());
    //如果为本地日程且日程类型为None则表示为节假日日程
    return account->getAccount()->accountType() == DAccount::Account_Local && scheduleType->privilege() == 0;
}

bool CScheduleOperation::scheduleIsInvariant(const DSchedule::Ptr &schedule)
{
    //如果为网络帐户，且没有网络或者帐户开关关闭

    AccountItem::Ptr accountItem = gAccountManager->getAccountItemByScheduleTypeId(schedule->scheduleTypeID());
    if (accountItem.isNull())
        return false;
    DAccount::Ptr account = accountItem->getAccount();
    if (account->accountType() == DAccount::Account_UnionID) {
        DOANetWorkDBus netManger;
        //网络判断
        //如果uid日历同步关闭则日程不可修改
        if (netManger.getNetWorkState() != DOANetWorkDBus::Active
                || !account->accountState().testFlag(DAccount::Account_Open)
                || !account->accountState().testFlag(DAccount::Account_Calendar)) {
            return true;
        }
    }
    return false;
}
