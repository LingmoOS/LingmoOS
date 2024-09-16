// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filtercontent.h"
#include "logcombox.h"
#include "lognormalbutton.h"
#include "logapplicationhelper.h"
#include "logperiodbutton.h"
#include "loglistview.h"
#include "structdef.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DComboBox>
#include <DCommandLinkButton>
#include <DFileDialog>

#include <QAbstractItemView>
#include <QDebug>
#include <QDir>
#include <QSizePolicy>
#include <QFileInfo>
#include <QFileInfoList>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QProcess>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QPainterPath>

#define BUTTON_WIDTH_MIN 68
#define BUTTON_HEIGHT_MIN 36
#define BUTTON_EXPORT_WIDTH_MIN 142
#define FONT_20_MIN_WIDTH 821
#define FONT_18_MIN_WIDTH 100
#define LEVEL_COMBO_WIDTH 120
DWIDGET_USE_NAMESPACE

/**
 * @brief FilterContent::FilterContent 构造函数初始化ui和信号槽连接
 * @param parent
 */
FilterContent::FilterContent(QWidget *parent)
    : DFrame(parent)
    , m_curBtnId(ALL)
    , m_curLvCbxId(INF)
{
    initUI();
    initConnections();
}

FilterContent::~FilterContent() {}

/**
 * @brief FilterContent::initUI 初始化界面
 */
