// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEDLG_H
#define SCHEDULEDLG_H

#include "dschedule.h"
#include "dcalendarddialog.h"
#include "cdateedit.h"
#include "jobtypecombobox.h"
#include "colorseletorwidget.h"

#include <DCheckBox>
#include <DDateEdit>
#include <DFrame>
#include <DLineEdit>
#include <DTextEdit>
#include <DComboBox>

#include <QLabel>
#include <QButtonGroup>

DWIDGET_USE_NAMESPACE
class CTimeEdit;
class CScheduleDlg : public DCalendarDDialog
{
    Q_OBJECT
public:

    enum ButtonId {
        RadioSolarId = 0, //公历选择按钮id
        RadioLunarId      //农历选择按钮id
    };

    CScheduleDlg(int type, QWidget *parent = nullptr, const bool isAllDay = true);
    ~CScheduleDlg() override;
    void setData(const DSchedule::Ptr &info);
    void setDate(const QDateTime &date);
    void setAllDay(bool flag);
private:
    //确定按钮处理
    bool clickOkBtn();
    //选择日程类型
    bool selectScheduleType();
    //创建日程
    bool createSchedule(const QString& scheduleTypeId);

    /**
     * @brief updateEndTimeListAndTimeDiff      更新结束时间（time），结束时间下拉列表和开始时间和结束时间差
     * @param begin                             开始时间（DateTime）
     * @param end                               结束时间 (DateTime）
     */
    void updateEndTimeListAndTimeDiff(const QDateTime &begin, const QDateTime &end);

    /**
     * @brief updateEndTimeList             更新结束时间下拉列表
     * @param begin                         开始时间(time)
     * @param isShowTimeInterval            是否显示时间偏移
     */
    void updateEndTimeList(const QTime &begin, bool isShowTimeInterval);
signals:
    void signalScheduleUpdate(int id = 0);
public slots:
    /**
     * @brief 开始时间改变
     *
     */
    void slotBeginTimeChange();

    /**
     * @brief 结束时间改变
     *
     */
    void slotEndTimeChange();

    /**
     * @brief 结束日期改变
     *
     */
    void slotEndDateChange(const QDate &date);
    /**
     * @brief slotRadioBtnClicked
     * 选择按钮控件点击时间
     * @param btnId 控件id
     */
    void slotRadioBtnClicked(int btnId);
    /**
     * @brief slotBtnAddItemClicked
     * 添加类型事件
     */
    void slotBtnAddItemClicked();
    /**
     * @brief slotTypeEditTextChanged
     * 类型下拉选择框文本改变事件
     */
    void slotTypeEditTextChanged(const QString &);
    void slotTypeRpeatactivated(int index);
    //对话框按钮点击处理
    void slotBtClick(int buttonIndex, const QString &buttonName);
    void slotTextChange();
    void slotendrepeatTextchange();
    void slotBDateEidtInfo(const QDate &date);
    void slotallDayStateChanged(int state);
    void slotbRpeatactivated(int index);
    void sloteRpeatactivated(int index);
    void slotJobComboBoxEditingFinished();
    void slotAccoutBoxActivated(const QString &text);
    //帐户登出信号
    void signalLogout(DAccount::Type);
    //帐户信息更新
    void slotAccountUpdate();
    //帐户状态
    void slotAccountStateChange();

protected:
    bool eventFilter(QObject *obj, QEvent *pEvent) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void updateDateTimeFormat() override;

private:
    void initUI();
    void resetColor(const AccountItem::Ptr&);
    void initConnection();
    void initDateEdit();
    void initJobTypeComboBox();
    void initRmindRpeatUI();
    /**
     * @brief setTheMe  根据主题type设置颜色
     * @param type  主题type
     */
    void setTheMe(const int type);
    //设置tab顺序
    void setTabFouseOrder();

    /**
     * @brief updateIsOneMoreDay        更新开始时间与结束时间标识是否超过一天
     * @param begin                     开始时间(datetime)
     * @param end                       结束时间(datetime)
     */
    void updateIsOneMoreDay(const QDateTime &begin, const QDateTime &end);
    /**
     * @brief updateRepeatCombox
     * 更新重复下拉列表
     * @param isLunar   是否是农历 true：是 false：不是
     */
    void updateRepeatCombox(bool isLunar = false);
    /**
     * @brief isShowLunar
     * 系统语言检查，根据语言类型判断是否显示农历信息
     */
    bool isShowLunar();

    /**
     * @brief setShowState      设置显示状态
     * @param jobIsLunar        日程是否为农历
     */
    void setShowState(bool jobIsLunar);

    void setWidgetEnabled(bool isEnabled);

    void resize();

    /**
     * @brief setOkBtnEnabled   根据选项设置保存按钮是否有效
     */
    void setOkBtnEnabled();

private:

    DComboBox *m_accountComBox = nullptr;   //帐户下拉选择框

    QLabel *m_typeLabel = nullptr;
    //DComboBox *m_typeComBox = nullptr;
    JobTypeComboBox *m_typeComBox = nullptr;
    ColorSeletorWidget *m_colorSeletorWideget = nullptr; //颜色选择器
    QLabel *m_contentLabel = nullptr;
    DTextEdit *m_textEdit = nullptr;
    QLabel *m_beginTimeLabel = nullptr;
    CDateEdit *m_beginDateEdit = nullptr;
    CTimeEdit *m_beginTimeEdit = nullptr;
    QLabel *m_endTimeLabel = nullptr;
    CDateEdit *m_endDateEdit = nullptr;
    CTimeEdit *m_endTimeEdit = nullptr;

    QLabel *m_adllDayLabel = nullptr;
    DCheckBox *m_allDayCheckbox = nullptr;
    QLabel *m_remindSetLabel = nullptr;
    DComboBox *m_rmindCombox = nullptr;
    QLabel *m_beginrepeatLabel = nullptr;
    DComboBox *m_beginrepeatCombox = nullptr;
    QLabel *m_endrepeatLabel = nullptr;
    DComboBox *m_endrepeatCombox = nullptr;
    DLineEdit *m_endrepeattimes = nullptr;
    QLabel *m_endrepeattimesLabel = nullptr;
    DWidget *m_endrepeattimesWidget;
    CDateEdit *m_endRepeatDate = nullptr;
    DWidget *m_endrepeatWidget = nullptr;
    DFrame *m_gwi = nullptr;
    QLabel *m_titleLabel = nullptr;
    QButtonGroup *m_calendarCategoryRadioGroup = nullptr;    //日历类别选择控件组
    DRadioButton *m_solarRadioBtn = nullptr;      //公历选择按钮
    DRadioButton *m_lunarRadioBtn = nullptr;        //农历选择按钮

    QString m_context;
    QString m_TypeContext;  //类型输入框上一次输入后的字符
    const bool m_createAllDay;
    bool m_setAccept {false}; //是否设置返回代码为Rejected
    qint64 m_timeDiff = 0;             //开始时间和结束时间差，不算日期
    bool m_isMoreThenOneDay = false;
private:
    //日程
    DSchedule::Ptr m_scheduleDataInfo;
    AccountItem::Ptr m_accountItem;
    int m_type; // 1新建 0 编辑日程
    QDateTime m_currentDate;
    QDateTime m_EndDate;
    bool m_typeEditStatus = false; //日程类型编辑状态
    int m_prevCheckRadioID = -1; //上一次点击Radio的id编号
    bool m_bCanCreateType = true;;
};

#endif // SCHEDULEDLG_H
