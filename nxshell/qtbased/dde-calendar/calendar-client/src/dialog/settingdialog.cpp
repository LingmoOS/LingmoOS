// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingdialog.h"
#include "cdynamicicon.h"
#include "accountmanager.h"
#include "commondef.h"
#include "dcalendargeneralsettings.h"
#include "settingWidget/userloginwidget.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "units.h"

#include <QSpacerItem>

#include <DComboBox>
#include <DSysInfo>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DBackgroundGroup>
#include <DHiDPIHelper>

#include <qglobal.h>
#include <qloggingcategory.h>

const QString ControlCenterDBusName = "org.deepin.dde.ControlCenter1";
const QString ControlCenterDBusPath = "/org/deepin/dde/ControlCenter1";
const QString ControlCenterPage = "datetime/region";

using namespace SettingWidget;
//静态的翻译不会真的翻译，但是会更新ts文件
//像static QString a = QObject::tr("hello"), a实际等于hello，但是ts会有hello这个词条
//调用DSetingDialog时会用到上述场景
static CalendarSettingSetting setting_account = {
    "setting_account",
    QObject::tr("Account settings"),
    {
        {
            "account",
            QObject::tr("Account"),
            {{
                    "login", //key
                    "", //name
                    "login", //type
                    "" //default
                }
            }
        },
        {
            "account_sync_items",
            QObject::tr("Select items to be synced"),
            {{
                    "Account_Calendar", //key
                    QObject::tr("Events"), //name
                    "SyncTagRadioButton", //type
                    "" //default
                },

                {
                    "Account_Setting", //key
                    QObject::tr("General settings"), //name
                    "SyncTagRadioButton", //type
                    "" //default
                }
            }
        },
        {
            "sync_interval",
            "",
            {   {
                    "Sync_interval", //key
                    QObject::tr("Sync interval"), //name
                    "SyncTimeCombobox", //type
                    ""
                }
            }
        },
        {
            "manual_sync",
            "",
            {{
                    "manual_sync", //key
                    "", //name
                    "ManualSyncButton", //type
                    "" //default
                }
            }
        },
    }
};

static CalendarSettingSetting setting_base = {
    "setting_base",
    QObject::tr("Manage calendar"),
    {
        {
            "acccount_items",
            "",
            {
                {
                    "AccountCombobox",                //key
                    QObject::tr("Calendar account"),  //name
                    "AccountCombobox",                //type
                    ""
                }
            }
        },
        {
            "event_types",
            QObject::tr("Event types"),
            {
                {
                    "JobTypeListView",   //key
                    "",                  //name
                    "JobTypeListView",   //type
                    ""                   //default
                }
            }
        }
    }
};

static CalendarSettingSetting setting_base_noaccount = {
    "setting_base",
    QObject::tr("Manage calendar"),
    {
        {
            "event_types",
            QObject::tr("Event types"),
            {
                {
                    "JobTypeListView",   //key
                    "",                  //name
                    "JobTypeListView",   //type
                    ""                   //default
                }
            }
        }
    }
};

static CalendarSettingSetting setting_general = {
    "setting_general",
    QObject::tr("General settings"),
    {
        {
            "general",
            QObject::tr("General"),
            {
                { "firstday", QObject::tr("First day of week"), "FirstDayofWeek", "", "Sunday" },
                { "time", QObject::tr("Time"), "Time", "" },
                { "control-center-button", "", "ControlCenterLink", "" },
            },
        },
    }
};
CSettingDialog::CSettingDialog(QWidget *parent) : DSettingsDialog(parent)
{
    m_controlCenterProxy = new ControlCenterProxy(ControlCenterDBusName,
                                                  ControlCenterDBusPath,
                                                  QDBusConnection::sessionBus(),
                                                  this);
    initWidget();
    initConnect();
    initData();
    initWidgetDisplayStatus();
    initView();
}

