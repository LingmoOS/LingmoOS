// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "settingWidget/settingwidgets.h"
#include "doanetworkdbus.h"
#include "controlCenterProxy.h"
#include <DSettingsDialog>
#include <DIconButton>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class CSettingDialog : public DSettingsDialog
{
    Q_OBJECT
public:
    explicit CSettingDialog(QWidget *parent = nullptr);

private:
    QPair<QWidget*, QWidget*> createFirstDayofWeekWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createTimeTypeWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createAccountCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncFreqCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncTagRadioButton(QObject *obj);
    QWidget *createManualSyncButton(QObject *obj);
    QWidget *createJobTypeListView(QObject *obj);
    DIconButton *createTypeAddButton();
    QWidget *createControlCenterLink(QObject *obj);

public slots:
    void slotGeneralSettingsUpdate();
    void slotAccountUpdate();
    void slotLogout(DAccount::Type);
    void slotLastSyncTimeUpdate(const QString &datetime);
    //帐户状态发送改变
    void slotAccountStateChange();

    void slotFirstDayofWeekCurrentChanged(int index);
    void slotTimeTypeCurrentChanged(int index);
    void slotAccountCurrentChanged(int index);
    void slotTypeAddBtnClickded();
    void slotTypeImportBtnClickded();
    void slotSetUosSyncFreq(int freq);
    void slotUosManualSync();
    void slotNetworkStateChange(DOANetWorkDBus::NetWorkState state);
    //更新同步项按钮状态
    void slotSyncTagButtonUpdate();
    //点击同步项时，更新uos账户状态
    void slotSyncAccountStateUpdate(bool);

private:
    void initFirstDayofWeekWidget();
    void initTimeTypeWidget();
    void initAccountComboBoxWidget();
    void initTypeAddWidget();
    void initScheduleTypeWidget();
    void initSyncFreqWidget();
    void initManualSyncButton();

    void setFirstDayofWeek(int value);
    void setTimeType(int value);
    void accountUpdate();

    void setTypeEnable(int index);
private:
    void initWidget();
    void initConnect();
    void initData();

    void initWidgetDisplayStatus();
    void initView();

private:
    //一周首日
    QWidget *m_firstDayofWeekWidget = nullptr;
    QComboBox *m_firstDayofWeekCombobox= nullptr;

    //时间格式
    QWidget *m_timeTypeWidget = nullptr;
    QComboBox *m_timeTypeCombobox = nullptr;

    //帐户选择
    QComboBox *m_accountComboBox = nullptr;
    //同步频率
    QComboBox *m_syncFreqComboBox = nullptr;

    DIconButton *m_typeAddBtn = nullptr;
    DIconButton *m_typeImportBtn = nullptr;

    JobTypeListView *m_scheduleTypeWidget = nullptr;

    //手动同步按钮和同步时间显示
    QLabel *m_syncTimeLabel = nullptr;
    QPushButton *m_syncBtn = nullptr;
    QWidget *m_manualSyncWidget = nullptr;
    DOANetWorkDBus *m_ptrNetworkState;
    SettingWidget::SyncTagRadioButton *m_radiobuttonAccountCalendar = nullptr;
    SettingWidget::SyncTagRadioButton *m_radiobuttonAccountSetting = nullptr;
    ControlCenterProxy *m_controlCenterProxy;
};

#endif // SETTINGDIALOG_H
