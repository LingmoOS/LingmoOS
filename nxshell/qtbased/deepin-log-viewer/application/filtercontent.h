// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILTERCONTENT_H
#define FILTERCONTENT_H
#include "structdef.h"

#include <DComboBox>
#include <DFrame>
#include <DLabel>
#include <DSuggestButton>
#include <DWidget>
#include <DPushButton>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QWidget>

class LogCombox;
class LogPeriodButton;
class LogNormalButton;
/**
 * @brief The FilterContent class 筛选控件
 */
class FilterContent : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit FilterContent(QWidget *parent = nullptr);
    ~FilterContent();

    void initUI();
    void initConnections();

    void shortCutExport();

    /**
     * @brief getLeftButtonState 获取是否点击左侧列表按钮状态
     * @return true 点击，false 没有点击
     */
    bool getLeftButtonState();

    /**
     * @brief setLeftButtonState 设置是否点击左侧列表按钮状态
     * @param value
     */
    void setLeftButtonState(bool value);

    /**
     * @brief getChangedcomboxstate 获取是否切换下拉选项
     * @return true 是，false 不是
     */
    bool getChangedcomboxstate();

    /**
     * @brief setChangedcomboxstate 设置是否切换的下拉选项
     * @param value
     */
    void setChangedcomboxstate(bool value);
protected:
    //  void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setAppComboBoxItem();
    void setSubmodulesComboBoxItem(const QString &app);

    void setSelectorVisible(bool lvCbx, bool appListCbx, bool statusCbx, bool period, bool needMove,
                            bool typecbx = false, bool dnfCbx = false, bool auditCbx = false); // modified by Airy
    void setSelection(FILTER_CONFIG iConifg);

    void setUeButtonSytle();

    void resizeWidth();
    void updateWordWrap();
    void updateDataState();
    void setCurrentConfig(FILTER_CONFIG iConifg);


signals:
    /**
     * @brief sigButtonClicked 按钮组触发信号 .包括时间筛选按钮和导出按钮
     * @param tId 按钮在按钮组中的下标 值和BUTTONID枚举对应
     * @param lId 等级筛选下拉框当前选择的下标减一 值和PRIORITY枚举对应
     * @param idx
     */
    void sigButtonClicked(int tId, int lId, QModelIndex idx);
    /**
     * @brief sigCbxAppIdxChanged 应用日志中应用选择下拉框触发信号
     * @param app 当前选择的应用项目名称
     */
    void sigCbxAppIdxChanged(int btnId, QString app);
    /**
     * @brief sigCbxSubModuleChanged 应用日志中子模塊选择下拉框触发信号
     * @param tId 下拉框当前index
     */
    void sigCbxSubModuleChanged(int tId);
    /**
     * @brief sigExportInfo 导出按钮触发信号
     */
    void sigExportInfo();
    /**
     * @brief sigStatusChanged 启动日志状态下拉框触发信号
     * @param str 当前筛选的状态字符串
     */
    void sigStatusChanged(QString str);
    /**
     * @brief sigLogtypeChanged  开关机日志事件类型下拉框触发筛选信号
     * @param tId 下拉框当前index
     */
    void sigLogtypeChanged(int tId);  // add by Airy
    /**
     * @brief sigAuditTypeChanged  审计日志审计类型下拉框触发筛选信号
     * @param tId 下拉框当前index
     */
    void sigAuditTypeChanged(int tId);
    /**
     * @brief sigResizeWidth  当前控件应有宽度信号
     * @param iWidth 计算宽度
     */
    void sigResizeWidth(int iWidth);
    void sigDnfLvlChanged(DNFPRIORITY iDnf);
public slots:
    void slot_logCatelogueClicked(const QModelIndex &index);
    void slot_logCatelogueRefresh(const QModelIndex &index);
    void slot_buttonClicked(int idx);
    void slot_exportButtonClicked();
    void slot_cbxLvIdxChanged(int idx);
    void slot_cbxAppIdxChanged(int idx);
    void slot_cbxSubmoduleChanged(int idx);
    void slot_cbxStatusChanged(int idx);
    void slot_cbxLogTypeChanged(int idx);  // add  by Airy
    void slot_cbxAuditTypeChanged(int idx);
    void setExportButtonEnable(bool iEnable);
    void slot_cbxDnfLvIdxChanged(int idx);