void CSettingDialog::initView()
{
    setIcon(CDynamicIcon::getInstance()->getPixmap());
    setFixedSize(682, 506);
    widgetFactory()->registerWidget("login",              UserloginWidget::createloginButton);
    widgetFactory()->registerWidget("FirstDayofWeek",     std::bind(&CSettingDialog::createFirstDayofWeekWidget,  this, std::placeholders::_1));
    widgetFactory()->registerWidget("Time",               std::bind(&CSettingDialog::createTimeTypeWidget,        this, std::placeholders::_1));
    widgetFactory()->registerWidget("ControlCenterLink",  std::bind(&CSettingDialog::createControlCenterLink,     this, std::placeholders::_1));
    widgetFactory()->registerWidget("AccountCombobox",    std::bind(&CSettingDialog::createAccountCombobox,       this, std::placeholders::_1));
    widgetFactory()->registerWidget("JobTypeListView",    std::bind(&CSettingDialog::createJobTypeListView,       this, std::placeholders::_1));
    widgetFactory()->registerWidget("SyncTagRadioButton", std::bind(&CSettingDialog::createSyncTagRadioButton,    this, std::placeholders::_1));
    widgetFactory()->registerWidget("SyncTimeCombobox",   std::bind(&CSettingDialog::createSyncFreqCombobox,      this, std::placeholders::_1));
    widgetFactory()->registerWidget("ManualSyncButton",   std::bind(&CSettingDialog::createManualSyncButton,      this, std::placeholders::_1));
    QString strJson;

    CalendarSettingSettings calendarSettings;
    if (gAccountManager->getIsSupportUid()) {
        calendarSettings.append(setting_account);
        calendarSettings.append(setting_base);
    } else {
        calendarSettings.append(setting_base_noaccount);
    }

    calendarSettings.append(setting_general);

    QJsonObject obj;
    obj.insert("groups", calendarSettings.toJson());
    strJson = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    auto settings = Dtk::Core::DSettings::fromJson(strJson.toLatin1());
    setObjectName("SettingDialog");
    updateSettings(settings);
    //恢复默认设置按钮不显示
    setResetVisible(false);
    //QList<Widget>
    QList<QWidget *> lstwidget = findChildren<QWidget *>();
    if (lstwidget.size() > 0) { //accessibleName
        for (QWidget *wid : lstwidget) {
            if ("ContentWidgetForsetting_base.event_types" == wid->accessibleName()
                    || ("ContentSubTitleText" == wid->objectName()  && "setting_base.event_types" == wid->property("key").toString())) {
                QSpacerItem *spaceitem = new QSpacerItem(1, 1, QSizePolicy::Policy::Expanding);
                wid->layout()->addItem(spaceitem);
                DIconButton *addButton = this->createTypeAddButton();
                wid->layout()->addWidget(addButton);
                wid->layout()->addWidget(m_typeImportBtn);
                //使addButton的右边距等于view的右边距
                int leftMargin = wid->layout()->contentsMargins().left();
                wid->layout()->setContentsMargins(leftMargin, 0, leftMargin, 0);
            }
            if (wid->accessibleName().contains("DefaultWidgetAtContentRow")) {
                //DefaultWidgetAtContentRow是设置对话框右边每一个option条目对应widget的accessibleName的前缀，所以如果后续有更多条目，需要做修改
                wid->layout()->setMargin(0);
            }
        }
    }

    //未登录uos帐号时，移除部分选项
    if (!gUosAccountItem) {
        setGroupVisible("setting_account.account_sync_items", false);
        setGroupVisible("setting_account.sync_interval", false);
        setGroupVisible("setting_account.manual_sync", false);
    }

    //移除立刻同步按钮的背景色
    QWidget  *ManualSyncWidget = this->findChild<QWidget *>("ManualSyncWidget");
    ManualSyncWidget     = ManualSyncWidget == nullptr ? nullptr : ManualSyncWidget->parentWidget();
    ManualSyncWidget     = ManualSyncWidget == nullptr ? nullptr : ManualSyncWidget->parentWidget();
    DBackgroundGroup *bk = ManualSyncWidget == nullptr ? nullptr : qobject_cast<DBackgroundGroup *>(ManualSyncWidget);
    if (bk) {
        bk->setBackgroundRole(QPalette::Base);
    }
    //首次显示JobTypeListView时，更新日程类型
    m_scheduleTypeWidget->updateCalendarAccount(m_accountComboBox->currentData().toString());


    //账户登出登入时，隐藏显示相关界面
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, [ = ]() {
        if (!this->groupIsVisible("setting_account"))
            return;
        setGroupVisible("setting_account.account_sync_items",   gUosAccountItem != nullptr);
        setGroupVisible("setting_account.sync_interval",        gUosAccountItem != nullptr);
        setGroupVisible("setting_account.manual_sync",          gUosAccountItem != nullptr);
    });

    //同步项
    m_radiobuttonAccountCalendar = qobject_cast<SyncTagRadioButton *>(this->findChild<QWidget *>("Account_Calendar"));
    m_radiobuttonAccountSetting = qobject_cast<SyncTagRadioButton *>(this->findChild<QWidget *>("Account_Setting"));

    // Q_ASSERT(m_radiobuttonAccountSetting);

    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &CSettingDialog::slotSyncTagButtonUpdate);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &CSettingDialog::slotSyncTagButtonUpdate);
    if (m_radiobuttonAccountCalendar)
        connect(m_radiobuttonAccountCalendar, &SyncTagRadioButton::clicked, this, &CSettingDialog::slotSyncAccountStateUpdate);
    if (m_radiobuttonAccountSetting)
        connect(m_radiobuttonAccountSetting, &SyncTagRadioButton::clicked, this, &CSettingDialog::slotSyncAccountStateUpdate);

    slotSyncTagButtonUpdate();
}

