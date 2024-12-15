// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULETYPEEDITDLG_H
#define SCHEDULETYPEEDITDLG_H

#include "colorseletorwidget.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "daccount.h"
#include "accountitem.h"

#include <DDialog>
#include <DLineEdit>
#include <DFileChooserEdit>

DWIDGET_USE_NAMESPACE

class ScheduleTypeEditDlg : public DDialog
{
    Q_OBJECT
public:
    enum DialogType
    {                    // 对话框类型
        DialogNewType,   // 新建日程类型
        DialogEditType,  // 编辑日程类型
        DialogImportType // 导入日程类型
    };
    //新增
    explicit ScheduleTypeEditDlg(QWidget *parent = nullptr);
    //iJobTypeNo==0 ？ 新增 or 修改
    explicit ScheduleTypeEditDlg(const DScheduleType &jobTypeOld, QWidget *parent = nullptr);
    // 新增 or 导入
    explicit ScheduleTypeEditDlg(const DialogType &type, QWidget *parent = nullptr);

    DScheduleType newJsonType();

    void setAccount(AccountItem::Ptr account);

private:
signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void signalRefreshScheduleType();
public slots:
    //编辑器文本改变事件
    void slotEditTextChanged(const QString &);

    /**
     * @brief slotFocusChanged      输入框焦点改变处理
     * @param onFocus
     */
    void slotFocusChanged(bool onFocus);
    /**
     * @brief slotBtnCancel 取消按钮
     */
    void slotBtnCancel();

    /**
     * @brief slotBtnNext 保存按钮，检查输入，保存、更新日程类型信息
     */
    void slotBtnNext();
    /**
     * @brief slotEditingFinished       编辑框编辑结束后处理
     */
    void slotEditingFinished();

    // 获取ICS文件
    QString getIcsFile();

    // 检查确认按钮是否需要禁用
    void slotCheckConfirmBtn();

private:
    void init();
    void initView();
    void initData();
private:
    DScheduleType m_jobTypeOld; //被修改的日程类型
    DScheduleType m_jobTypeNew; //修改后的日程类型
    QString m_title = "";   //弹窗名
    DLineEdit *m_lineEdit = nullptr; // 类型名编辑框
    DFileChooserEdit *m_fileEdit = nullptr; // ics文件编辑框
    QLabel *m_titleLabel = nullptr; //弹窗名控件
    ColorSeletorWidget *m_colorSeletor = nullptr; //颜色选择器
    DialogType m_dialogType;
    QString m_typeText; //输入框上一次输入后的文本

};

#endif // SCHEDULETYPEEDITDLG_H