void FilterContent::initUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    // set period info
    hLayout_period = new QHBoxLayout;
    periodLabel = new DLabel(DApplication::translate("Label", "Period:"), this);
    periodLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    m_btnGroup = new QButtonGroup(this);
    //初始化时间筛选按钮部分布局
    m_allBtn = new LogPeriodButton(DApplication::translate("Button", "All"), this);
    m_allBtn->setToolTip(DApplication::translate("Button", "All"));  // add by Airy for bug 16245
    m_btnGroup->addButton(m_allBtn, 0);
    m_todayBtn = new LogPeriodButton(DApplication::translate("Button", "Today"), this);
    m_todayBtn->setToolTip(DApplication::translate("Button", "Today"));  // add by Airy for bug
    m_btnGroup->addButton(m_todayBtn, 1);
    m_threeDayBtn = new LogPeriodButton(DApplication::translate("Button", "3 days"), this);
    m_threeDayBtn->setToolTip(DApplication::translate("Button", "3 days")); // add by Airy for bug 16245
    m_btnGroup->addButton(m_threeDayBtn, 2);
    m_lastWeekBtn = new LogPeriodButton(DApplication::translate("Button", "1 week"), this);
    m_lastWeekBtn->setToolTip(DApplication::translate("Button", "1 week")); // add by Airy for bug 16245
    m_btnGroup->addButton(m_lastWeekBtn, 3);
    m_lastMonthBtn = new LogPeriodButton(DApplication::translate("Button", "1 month"), this);
    m_lastMonthBtn->setToolTip(DApplication::translate("Button", "1 month")); // add by Airy for bug 16245
    m_btnGroup->addButton(m_lastMonthBtn, 4);
    m_threeMonthBtn = new LogPeriodButton(DApplication::translate("Button", "3 months"), this);
    m_threeMonthBtn->setToolTip(DApplication::translate("Button", "3 months")); // add by Airy for bug 16245
    m_btnGroup->addButton(m_threeMonthBtn, 5);
    //设置初始时间筛选为全部
    setUeButtonSytle();
    //所有时间筛选按钮放在上排layout
    hLayout_period->addWidget(periodLabel);
    hLayout_period->addWidget(m_allBtn);
    hLayout_period->addWidget(m_todayBtn);
    hLayout_period->addWidget(m_threeDayBtn);
    hLayout_period->addWidget(m_lastWeekBtn);
    hLayout_period->addWidget(m_lastMonthBtn);
    hLayout_period->addWidget(m_threeMonthBtn);
    hLayout_period->addStretch(1);
    hLayout_period->setSpacing(10);

    // set level info
    hLayout_all = new QHBoxLayout;
    QHBoxLayout *hLayout_lvl = new QHBoxLayout;
    lvTxt = new DLabel(DApplication::translate("Label", "Level:  "), this);
    lvTxt->setAccessibleName(DApplication::translate("Label", "Level:  "));
    lvTxt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cbx_lv = new LogCombox(this);
    cbx_lv->view()->setAccessibleName("combobox_level_view");
    cbx_lv->setMinimumSize(QSize(LEVEL_COMBO_WIDTH, BUTTON_HEIGHT_MIN));
    cbx_lv->addItems(QStringList() << DApplication::translate("ComboBox", "All")
                     << DApplication::translate("ComboBox", "Emergency")
                     << DApplication::translate("ComboBox", "Alert")
                     << DApplication::translate("ComboBox", "Critical")
                     << DApplication::translate("ComboBox", "Error")
                     << DApplication::translate("ComboBox", "Warning")
                     << DApplication::translate("ComboBox", "Notice")
                     << DApplication::translate("ComboBox", "Info")
                     << DApplication::translate("ComboBox", "Debug"));
    cbx_lv->setCurrentText(DApplication::translate("ComboBox", "Info"));
    hLayout_lvl->addWidget(lvTxt);
    hLayout_lvl->addWidget(cbx_lv, 1);
    hLayout_lvl->setSpacing(6);
    hLayout_all->addLayout(hLayout_lvl);
    QHBoxLayout *hLayout_dnf_lvl = new QHBoxLayout;
    dnflvTxt = new DLabel(DApplication::translate("Label", "Level:  "), this);
    dnflvTxt->setAccessibleName("dnf" + DApplication::translate("Label", "Level:  "));
    dnflvTxt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cbx_dnf_lv = new LogCombox(this);
    cbx_dnf_lv->view()->setAccessibleName("combobox_dnflevel_view");
    cbx_dnf_lv->setMinimumSize(QSize(198, BUTTON_HEIGHT_MIN));
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "All"), DNFLVALL);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Super critical"), SUPERCRITICAL);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Critical"), CRITICAL);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Error"), ERROR);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Warning"), WARNING);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Info"), INFO);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Debug"), DEBUG);
    cbx_dnf_lv->addItem(DApplication::translate("ComboBox", "Trace"), TRACE);
    hLayout_dnf_lvl->addWidget(dnflvTxt);
    hLayout_dnf_lvl->addWidget(cbx_dnf_lv, 1);
    hLayout_dnf_lvl->setSpacing(6);
    hLayout_all->addLayout(hLayout_dnf_lvl);
    // set all files under ~/.cache/deepin
    QHBoxLayout *hLayout_app = new QHBoxLayout;
    appTxt = new DLabel(DApplication::translate("Label", "Application:"), this);
    cbx_app = new LogCombox(this);
    cbx_app->view()->setAccessibleName("combobox_app_view");

    cbx_app->setMinimumSize(QSize(160, BUTTON_HEIGHT_MIN));
    hLayout_app->addWidget(appTxt);
    hLayout_app->addWidget(cbx_app, 1);
    hLayout_app->setSpacing(6);
    hLayout_all->addLayout(hLayout_app);

    // app submodules
    QHBoxLayout *hLayout_submodule = new QHBoxLayout;
    submoduleTxt = new DLabel(DApplication::translate("Label", "Submodule:"), this);
    cbx_submodule = new LogCombox(this);
    cbx_submodule->view()->setAccessibleName("combobox_submodule_view");

    cbx_submodule->setMinimumSize(QSize(143, BUTTON_HEIGHT_MIN));
    hLayout_submodule->addWidget(submoduleTxt);
    hLayout_submodule->addWidget(cbx_submodule, 1);
    hLayout_submodule->setSpacing(6);
    hLayout_all->addLayout(hLayout_submodule);


    // add status item
    QHBoxLayout *hLayout_status = new QHBoxLayout;
    statusTxt = new DLabel(DApplication::translate("Label", "Status:"), this);
    cbx_status = new LogCombox(this);
    cbx_status->view()->setAccessibleName("combobox_status_view");
    cbx_status->setMinimumWidth(120);
    cbx_status->setMinimumSize(QSize(120, BUTTON_HEIGHT_MIN));
    cbx_status->addItems(QStringList() << DApplication::translate("ComboBox", "All") << "OK"
                         << "Failed");
    hLayout_status->addWidget(statusTxt);
    hLayout_status->addWidget(cbx_status, 1);
    hLayout_status->setSpacing(6);
    hLayout_all->addLayout(hLayout_status);

    // add by Airy for adding type item
    QHBoxLayout *hLayout_type = new QHBoxLayout;
    typeTxt = new DLabel(DApplication::translate("Label", "Event Type:"), this);
    typeCbx = new LogCombox(this);
    typeCbx->view()->setAccessibleName("combobox_eventtype_view");
    typeCbx->setMinimumWidth(120);
    typeCbx->setMinimumSize(QSize(120, BUTTON_HEIGHT_MIN));
    typeCbx->addItems(QStringList() << DApplication::translate("ComboBox", "All")
                      << DApplication::translate("ComboBox", "Login")
                      << DApplication::translate("ComboBox", "Boot")
                      << DApplication::translate("ComboBox", "Shutdown"));
    hLayout_status->addWidget(typeTxt);
    hLayout_status->addWidget(typeCbx, 1);
    hLayout_status->setSpacing(6);
    hLayout_all->addLayout(hLayout_type);  // end add

    // 审计日志审计类型筛选下拉框
    QHBoxLayout *hLayout_auditType = new QHBoxLayout;
    auditTypeTxt = new DLabel(DApplication::translate("Label", "Audit Type:"), this);
    auditTypeCbx = new LogCombox(this);
    auditTypeCbx->setMinimumWidth(120);
    auditTypeCbx->setMinimumSize(QSize(120, BUTTON_HEIGHT_MIN));
    auditTypeCbx->addItems(QStringList() << DApplication::translate("ComboBox", "All")
                      << DApplication::translate("ComboBox", "Identity authentication")
                      << DApplication::translate("ComboBox", "Discretionary Access Control")
                      << DApplication::translate("ComboBox", "Mandatory access control")
                      << DApplication::translate("ComboBox", "Remote")
                      << DApplication::translate("ComboBox", "Document audit")
                      << DApplication::translate("ComboBox", "Other"));
    hLayout_status->addWidget(auditTypeTxt);
    hLayout_status->addWidget(auditTypeCbx, 1);
    hLayout_status->setSpacing(6);
    hLayout_all->addLayout(hLayout_auditType);  // end add

    hLayout_all->addStretch(1);
    exportBtn = new LogNormalButton(DApplication::translate("Button", "Export", "button"), this);
    exportBtn->setContentsMargins(0, 0, 18, 18);
    exportBtn->setFixedWidth(BUTTON_EXPORT_WIDTH_MIN);
    hLayout_all->addWidget(exportBtn);
    // set layout
    vLayout->addLayout(hLayout_period);
    vLayout->addLayout(hLayout_all);
    vLayout->setSpacing(16);
    this->setLayout(vLayout);
    setSelectorVisible(true, false, false, true, false);
    m_currentType = JOUR_TREE_DATA;
    //设置初始筛选选项

    updateDataState();
    //为时间筛选按钮设置事件过滤器,为处理左右按键键盘事件触发切换选择时间筛选器选项
    m_allBtn->installEventFilter(this);
    m_todayBtn->installEventFilter(this);
    m_threeDayBtn->installEventFilter(this);
    m_lastWeekBtn->installEventFilter(this);
    m_lastMonthBtn->installEventFilter(this);
    m_threeMonthBtn->installEventFilter(this);
}