private:
    /**
     * @brief m_btnGroup 时间筛选按钮加导出按钮按钮组
     */
    QButtonGroup *m_btnGroup;
    /**
     * @brief lvTxt 等级筛选下拉框前面的提示文字
     */
    Dtk::Widget::DLabel *lvTxt;
    /**
     * @brief periodLabel 时间筛选按钮前面的"周期"提示文字
     */
    Dtk::Widget::DLabel *periodLabel;
    /**
        * @brief dnflvTxt dnf日志等级筛选下拉框前面的提示文字
        */
    Dtk::Widget::DLabel *dnflvTxt;
    /**
     * @brief cbx_lv 日志等级下拉框
     */
    LogCombox *cbx_lv;
    /**
     * @brief cbx_dnf_lv dnf日志等级下拉框
     */
    LogCombox *cbx_dnf_lv;
    /**
     * @brief appTxt 应用日志筛选下拉框提示文字
     */
    Dtk::Widget::DLabel *appTxt;
    /**
     * @brief cbx_app 应用日志应用筛选下拉框
     */
    LogCombox *cbx_app;
    /**
     * @brief submoduleTxt 应用日志子模块筛选下拉框提示文字
     */
    Dtk::Widget::DLabel *submoduleTxt;
    /**
     * @brief cbx_submodule 应用日志子模块筛选下拉框
     */
    LogCombox *cbx_submodule;
    /**
     * @brief statusTxt 启动日志状态筛选下拉框前面的提示文字
     */
    Dtk::Widget::DLabel *statusTxt;
    /**
     * @brief cbx_status 启动日志状态筛选
     */
    LogCombox *cbx_status;
    /**
     * @brief typeTxt 开关机日志日志种类筛选下拉框前面的提示文字
     */
    Dtk::Widget::DLabel *typeTxt;     // add by Airy
    /**
     * @brief typeCbx 开关机日志日志种类筛选下拉框
     */
    LogCombox *typeCbx;  // add by Airy
    /**
     * @brief typeTxt 审计日志审计类型筛选下拉框前面的提示文字
     */
    Dtk::Widget::DLabel *auditTypeTxt;
    /**
     * @brief typeCbx 审计日志审计类型筛选下拉框
     */
    LogCombox *auditTypeCbx;
    /**
     * @brief m_curTreeIndex 日志种类选择listview传进来的当前选择的日志种类信息
     */
    QModelIndex m_curTreeIndex;
    //时间筛选中的全部筛选项按钮
    LogPeriodButton *m_allBtn = nullptr;
    //时间筛选中的今天筛选项按钮
    LogPeriodButton *m_todayBtn = nullptr;
    //时间筛选中的三天内筛选项按钮
    LogPeriodButton *m_threeDayBtn = nullptr;
    //时间筛选中的一周内筛选项按钮
    LogPeriodButton *m_lastWeekBtn = nullptr;
    //时间筛选中的一个月内筛选项按钮
    LogPeriodButton *m_lastMonthBtn = nullptr;
    //时间筛选中的三个月内筛选项按钮
    LogPeriodButton *m_threeMonthBtn = nullptr;
    //导出按钮
    LogNormalButton *exportBtn = nullptr;
    //当前时间筛选按钮选中项 ,当前等级下拉框筛选项
    int m_curBtnId, m_curLvCbxId;
    /**
     * @brief hLayout_period 上半部分layout 包括时间筛选按钮
     */
    QHBoxLayout *hLayout_period;
    /**
     * @brief hLayout_all 下半部分layout 其他大部分控件都在里面
     */
    QHBoxLayout *hLayout_all;
    //是否为按钮省略状态
    bool m_isIndentation = false;
    /**
     * @brief m_config 日志类型对应当前筛选项的成员变量,用来记录每个日志上次选择的项目以在切换日志类型时恢复选择状态
     */
    QMap<QString, FILTER_CONFIG>m_config;
    /**
     * @brief m_currentType 当前日志类型
     */
    QString m_currentType = "";
    bool isLeval = true;
    bool m_bIsClickLeftlistButton = false;
    bool m_bIsCombox = false;
};

#endif  // FILTERCONTENT_H
