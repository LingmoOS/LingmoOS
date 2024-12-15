// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERIES_H
#define QUERIES_H

#include <DApplicationHelper>
#include <DDialog>
#include <DLabel>

#include <QMutex>
#include <QVariant>
#include <QWaitCondition>

DWIDGET_USE_NAMESPACE

struct NewStr
{
    QStringList strList;
    QString resultStr;
    int fontHeifht = 0;
};

NewStr autoCutText(const QString &text, DLabel *pDesLbl);

/**
 * @brief The OverwriteQuery_Result enum   处理文件已存在时的选项
 */
enum OverwriteQuery_Result {
    Result_Cancel = 0,   // 取消
    Result_Skip = 1,   // 跳过
    Result_SkipAll = 2,   // 全部跳过
    Result_Overwrite = 3,   // 替换
    Result_OverwriteAll = 4,   // 全部替换
    Result_Readonly = 5   // 以只读方式打开，例如损坏的分卷包
};

typedef QHash<QString, QVariant> QueryData;

class CustomDDialog : public DDialog
{
    Q_OBJECT
public:
    explicit CustomDDialog(QWidget *parent = nullptr);
    explicit CustomDDialog(const QString &title, const QString &message, QWidget *parent = 0);

    // QWidget interface
protected:
    void changeEvent(QEvent *event) override;

Q_SIGNALS:
    void signalFontChange();
};

// 询问对话框
class Query : public QObject
{
    Q_OBJECT
public:
    explicit Query(QObject *parent = nullptr);
    ~Query() override;

    /**
     * @brief setParent     设置父窗口以居中显示
     * @param pParent
     */
    void setParent(QWidget *pParent);

    /**
     * @brief waitForResponse   等待对话框做出选择
     */
    void waitForResponse();

    /**
     * @brief setResponse   设置对话框做出的选择
     * @param response      选择值
     */
    void setResponse(const QVariant &response);

    /**
     * @brief execute   创建对话框
     */
    virtual void execute() = 0;

protected:
    /**
     * @brief toShortString     字符串转换
     * @param strSrc
     * @param limitCounts
     * @param left
     * @return
     */
    QString toShortString(QString strSrc, int limitCounts, int left);

protected:
    QWidget *m_pParent = nullptr;
    QueryData m_data;

private:
    QWaitCondition m_responseCondition;
    QMutex m_responseMutex;
};

// 询问重复文件对话框
class OverwriteQuery : public Query
{
    Q_OBJECT
public:
    /**
     * @brief OverwriteQuery
     * @param filename      文件名（包含路径）
     * @param parent
     */
    explicit OverwriteQuery(const QString &filename, QObject *parent = nullptr);
    ~OverwriteQuery() override;

    /**
     * @brief execute   创建对话框并显示
     */

    void execute() override;

    /**
     * @brief responseCancelled     是否取消
     * @return
     */
    bool responseCancelled();

    /**
     * @brief responseSkip      是否跳过
     * @return
     */
    bool responseSkip();

    /**
     * @brief responseSkipAll      是否全部跳过
     * @return
     */
    bool responseSkipAll();

    /**
     * @brief responseOverwrite      是否覆盖
     * @return
     */
    bool responseOverwrite();

    /**
     * @brief responseOverwriteAll      是否全部覆盖
     * @return
     */
    bool responseOverwriteAll();

    /**
     * @brief autoFeed 自动换行
     * @param label1
     * @param label2
     * @param dialog
     */
    void autoFeed(DLabel *label1, DLabel *label2, CustomDDialog *dialog);

private:
    /**
     * @brief setWidgetColor    设置面板颜色
     * @param pWgt
     * @param ct
     * @param alphaF
     */
    void setWidgetColor(QWidget *pWgt, DPalette::ColorRole ct, double alphaF);

    /**
     * @brief setWidgetType     设置面板类型颜色
     * @param pWgt
     * @param ct
     * @param alphaF
     */
    void setWidgetType(QWidget *pWgt, DPalette::ColorType ct, double alphaF);

private:
    QString m_strDesText;
    QString m_strFileName;
    int m_iLabelOldHeight = 0;
    int m_iLabelOld1Height = 0;
    int m_iCheckboxOld1Height = 0;
    int m_iDialogOldHeight = 0;
};

// 密码输入框
class PasswordNeededQuery : public Query
{
    Q_OBJECT
public:
    /**
     * @brief PasswordNeededQuery
     * @param strFileName       文件名（含路径）
     * @param parent
     */
    explicit PasswordNeededQuery(const QString &strFileName, QObject *parent = nullptr);
    ~PasswordNeededQuery() override;

    /**
     * @brief execute   显示密码输入对话框
     */
    void execute() override;

    /**
     * @brief autoFeed 自动换行
     * @param label1
     * @param label2
     * @param dialog
     */
    void autoFeed(DLabel *label1, DLabel *label2, CustomDDialog *dialog);

    /**
     * @brief responseCancelled     是否取消
     * @return
     */
    bool responseCancelled();

    /**
     * @brief password  获取输入的密码
     * @return 输入的密码
     */
    QString password();

private:
    QString m_strDesText;
    QString m_strFileName;
    int m_iLabelOldHeight = 0;
    int m_iLabelOld1Height = 0;
    int m_iCheckboxOld1Height = 0;
    int m_iDialogOldHeight = 0;
};

// 加载非致命损坏对话框
class LoadCorruptQuery : public Query
{
    Q_OBJECT
public:
    explicit LoadCorruptQuery(const QString &archiveFilename, QObject *parent = nullptr);
    ~LoadCorruptQuery() override;
    void execute() override;

    bool responseYes();

    /**
     * @brief autoFeed 自动换行
     * @param label
     * @param dialog
     */
    void autoFeed(DLabel *label, CustomDDialog *dialog);

private:
    QString m_strDesText;
    int m_iLabelOldHeight = 0;
    int m_iDialogOldHeight = 0;
};

#endif   // QUERIES_H