/**
 * @brief FilterContent::initConnections 连接信号槽
 */
void FilterContent::initConnections()
{
    connect(m_btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonClicked(int)));
    connect(exportBtn, &DPushButton::clicked, this, &FilterContent::slot_exportButtonClicked);
    connect(cbx_dnf_lv, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxDnfLvIdxChanged(int)));

    connect(cbx_lv, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxLvIdxChanged(int)));
    connect(cbx_app, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAppIdxChanged(int)), Qt::UniqueConnection);
    connect(cbx_status, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxStatusChanged(int)));
    connect(typeCbx, SIGNAL(currentIndexChanged(int)), this,
            SLOT(slot_cbxLogTypeChanged(int)));  // add by Airy
    connect(auditTypeCbx, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAuditTypeChanged(int)));
}

/**
 * @brief FilterContent::shortCutExport 主窗口触发导出快捷键时,在这里判断是否能够导出再发出导出信号
 */
void FilterContent::shortCutExport()
{
    QString itemData = m_curTreeIndex.data(ITEM_DATE_ROLE).toString();
    bool canExport = true;
    if (exportBtn) {
        canExport = exportBtn->isEnabled();
    }
    //判断现在导出按钮是否能够点击,不能点击时不可以导出
    if (!itemData.isEmpty() && canExport)
        emit sigExportInfo();
}

/**
 * @brief FilterContent::setAppComboBoxItem 刷新应用种类下拉列表
 */
void FilterContent::setAppComboBoxItem()
{
    //必须先disconnect变动值的信号槽,因为改变下拉选项会几次触发currentIndexChanged信号,这不是我们想要的
    disconnect(cbx_app, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAppIdxChanged(int)));
    cbx_app->clear();
    //获取应用列表
    auto *appHelper = LogApplicationHelper::instance();
    AppLogConfigList appConfigs =  appHelper->getAppLogConfigs();
    //添加数据进combox
    for (auto appConfig : appConfigs) {
        // 没有子模块，不在应用列表显示
        if (appConfig.subModules.size() == 0 || !appConfig.visible)
            continue;

        if (!appConfig.transName.isEmpty())
            cbx_app->addItem(appConfig.transName);
        else
            cbx_app->addItem(appConfig.name);
        // 应用日志的项目名称为Qt::UserRole + 1
        cbx_app->setItemData(cbx_app->count() - 1, appConfig.name, Qt::UserRole + 1);
    }

    connect(cbx_app, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAppIdxChanged(int)), Qt::UniqueConnection);

}

void FilterContent::setSubmodulesComboBoxItem(const QString &app)
{
    AppLogConfig logConfig = LogApplicationHelper::instance()->appLogConfig(app);

    // 子模块只有一个，则不显示子模块下拉框
    if (logConfig.subModules.size() < 2) {
        submoduleTxt->setVisible(false);
        cbx_submodule->setVisible(false);
    } else {
        submoduleTxt->setVisible(true);
        cbx_submodule->setVisible(true);
        disconnect(cbx_submodule, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxSubmoduleChanged(int)));
        cbx_submodule->clear();
        cbx_submodule->addItem(DApplication::translate("ComboBox", "All"));
        for (auto submodule : logConfig.subModules) {
            cbx_submodule->addItem(submodule.name);
        }
        cbx_submodule->setCurrentText(DApplication::translate("ComboBox", "All"));
        connect(cbx_submodule, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxSubmoduleChanged(int)));
    }
}