void CSettingDialog::initWidget()
{
    initFirstDayofWeekWidget();
    initTimeTypeWidget();
    initAccountComboBoxWidget();
    initScheduleTypeWidget();
    initTypeAddWidget();
    initSyncFreqWidget();
    initManualSyncButton();
}

void CSettingDialog::initConnect()
{
    connect(gAccountManager, &AccountManager::signalGeneralSettingsUpdate, this, &CSettingDialog::slotGeneralSettingsUpdate);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &CSettingDialog::slotAccountUpdate);
    connect(gAccountManager, &AccountManager::signalLogout, this, &CSettingDialog::slotLogout);
    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &CSettingDialog::slotAccountStateChange);
    connect(m_firstDayofWeekCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotFirstDayofWeekCurrentChanged);
    connect(m_timeTypeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotTimeTypeCurrentChanged);
    connect(m_accountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotAccountCurrentChanged);
    connect(m_typeAddBtn, &DIconButton::clicked, this, &CSettingDialog::slotTypeAddBtnClickded);
    connect(m_typeImportBtn, &DIconButton::clicked, this, &CSettingDialog::slotTypeImportBtnClickded);
    //当日常类型超过上限时，更新button的状态
    connect(m_scheduleTypeWidget, &JobTypeListView::signalAddStatusChanged, m_typeAddBtn, &DIconButton::setEnabled);
    connect(m_scheduleTypeWidget, &JobTypeListView::signalAddStatusChanged, m_typeImportBtn, &DIconButton::setEnabled);
    //TODO:更新union帐户的的同步频率
    connect(m_syncFreqComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotSetUosSyncFreq);
    connect(m_syncBtn, &QPushButton::clicked, this, &CSettingDialog::slotUosManualSync);
    connect(m_ptrNetworkState, &DOANetWorkDBus::sign_NetWorkChange, this, &CSettingDialog::slotNetworkStateChange);
}

void CSettingDialog::slotNetworkStateChange(DOANetWorkDBus::NetWorkState state)
{
    if (DOANetWorkDBus::NetWorkState::Active == state) {
        if (!gUosAccountItem.isNull() && (gUosAccountItem->isCanSyncSetting() || gUosAccountItem->isCanSyncShedule())) {
            m_syncBtn->setEnabled(true);
        }
    } else if (DOANetWorkDBus::NetWorkState::Disconnect == state) {
        m_syncBtn->setEnabled(false);
    }
}

