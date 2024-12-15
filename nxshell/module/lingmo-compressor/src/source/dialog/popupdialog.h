// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <DDialog>
#include <DLabel>
#include <DPasswordEdit>

DWIDGET_USE_NAMESPACE

// 提示性对话框（描述 + 确定按钮）
class TipDialog : public DDialog
{
    Q_OBJECT

public:
    explicit TipDialog(QWidget *parent = nullptr);
    ~TipDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param strDesText    描述内容
     * @param btnMsg        按钮内容
     * @param btnType       按钮类型
     * @param strToolTip    提示信息
     * @return              操作返回值
     */
    int showDialog(const QString &strDesText = "", const QString btnMsg = "", ButtonType btnType = ButtonNormal, const QString &strToolTip = "");
    /**
     * @brief autoFeed 自动换行
     * @param label
     */
    void autoFeed(DLabel *label);

protected:
    void changeEvent(QEvent *event) override;

private:
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iDialogOldHeight = 0;
};

class ConvertDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ConvertDialog(QWidget *parent = nullptr);
    ~ConvertDialog() override;

    QStringList showDialog();
    /**
     * @brief autoFeed 自动换行
     * @param label
     */
    void autoFeed(DLabel *label);

protected:
    void changeEvent(QEvent *event) override;

private:
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iLabelOld1Height = 0;
    int m_iDialogOldHeight = 0;
};


// 简单询问对话框（描述 + 多个按钮）
class SimpleQueryDialog : public DDialog
{
    Q_OBJECT

public:
    explicit SimpleQueryDialog(QWidget *parent = nullptr);
    ~SimpleQueryDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param strDesText    描述内容
     * @param btnMsg1       第一个按钮内容
     * @param btnType1      第一个按钮类型
     * @param btnMsg2       第二个按钮内容
     * @param btnType2      第二个按钮类型
     * @param btnMsg3       第三个按钮内容
     * @param btnType3      第三个按钮类型
     * @return
     */
    int showDialog(const QString &strDesText = "", const QString btnMsg1 = "", ButtonType btnType1 = ButtonNormal,
                   const QString btnMsg2 = "", ButtonType btnType2 = ButtonNormal,
                   const QString btnMsg3 = "", ButtonType btnType3 = ButtonNormal);

    /**
     * @brief autoFeed 自动换行
     * @param label
     */
    void autoFeed(DLabel *label);

protected:
    void changeEvent(QEvent *event) override;

private:
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iDialogOldHeight = 0;
};

/**
 * @brief The Overwrite_Result enum  替换对话框选择结果
 */
enum Overwrite_Result {
    OR_Cancel = 0,      // 取消
    OR_Skip = 1,        // 跳过
    OR_Overwrite = 2,   // 替换
};

// 同名文件提示是否替换
class OverwriteQueryDialog : public DDialog
{
    Q_OBJECT

public:
    explicit OverwriteQueryDialog(QWidget *parent = nullptr);
    ~OverwriteQueryDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param file   同名文件名
     * @param bDir   是否是文件夹
     */
    void showDialog(QString file, bool bDir = false);

    /**
     * @brief getDialogResult  返回对话框状态
     * @return
     */
    Overwrite_Result getDialogResult();

    /**
     * @brief getApplyAll  返回是否应用到全部文件
     * @return
     */
    bool getApplyAll();

    /**
     * @brief autoFeed 自动换行
     * @param label1
     * @param label2
     */
    void autoFeed(DLabel *label1, DLabel *label2);

protected:
    void changeEvent(QEvent *event) override;

private:
    Overwrite_Result m_retType = OR_Cancel;   // 对话框选择结果
    bool m_applyAll = false;  // 应用到全部文件

    QString m_strFilesname; // 重复文件名
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iLabelOld1Height = 0;
    int m_iCheckboxOld1Height = 0;
    int m_iDialogOldHeight = 0;
};

// 追加压缩参数对话框
class AppendDialog : public DDialog
{
    Q_OBJECT

public:
    explicit AppendDialog(QWidget *parent = nullptr);
    ~AppendDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param bMultiplePassword 是否支持输入密码
     * @return
     */
    int  showDialog(bool bMultiplePassword);

    /**
     * @brief password  获取输入的密码
     * @return
     */
    QString password();

    /**
     * @brief autoFeed 自动换行
     * @param pLabel
     */
    void autoFeed(DLabel *pLabel);

protected:
    void changeEvent(QEvent *event) override;

private:
    QString m_strPassword;
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iDialogOldHeight = 0;
    int m_iCheckboxOld1Height = 0;
    bool m_bPasswordVisible = false;
};

// 重命名对话框
class RenameDialog : public DDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = nullptr);
    ~RenameDialog() override;

    /**
     * @brief showDialog    显示对话框
     * @param strReName 重命名名字
     * @return
     */
    int  showDialog(const QString &strReName, const QString &strAlias = "", bool isDirectory = false, bool isRepeat = false);
    QString getNewNameText() const;

private:
    QString m_strName;
    bool m_isDirectory;
    DLineEdit *m_lineEdit;
    bool m_bPasswordVisible = false;
    int m_nOkBtnIndex; //确认(Ok)按钮在对话框中的索引
};
#endif // POPUPDIALOG_H