/**
 * @brief FilterContent::setSelectorVisible 设置筛选控件显示或不显示以适应各种日志类型的筛选情况
 * @param lvCbx 等级筛选下拉框是否显示
 * @param appListCbx 应用筛选下拉框是否显示
 * @param statusCbx 启动日志状态筛选下拉框是否显示
 * @param period 时间筛选按钮是否显示
 * @param needMove 如果筛选器只有单排布局,则需要移动导出按钮到上排布局,这个参数表示是否把导出按钮移动到上排的布局
 * @param typecbx 开关机日志日志种类筛选下拉框是否显示
 * @param auditcbx 审计日志审计类型筛选下拉框是否显示
 */
void FilterContent::setSelectorVisible(bool lvCbx, bool appListCbx, bool statusCbx, bool period,
                                       bool needMove, bool typecbx, bool dnfCbx, bool auditCbx)
{
    //先不立马更新界面,等全部更新好控件状态后再更新界面,否则会导致界面跳动
    setUpdatesEnabled(false);
    lvTxt->setVisible(lvCbx);
    cbx_lv->setVisible(lvCbx);


    appTxt->setVisible(appListCbx);
    cbx_app->setVisible(appListCbx);

    submoduleTxt->setVisible(false);
    cbx_submodule->setVisible(false);

    dnflvTxt->setVisible(dnfCbx);
    cbx_dnf_lv->setVisible(dnfCbx);

    typeTxt->setVisible(typecbx);  // add by Airy
    typeCbx->setVisible(typecbx);  // add by Airy

    auditTypeTxt->setVisible(auditCbx);
    auditTypeCbx->setVisible(auditCbx);

    periodLabel->setVisible(period);
    //button的setVisible false会触发taborder到下一个可视控件,比如cbx_status,所以先设置button,再设置cbx_status可防止点击后时间筛选button后再切启动日志导致cbx_status自动进入tabfocus状态,但是这样会引起本窗口焦点重置,所以设置完后需要对loglist setfoucs
    for (int i = 0; i < 6; i++) {
        LogPeriodButton *pushBtn = static_cast<LogPeriodButton *>(m_btnGroup->button(i));
        pushBtn->setVisible(period);
    }
    statusTxt->setVisible(statusCbx);
    cbx_status->setVisible(statusCbx);
    //根据是否需要移动导出按钮移动到对应布局
    if (needMove) {
        hLayout_period->addWidget(exportBtn);
        hLayout_all->removeWidget(exportBtn);
    } else {
        hLayout_period->removeWidget(exportBtn);
        hLayout_all->addWidget(exportBtn);

    }
    resizeWidth();
    //先不立马更新界面,等全部更新好控件状态后再更新界面,否则会导致界面跳动
    setUpdatesEnabled(true);

    cbx_lv->setObjectName("level_combox");
    cbx_app->setObjectName("app_combox");
    cbx_status->setObjectName("status_combox");
    typeCbx->setObjectName("event_type_combox");
}

/**
 * @brief FilterContent::setSelection 设置当前筛选器选择的状态
 * @param iConifg 控件的选择值的结构体
 */
void FilterContent::setSelection(FILTER_CONFIG iConifg)
{
    //控件不显示说明此情况下不需要给它设置值
    if (cbx_lv->isVisible())
        cbx_lv->setCurrentIndex(iConifg.levelCbx);
    if (cbx_app->isVisible()) {
        QString app = iConifg.appListCbx;
        int appCount =  cbx_app->count();
        int rsIndex = 0;

        if (!app.isEmpty()) {
            //找原来选的选项的index,如果这个应用日志没了,就选第一个
            for (int i = 0; i < appCount; ++i) {
                if (cbx_app->itemData(i, Qt::UserRole + 1).toString() == app) {
                    rsIndex = i;
                    break;
                }
            }
            //不能直接connect再setCurrentIndex,而是要手动发出改变app的信号让其刷新,让combox自己发的话,如果原来的index是0他不发currentindexChanged信号
        }
        if (rsIndex == 0) {
            Q_EMIT cbx_app->currentIndexChanged(0);
        } else {
            cbx_app->setCurrentIndex(rsIndex);
        }

        if (cbx_submodule->isVisible()) {

            int nSubIndex = 0;
            //找原来选的选项的index,如果这个子模块没了,就选择所有
            for (int i = 0; i < cbx_submodule->count(); ++i) {
                if (cbx_submodule->itemText(i) == iConifg.app2Submodule[app]) {
                    nSubIndex = i;
                    break;
                }
            }
            cbx_submodule->setCurrentIndex(nSubIndex);
        }
    }

    if (cbx_status->isVisible())
        Q_EMIT cbx_status->currentIndexChanged(iConifg.statusCbx);
    if (typeCbx->isVisible())
        typeCbx->setCurrentIndex(iConifg.typeCbx);
    if (auditTypeCbx->isVisible())
        auditTypeCbx->setCurrentIndex(iConifg.auditCbx);
    if (cbx_dnf_lv->isVisible()) {
        cbx_dnf_lv->setCurrentIndex(iConifg.dnfCbx);
    }
    m_btnGroup->button(iConifg.dateBtn)->setChecked(true); //add by Airy for bug 19660:period button default setting
    Q_EMIT m_btnGroup->button(iConifg.dateBtn)->click();
}

/**
 * @brief FilterContent::setUeButtonSytle 时间筛选器重置为默认选择全部
 */