void CSettingDialog::initData()
{
    //通用设置数据初始化
    slotGeneralSettingsUpdate();
    //初始化账户信息
    slotAccountUpdate();
    //日程类型添加按钮初始化
    m_typeAddBtn->setEnabled(m_scheduleTypeWidget->canAdd());
    m_typeImportBtn->setEnabled(m_scheduleTypeWidget->canAdd());
    //同步频率数据初始化
    {
        int index = 0;
        if (gUosAccountItem) {
            index = m_syncFreqComboBox->findData(gUosAccountItem->getAccount()->syncFreq());
        }
        m_syncFreqComboBox->setCurrentIndex(index);
    }
    slotAccountStateChange();
}

void CSettingDialog::initWidgetDisplayStatus()
{

}

void CSettingDialog::initFirstDayofWeekWidget()
{
    m_firstDayofWeekWidget = new QWidget();

    m_firstDayofWeekCombobox = new QComboBox(m_firstDayofWeekWidget);
    m_firstDayofWeekCombobox->setFixedSize(150, 36);
    m_firstDayofWeekCombobox->addItem(tr("Sunday"));
    m_firstDayofWeekCombobox->addItem(tr("Monday"));
    m_firstDayofWeekCombobox->addItem(tr("Use System Setting"));

    QHBoxLayout *layout = new QHBoxLayout(m_firstDayofWeekWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(10);
    layout->addWidget(m_firstDayofWeekCombobox, 1);

    m_firstDayofWeekWidget->setLayout(layout);
}

void CSettingDialog::initTimeTypeWidget()
{
    m_timeTypeWidget = new QWidget();

    m_timeTypeCombobox = new QComboBox(m_timeTypeWidget);
    m_timeTypeCombobox->setFixedSize(150, 36);
    m_timeTypeCombobox->addItem(tr("24-hour clock"));
    m_timeTypeCombobox->addItem(tr("12-hour clock"));
    m_timeTypeCombobox->addItem(tr("Use System Setting"));

    QHBoxLayout *layout = new QHBoxLayout(m_timeTypeWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(10);
    layout->addWidget(m_timeTypeCombobox, 1);

    m_timeTypeWidget->setLayout(layout);
}

void CSettingDialog::initAccountComboBoxWidget()
{
    m_accountComboBox = new QComboBox();
    m_accountComboBox->setFixedSize(150, 36);
}

void CSettingDialog::initTypeAddWidget()
{
    m_typeAddBtn = new DIconButton(DStyle::SP_IncreaseElement, nullptr);
    m_typeAddBtn->setFixedSize(20, 20);

    m_typeImportBtn = new DIconButton(DStyle::SP_SelectElement, nullptr);
    m_typeImportBtn->setToolTip(tr("import ICS file"));
    m_typeImportBtn->setFixedSize(20, 20);
}

void CSettingDialog::initScheduleTypeWidget()
{
    m_scheduleTypeWidget = new JobTypeListView;
    m_scheduleTypeWidget->setObjectName("JobTypeListView");
}

void CSettingDialog::initSyncFreqWidget()
{
    m_syncFreqComboBox = new QComboBox;
    m_syncFreqComboBox->setMaximumWidth(150);
    m_syncFreqComboBox->addItem(tr("Manual"),   DAccount::SyncFreq_Maunal);
    m_syncFreqComboBox->addItem(tr("15 mins"),  DAccount::SyncFreq_15Mins);
    m_syncFreqComboBox->addItem(tr("30 mins"),  DAccount::SyncFreq_30Mins);
    m_syncFreqComboBox->addItem(tr("1 hour"),   DAccount::SyncFreq_1hour);
    m_syncFreqComboBox->addItem(tr("24 hours"), DAccount::SyncFreq_24hour);
    m_syncFreqComboBox->setCurrentIndex(1); // 默认15分钟
}

void CSettingDialog::initManualSyncButton()
{
    m_manualSyncWidget = new QWidget;
    m_ptrNetworkState = new DOANetWorkDBus(this);
    m_manualSyncWidget->setObjectName("ManualSyncWidget");
    m_syncBtn = new QPushButton(m_manualSyncWidget);
    m_syncBtn->setFixedSize(266, 36);
    m_syncBtn->setText(tr("Sync Now"));

    m_syncTimeLabel = new QLabel;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_syncBtn, 0, Qt::AlignCenter);
    layout->addWidget(m_syncTimeLabel, 0, Qt::AlignCenter);
    m_manualSyncWidget->setLayout(layout);

}

void CSettingDialog::slotGeneralSettingsUpdate()
{
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();
    if (!setting) {
        return;
    }
    setFirstDayofWeek(setting->firstDayOfWeek());
    setTimeType(setting->timeShowType());
}

void CSettingDialog::slotAccountUpdate()
{
    accountUpdate();
    //判断账户是否为登录状态，并建立连接
    if (gUosAccountItem) {
        slotLastSyncTimeUpdate(gUosAccountItem->getDtLastUpdate());
        connect(gUosAccountItem.get(), &AccountItem::signalDtLastUpdate, this, &CSettingDialog::slotLastSyncTimeUpdate);
    }
}

void CSettingDialog::slotLogout(DAccount::Type type)
{
    if (DAccount::Account_UnionID == type) {

    }
}

void CSettingDialog::slotFirstDayofWeekCurrentChanged(int index)
{
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();

    //此次只设置一周首日，不刷新界面
    if (index == 0) {
        gAccountManager->setFirstDayofWeek(7);
        gCalendarManager->setFirstDayOfWeek(7, false);
    } else if (index == 1) {
        gAccountManager->setFirstDayofWeek(1);
        gCalendarManager->setFirstDayOfWeek(1, false);
    } else {
        if (gAccountManager->getFirstDayofWeekSource() != DCalendarGeneralSettings::Source_System) {
            gAccountManager->setFirstDayofWeekSource(DCalendarGeneralSettings::Source_System);
        }
    }
}

void CSettingDialog::slotTimeTypeCurrentChanged(int index)
{
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();

    if (index == 0) {
        gAccountManager->setTimeFormatType(DCalendarGeneralSettings::TwentyFour);
        gCalendarManager->setTimeShowType(DCalendarGeneralSettings::TwentyFour, false);
    } else if (index == 1) {
        gAccountManager->setTimeFormatType(DCalendarGeneralSettings::Twelve);
        gCalendarManager->setTimeShowType(DCalendarGeneralSettings::Twelve, false);
    } else {
        if (gAccountManager->getTimeFormatTypeSource() != DCalendarGeneralSettings::Source_System) {
            gAccountManager->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_System);
        }
    }
}