void FilterContent::setUeButtonSytle()
{
    for (QAbstractButton *abtn : m_btnGroup->buttons()) {
        LogPeriodButton *btn = static_cast<LogPeriodButton *>(abtn);

        if (btn->objectName() == "allBtn")
            btn->setChecked(true);
    }

}

/**
 * @brief FilterContent::paintEvent 绘制事件徐函数,主要用来绘制出筛选控件背景的圆角矩形效果
 * @param event 绘制事件
 */
void FilterContent::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Save pen
    QPen oldPen = painter.pen();
    //设置画笔颜色角色,根据主题变色
    painter.setRenderHint(QPainter::Antialiasing);
    DPalette pa = DApplicationHelper::instance()->palette(this);
    painter.setBrush(QBrush(pa.color(DPalette::Base)));
    QColor penColor = pa.color(DPalette::FrameBorder);
    //设置透明度
    penColor.setAlphaF(0.05);
    painter.setPen(QPen(penColor));
    //算出背景圆角矩形的大小位置
    QRectF rect = this->rect();
    rect.setX(0.5);
    rect.setY(0.5);
    rect.setWidth(rect.width() - 0.5);
    rect.setHeight(rect.height() - 0.5);

    QPainterPath painterPath;
    //绘制圆角矩形
    painterPath.addRoundedRect(rect, 8, 8);
    painter.drawPath(painterPath);

    // Restore the pen
    painter.setPen(oldPen);

    DFrame::paintEvent(event);
}

/**
 * @brief FilterContent::eventFilter 事件过滤器
 * @param obj 发生事件的对象
 * @param event 事件指针
 * @return true为截获事件,处理后不继续传递,否则继续传递下去
 */
bool FilterContent::eventFilter(QObject *obj, QEvent *event)
{
    //判断是否为键盘按下事件
    if (event->type() == QEvent::KeyPress) {
        if (obj == m_allBtn) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            //左右按钮使焦点和选项变为两边的按钮,第一个按钮往左是最后一个按钮
            if(kev->key()==Qt::Key_Tab||kev->key()==Qt::Key_Backtab){
               return false;
            }
            if (kev->key() == Qt::Key_Right) {
                m_todayBtn->click();
                m_todayBtn->setFocus(Qt::TabFocusReason);
            } else if (kev->key() == Qt::Key_Left) {
                m_threeMonthBtn->click();
                m_threeMonthBtn->setFocus(Qt::TabFocusReason);
            }
            return true;
        } else if (obj == m_todayBtn) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if(kev->key()==Qt::Key_Tab||kev->key()==Qt::Key_Backtab){
               return false;
            }
            if (kev->key() == Qt::Key_Right) {
                m_threeDayBtn->click();
                m_threeDayBtn->setFocus(Qt::TabFocusReason);
            } else if (kev->key() == Qt::Key_Left) {
                m_allBtn->click();
                m_allBtn->setFocus(Qt::TabFocusReason);
            }
            return true;
        } else if (obj == m_threeDayBtn) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if(kev->key()==Qt::Key_Tab||kev->key()==Qt::Key_Backtab){
               return false;
            }
            if (kev->key() == Qt::Key_Right) {
                m_lastWeekBtn->click();
                m_lastWeekBtn->setFocus(Qt::TabFocusReason);
            } else if (kev->key() == Qt::Key_Left) {
                m_todayBtn->click();
                m_todayBtn->setFocus(Qt::TabFocusReason);
            }
            return true;
        } else if (obj == m_lastWeekBtn) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if(kev->key()==Qt::Key_Tab||kev->key()==Qt::Key_Backtab){
               return false;
            }
            if (kev->key() == Qt::Key_Right) {
                m_lastMonthBtn->click();
                m_lastMonthBtn->setFocus(Qt::TabFocusReason);
            } else if (kev->key() == Qt::Key_Left) {
                m_threeDayBtn->click();
                m_threeDayBtn->setFocus(Qt::TabFocusReason);
            }
            return true;
        } else if (obj == m_lastMonthBtn) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if(kev->key()==Qt::Key_Tab||kev->key()==Qt::Key_Backtab){
               return false;
            }
            if (kev->key() == Qt::Key_Right) {
                m_threeMonthBtn->click();
                m_threeMonthBtn->setFocus(Qt::TabFocusReason);
            } else if (kev->key() == Qt::Key_Left) {
                m_lastWeekBtn->click();
                m_lastWeekBtn->setFocus(Qt::TabFocusReason);
            }
            return true;
        }  else if (obj == m_threeMonthBtn) {
            //最后一个按钮往右为第一个按钮,如此循环
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            if(kev->key()==Qt::Key_Tab||kev->key()==Qt::Key_Backtab){
               return false;
            }
            if (kev->key() == Qt::Key_Right) {
                m_allBtn->click();
                m_allBtn->setFocus(Qt::TabFocusReason);
            } else if (kev->key() == Qt::Key_Left) {
                m_lastMonthBtn->click();
                m_lastMonthBtn->setFocus(Qt::TabFocusReason);
            }
            return true;
        }
    }
    return DFrame::eventFilter(obj, event);

}