void CSettingDialog::slotAccountCurrentChanged(int index)
{
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->updateCalendarAccount(m_accountComboBox->itemData(index).toString());
        setTypeEnable(index);
    }
}

void CSettingDialog::slotTypeAddBtnClickded()
{
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->slotAddScheduleType();
    }
}

void CSettingDialog::slotTypeImportBtnClickded()
{
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->slotImportScheduleType();
    }
}

void CSettingDialog::slotSetUosSyncFreq(int freq)
{
    QComboBox *com = qobject_cast<QComboBox *>(sender());
    if (!com)
        return;
    if (!gUosAccountItem)
        return;
    gUosAccountItem->setSyncFreq(DAccount::SyncFreqType(com->itemData(freq).toInt()));
}

void CSettingDialog::slotUosManualSync()
{
    if (!gUosAccountItem)
        return;
    gAccountManager->downloadByAccountID(gUosAccountItem->getAccount()->accountID());
}

void CSettingDialog::slotSyncTagButtonUpdate()
{
    if (!gUosAccountItem)
        return;

    auto state = gUosAccountItem->getAccount()->accountState();
    m_radiobuttonAccountCalendar->setChecked(state & DAccount::Account_Calendar);
    m_radiobuttonAccountSetting->setChecked(state & DAccount::Account_Setting);

    m_radiobuttonAccountCalendar->setEnabled(gUosAccountItem->isEnableForUosAccount());
    m_radiobuttonAccountSetting->setEnabled(gUosAccountItem->isEnableForUosAccount());
}

void CSettingDialog::slotSyncAccountStateUpdate(bool status)
{
    if (!gUosAccountItem)
        return;

    auto state = gUosAccountItem->getAccountState();

    if (m_radiobuttonAccountSetting->isChecked())
        state = state | DAccount::Account_Setting;
    else
        state = state & ~DAccount::Account_Setting;

    if (m_radiobuttonAccountCalendar->isChecked())
        state = state | DAccount::Account_Calendar;
    else
        state = state & ~DAccount::Account_Calendar;

    gUosAccountItem->setAccountState(state);
    if (status) {
        slotUosManualSync();
    }
}

void CSettingDialog::slotLastSyncTimeUpdate(const QString &datetime)
{
    QString dtstr;
    if (gCalendarManager->getTimeShowType()) {
        dtstr = dtFromString(datetime).toString("yyyy/MM/dd ap hh:mm");
    } else {
        dtstr = dtFromString(datetime).toString("yyyy/MM/dd hh:mm");
    }

    if (m_syncTimeLabel && gUosAccountItem && !dtstr.isEmpty()) {
        m_syncTimeLabel->setText(tr("Last sync") + ":" + dtstr);
    }
}

void CSettingDialog::slotAccountStateChange()
{
    if (!gUosAccountItem) {
        return;
    }
    if (m_syncBtn) {
        if (gUosAccountItem->isCanSyncSetting() || gUosAccountItem->isCanSyncShedule()) {
            m_syncBtn->setEnabled(true);
        } else {
            m_syncBtn->setEnabled(false);
        }
    }

    if (m_syncFreqComboBox) {
        if (gUosAccountItem->isCanSyncSetting() || gUosAccountItem->isCanSyncShedule()) {
            m_syncFreqComboBox->setEnabled(true);
        } else {
            m_syncFreqComboBox->setEnabled(false);
        }
    }
    if (m_accountComboBox && m_typeAddBtn) {
        setTypeEnable(m_accountComboBox->currentIndex());
    }
}

void CSettingDialog::setFirstDayofWeek(int value)
{
    if (!m_firstDayofWeekCombobox) {
        return;
    }
    auto sourceSystem =
        gAccountManager->getFirstDayofWeekSource() == DCalendarGeneralSettings::Source_System;

    if (sourceSystem) {
        m_firstDayofWeekCombobox->setCurrentIndex(m_firstDayofWeekCombobox->count() - 1);
    } else {
        if (value == 1) {
            m_firstDayofWeekCombobox->setCurrentIndex(1);
        } else {
            m_firstDayofWeekCombobox->setCurrentIndex(0);
        }
    }
    // 设置一周首日并刷新界面
    gCalendarManager->setFirstDayOfWeek(value, true);
}

void CSettingDialog::setTimeType(int value)
{
    if (!m_timeTypeCombobox) {
        return;
    }
    if (value > 1 || value < 0) {
        value = 0;
    }
    auto sourceSystem =
        gAccountManager->getTimeFormatTypeSource() == DCalendarGeneralSettings::Source_System;
    // 设置时间显示格式并刷新界面
    if (sourceSystem) {
        m_timeTypeCombobox->setCurrentIndex(m_firstDayofWeekCombobox->count() - 1);
    } else {
        m_timeTypeCombobox->setCurrentIndex(value);
    }
    gCalendarManager->setTimeShowType(value, true);
}