/**
 * @brief FilterContent::resizeWidth 自适应宽度
 */
void FilterContent::resizeWidth()
{
    int periodWidth = 0;
    int periodCount = hLayout_period->count();

    for (int i = 0; i < periodCount ; ++i) {
        QLayoutItem *item = hLayout_period->itemAt(i);
        QWidget *item_w = item->widget();
        if (item_w && (!item_w->isHidden())) {
            int  itemWidth = item_w->geometry().width();
            periodWidth += itemWidth;
        }
    }
    emit   sigResizeWidth(periodWidth);
}

/**
 * @brief FilterContent::updateWordWrap 根据当前宽度省略日期筛选按钮文字以适应宽度，让控件能塞下
 */
void FilterContent::updateWordWrap()
{
    int currentWidth = this->rect().width();
    setUpdatesEnabled(false);
    //12对应系统16字号，13.5对应系统18字号,15对应系统20字号
    qreal fontSize = m_allBtn->font().pointSizeF();
    int minWidth = -1;
    if (fontSize <= 13.5) {
        minWidth = FONT_20_MIN_WIDTH;
    } else {
        minWidth = FONT_20_MIN_WIDTH;
    }
    //判断各字体下能塞下控件的最小宽度
    if ((currentWidth <= minWidth) && (hLayout_period->count() > 9)) {
        QFont standFont = m_allBtn->font();
        QFont standFontBig = standFont;
        standFont.setPointSizeF(13.5);
        periodLabel->setText(QFontMetrics(periodLabel->font()).elidedText(DApplication::translate("Label", "Period:"), Qt::ElideRight, 1 + QFontMetrics(periodLabel->font()).width(DApplication::translate("Label", "Period:"))));
        m_todayBtn->setText(QFontMetrics(m_todayBtn->font()).elidedText(DApplication::translate("Button", "Today"), Qt::ElideRight, 1 + QFontMetrics(standFont).width(DApplication::translate("Button", "Today"))));
        m_threeDayBtn->setText(QFontMetrics(m_threeDayBtn->font()).elidedText(DApplication::translate("Button", "3 days"), Qt::ElideRight, 1 + QFontMetrics(standFont).width(DApplication::translate("Button", "3 days"))));
        m_lastWeekBtn->setText(QFontMetrics(m_lastWeekBtn->font()).elidedText(DApplication::translate("Button", "1 week"), Qt::ElideRight, 1 + QFontMetrics(standFont).width(DApplication::translate("Button", "1 week"))));
        m_lastMonthBtn->setText(QFontMetrics(m_lastMonthBtn->font()).elidedText(DApplication::translate("Button", "1 month"), Qt::ElideRight, 1 + QFontMetrics(standFont).width(DApplication::translate("Button", "1 month"))));
        m_threeMonthBtn->setText(QFontMetrics(m_threeMonthBtn->font()).elidedText(DApplication::translate("Button", "3 months"), Qt::ElideRight, 1 + QFontMetrics(standFont).width(DApplication::translate("Button", "3 months"))));
    }  else  {
        periodLabel->setText(DApplication::translate("Label", "Period:"));
        m_allBtn->setText(DApplication::translate("Button", "All"));
        m_todayBtn->setText(DApplication::translate("Button", "Today"));
        m_threeDayBtn->setText(DApplication::translate("Button", "3 days"));
        m_lastWeekBtn->setText(DApplication::translate("Button", "1 week"));
        m_lastMonthBtn->setText(DApplication::translate("Button", "1 month"));
        m_threeMonthBtn->setText(DApplication::translate("Button", "3 months"));
    }
    setUpdatesEnabled(true);
}

/**
 * @brief FilterContent::updateDataState  每次日志类型变动或需要刷新时,按记录的数据刷新筛选器的选项
 */
void FilterContent::updateDataState()
{
    //如果没有记录当前日志的选项,则给一个默认的
    if (!m_config.contains(m_currentType)) {
        FILTER_CONFIG newConfig;
        m_config.insert(m_currentType, newConfig);
    }
    //按记录的筛选器选项还原控件选项
    setSelection(m_config.value(m_currentType));
}

/**
 * @brief FilterContent::setCurrentConfig 控件发生值改变时,改变当前类型的筛选器选项
 * @param iConifg 筛选器选项
 */
void FilterContent::setCurrentConfig(FILTER_CONFIG iConifg)
{
    m_config.insert(m_currentType, iConifg);
}

/**
 * @brief FilterContent::slot_logCatelogueClicked 连接日志种类listview  itemchanged信号,根据日志种类改变筛选控件布局
 * @param index  listview 当前选中index,用来确定日志种类
 */
void FilterContent::slot_logCatelogueClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    //获取日志种类
    QString itemData = index.data(ITEM_DATE_ROLE).toString();
    if (itemData.isEmpty()) {
        return;
    }
    setLeftButtonState(true);
    m_curTreeIndex = index;
    //根据日志种类改变布局
    this->setVisible(true);
    if (itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
        m_currentType = APP_TREE_DATA;
        this->setAppComboBoxItem();
        this->setSelectorVisible(true, true, false, true, false);
    } else if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive)) {
        m_currentType = JOUR_TREE_DATA;
        this->setSelectorVisible(true, false, false, true, false);
    } else if (itemData.contains(BOOT_TREE_DATA)) {
        m_currentType = BOOT_TREE_DATA;
        this->setSelectorVisible(false, false, true, false, false);
    } else if (itemData.contains(KERN_TREE_DATA)) {
        m_currentType = KERN_TREE_DATA;
        this->setSelectorVisible(false, false, false, true, true);
    } else if (itemData.contains(DPKG_TREE_DATA)) {
        m_currentType = DPKG_TREE_DATA;
        this->setSelectorVisible(false, false, false, true, true);
    } else if (itemData.contains(XORG_TREE_DATA, Qt::CaseInsensitive)) {
        m_currentType = XORG_TREE_DATA;
        this->setSelectorVisible(false, false, false, false,
                                 true);  // modified by Airy for showing  peroid
    } else if (itemData.contains(LAST_TREE_DATA, Qt::CaseInsensitive)) {  // add by Airy
        m_currentType = LAST_TREE_DATA;
        this->setSelectorVisible(false, false, false, true, false,
                                 true);  // modifed by Airy for showing peroid
    } else if (itemData.contains(KWIN_TREE_DATA)) {
        m_currentType = KWIN_TREE_DATA;
        this->setSelectorVisible(false, false, false, false, false);
    } else if (itemData.contains(BOOT_KLU_TREE_DATA)) {
        m_currentType = BOOT_KLU_TREE_DATA;
        this->setSelectorVisible(true, false, false, false, false);
    } else if (itemData.contains(DNF_TREE_DATA)) {
        m_currentType = DNF_TREE_DATA;
        this->setSelectorVisible(false, false, false, true, false, false, true);
    } else if (itemData.contains(DMESG_TREE_DATA, Qt::CaseInsensitive)) {
        m_currentType = DMESG_TREE_DATA;
        this->setSelectorVisible(true, false, false, true,
                                 false);
    } else if (itemData.contains(OTHER_TREE_DATA)) {
        m_currentType = OTHER_TREE_DATA;
        //this->setSelectorVisible(false, false, false, true, false, false, true);
        this->setVisible(false);
    } else if (itemData.contains(CUSTOM_TREE_DATA)) {
        m_currentType = CUSTOM_TREE_DATA;
        //this->setSelectorVisible(false, false, false, true, false, false, true);
        this->setVisible(false);
    } else if (itemData.contains(AUDIT_TREE_DATA)) {
        m_currentType = AUDIT_TABLE_DATA;
        this->setSelectorVisible(false, false, false, true, false, false, false, true);
    } else if (itemData.contains(COREDUMP_TREE_DATA)) {
        m_currentType = COREDUMP_TREE_DATA;
        this->setSelectorVisible(false, false, false, true, true, false, false, false);
    }
    updateDataState();
    //必须需要,因为会丢失当前焦点顺序
    LogListView *logList =  qobject_cast<LogListView *>(sender());
    if (logList) {
        logList->setFocus();
    }

}

/**
 * @brief FilterContent::slot_logCatelogueRefresh 日志种类选择listview的右键菜单刷新时处理的槽函数
 * @param index 刷新对应的日志种类
 */
void FilterContent::slot_logCatelogueRefresh(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QString itemData = index.data(ITEM_DATE_ROLE).toString();
    if (itemData.isEmpty()) {
        return;
    }
    setLeftButtonState(true);
    //现在只需处理应用日志刷新时需要刷新应用选择下拉列表的数据
    if (itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
        //记录当前选择项目以便改变combox内容后可以选择原来的选项刷新
        //先disconnect防止改变combox内容时发出currentIndexChanged让主表获取
        disconnect(cbx_app, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAppIdxChanged(int)));
        this->setAppComboBoxItem();

        updateDataState();
        //第一次刷新手动发出选项变化信号
        emit sigCbxAppIdxChanged(m_config.value(m_currentType).dateBtn, cbx_app->itemData(cbx_app->currentIndex(), Qt::UserRole + 1).toString());
        connect(cbx_app, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAppIdxChanged(int)), Qt::UniqueConnection);
    }

    if(itemData.contains(BOOT_TREE_DATA, Qt::CaseInsensitive)){
        Q_EMIT cbx_status->currentIndexChanged(m_config.value(m_currentType).statusCbx);
    }
}

/**
 * @brief FilterContent::slot_buttonClicked 时间筛选buttongroup值变化处理槽函数
 * @param idx 当前选择的按钮
 */
void FilterContent::slot_buttonClicked(int idx)
{
    /** note: In order to adapt to the new scene, select time-period first,
     *        then select any log item, should display current log info.
     *        so comment this judge.
     */
    QString itemData = m_curTreeIndex.data(ITEM_DATE_ROLE).toString();
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.dateBtn = idx;
    setCurrentConfig(curConfig);

    switch (idx) {
    case ALL:
    case ONE_DAY:
    case THREE_DAYS:
    case ONE_WEEK:
    case ONE_MONTH:
    case THREE_MONTHS: {
        //根据选择时间发出当前选项变化信号
        m_curBtnId = idx;
        emit sigButtonClicked(idx, m_curLvCbxId, m_curTreeIndex);
    } break;
    case RESET: {
        m_curBtnId = ALL;
        if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive) ||
                itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
            cbx_lv->setCurrentIndex(INF + 1);
            cbx_dnf_lv->setCurrentIndex(5);
        } else {
            emit sigButtonClicked(m_curBtnId, INVALID, m_curTreeIndex);
        }
    } break;
    default:
        break;
    }
}