void CSettingDialog::accountUpdate()
{
    if (nullptr == m_accountComboBox) {
        return;
    }
    QVariant oldAccountID = m_accountComboBox->currentData();
    m_accountComboBox->blockSignals(true);
    m_accountComboBox->clear();
    for (auto account : gAccountManager->getAccountList()) {
        m_accountComboBox->addItem(account->getAccount()->accountName(), account->getAccount()->accountID());
    }
    m_accountComboBox->setCurrentIndex(m_accountComboBox->findData(oldAccountID));
    if (m_accountComboBox->currentIndex() < 0)
        m_accountComboBox->setCurrentIndex(0);
    m_accountComboBox->blockSignals(false);

    m_syncFreqComboBox->setCurrentIndex(1);  //每次登录的时候 默认15分钟
    slotAccountCurrentChanged(m_accountComboBox->currentIndex());
}

void CSettingDialog::setTypeEnable(int index)
{
    if (!gUosAccountItem) return;

    QString accountId = m_accountComboBox->itemData(index).toString();

    AccountItem::Ptr account = gAccountManager->getAccountItemByAccountId(accountId);

    if (account->getAccount()->accountType() == DAccount::Account_Local || gUosAccountItem->isCanSyncShedule()) {
        if (account->getScheduleTypeList().count() < 20) {
            m_typeAddBtn->setEnabled(true);
            m_typeImportBtn->setEnabled(true);
            m_scheduleTypeWidget->setItemEnabled(true);
        }
    } else {
        m_typeAddBtn->setEnabled(false);
        m_typeImportBtn->setEnabled(false);
        m_scheduleTypeWidget->setItemEnabled(false);
    }
}

QPair<QWidget *, QWidget *> CSettingDialog::createFirstDayofWeekWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_firstDayofWeekWidget);
    // 获取初始值
    option->setValue(option->defaultValue());
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createTimeTypeWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_timeTypeWidget);
    // 获取初始值
    option->setValue(option->defaultValue());
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createAccountCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_accountComboBox);
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createSyncFreqCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_syncFreqComboBox);
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createSyncTagRadioButton(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    DAccount::AccountState type = DAccount::Account_Calendar;
    if (option->key().endsWith("Account_Calendar"))
        type = DAccount::Account_Calendar;
    if (option->key().endsWith("Account_Setting"))
        type = DAccount::Account_Setting;

    SyncTagRadioButton *widget = new SyncTagRadioButton;
    widget->setObjectName(option->key().section('.', -1));//设置objectname为Account_Calendar 或 Account_Setting
    widget->setFixedWidth(16);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);

    //iconLabel
    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedSize(24, 24);
    if (DAccount::Account_Calendar == type)
        iconLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/icons/dde_calendar_sync_schedule_32px.svg"));
    if (DAccount::Account_Setting == type)
        iconLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/icons/dde_calendar_sync_setting_32px.svg"));

    //iconWidget
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(iconLabel);
    layout->addWidget(optionWidget.first);
    layout->setContentsMargins(0, 1, 0, 1);
    QWidget *iconWidget = new QWidget;
    iconWidget->setLayout(layout);
    optionWidget.first = iconWidget;

    return optionWidget;
}

QWidget *CSettingDialog::createManualSyncButton(QObject *obj)
{
    return m_manualSyncWidget;
}

QWidget *CSettingDialog::createJobTypeListView(QObject *)
{
    return m_scheduleTypeWidget;
}

DIconButton *CSettingDialog::createTypeAddButton()
{
    return m_typeAddBtn;
}

QWidget *CSettingDialog::createControlCenterLink(QObject *obj)
{
    DLabel *myLabel = new DLabel(tr("Please go to the <a href='/'>Control Center</a> to change system settings"), this);
    myLabel->setTextFormat(Qt::RichText);
    myLabel->setFixedHeight(36);
    connect(myLabel, &DLabel::linkActivated, this, [this]{
        qCDebug(ClientLogger) << "open deepin control center";
        this->m_controlCenterProxy->ShowPage(ControlCenterPage);
    });
    auto w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 10, 0);
    layout->setSpacing(0);
    layout->addStretch(10);
    layout->addWidget(myLabel, 1);

    w->setLayout(layout);
    return w;
}