/**
 * @brief FilterContent::slot_exportButtonClicked 导出按钮触发处理槽函数
 */
void FilterContent::slot_exportButtonClicked()
{
    QString itemData = m_curTreeIndex.data(ITEM_DATE_ROLE).toString();
    //当前日志种类不为空则发出导出数据信号给其他类处理
    if (!itemData.isEmpty())
        emit sigExportInfo();
}

/**
 * @brief FilterContent::slot_cbxLvIdxChanged 日志等级下拉框选择变化处理槽函数
 * @param idx 当前选择的选项下标
 */
void FilterContent::slot_cbxLvIdxChanged(int idx)
{
    setChangedcomboxstate(true);
    m_curLvCbxId = idx - 1;
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.levelCbx = idx;
    //变化时改变记录选择选项的数据结构,以便下次还原
    setCurrentConfig(curConfig);
    //发出信号以供数据显示控件刷新数据
    emit sigButtonClicked(m_curBtnId, m_curLvCbxId, m_curTreeIndex);
}

/**
 * @brief FilterContent::slot_cbxAppIdxChanged 应用选择下拉框选择变化处理槽函数
 * @param idx 当前选择的选项下标
 */
void FilterContent::slot_cbxAppIdxChanged(int idx)
{
    setChangedcomboxstate(!getLeftButtonState());
    QString app = cbx_app->itemData(idx, Qt::UserRole + 1).toString();
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.appListCbx = app;
    //变化时改变记录选择选项的数据结构,以便下次还原
    setCurrentConfig(curConfig);
    //发出信号以供数据显示控件刷新数据
    emit sigCbxAppIdxChanged(curConfig.dateBtn, app);

    // 初始化子模块列表
    setSubmodulesComboBoxItem(app);
}

void FilterContent::slot_cbxSubmoduleChanged(int idx)
{
    setChangedcomboxstate(true);
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.app2Submodule[curConfig.appListCbx] = cbx_submodule->currentText();
    setCurrentConfig(curConfig);
    emit sigCbxSubModuleChanged(idx);
}

/**
 * @brief FilterContent::slot_cbxStatusChanged 启动日志状态选择下拉框选择变化处理槽函数
 * @param idx 当前选择的选项下标
 */
void FilterContent::slot_cbxStatusChanged(int idx)
{
    setChangedcomboxstate(!getLeftButtonState());
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.statusCbx = idx;
    //变化时改变记录选择选项的数据结构,以便下次还原
    setCurrentConfig(curConfig);
    QString str;
    if (idx == 1)
        str = "OK";
    else if (idx == 2)
        str = "Failed";
    //发出信号以供数据显示控件刷新数据
    emit sigStatusChanged(str);
}

/**
 * @brief FilterContent::slot_cbxLogTypeChanged 开关机日志日志种类选择下拉框选择变化处理槽函数
 * @param idx 当前选择的选项下标
 */
void FilterContent::slot_cbxLogTypeChanged(int idx)
{
    setChangedcomboxstate(true);
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.typeCbx = idx;
    setCurrentConfig(curConfig);
    emit sigLogtypeChanged(idx);
}

void FilterContent::slot_cbxAuditTypeChanged(int idx)
{
    setChangedcomboxstate(true);
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.auditCbx = idx;
    setCurrentConfig(curConfig);
    emit sigAuditTypeChanged(idx);
}

/**
 * @brief FilterContent::setExportButtonEnable 导出按钮是否置灰
 * @param iEnable true 不置灰 false 置灰
 */
void FilterContent::setExportButtonEnable(bool iEnable)
{
    if (exportBtn) {
        exportBtn->setEnabled(iEnable);
    }
}

/**
 * @brief FilterContent::slot_cbxDnfLvIdxChanged dnf日志等级选择下拉框择变化处理槽函数
 * @param idx 当前选择的选项下标
 */
void FilterContent::slot_cbxDnfLvIdxChanged(int idx)
{
    Q_UNUSED(idx)
    setChangedcomboxstate(!getLeftButtonState());
    FILTER_CONFIG curConfig = m_config.value(m_currentType);
    curConfig.dnfCbx = idx;
    setCurrentConfig(curConfig);
    emit sigDnfLvlChanged(cbx_dnf_lv->currentData().value<DNFPRIORITY>());
}

bool FilterContent::getLeftButtonState()
{
    return m_bIsClickLeftlistButton;
}

bool FilterContent::getChangedcomboxstate()
{
    return m_bIsCombox;
}

void FilterContent::setLeftButtonState(bool value)
{
    m_bIsClickLeftlistButton = value;
}

void FilterContent::setChangedcomboxstate(bool value)
{
    m_bIsCombox = value;
}
