// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <DTitlebar>
//#include <DPushButton>
#include <DLabel>
#include <DProgressBar>
#include <DAbstractDialog>
#include <DDialog>

DWIDGET_USE_NAMESPACE

class ProgressDialog: public DAbstractDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    void initUI();
    void initConnect();

    void setCurrentTask(const QString &file);
    void setCurrentFile(const QString &file);
    void setProcess(double value);
    void setFinished();
    void clearprocess();

    void showDialog();

protected:
    void closeEvent(QCloseEvent *) override;

Q_SIGNALS:
    void stopExtract();
    void extractSuccess(QString msg);
    void sigResetPercentAndTime();
    /**
     * @brief signalPause 暂停信号
     */
    void signalPause();

    /**
     * @brief signalContinue    继续信号
     */
    void signalContinue();

    /**
     * @brief signalCancel       取消信号
     */
    void signalCancel();

public slots:
    void slotextractpress(int index);

private:
    int m_defaultWidth = 380;
    int m_defaultHeight = 120;
    double m_dPerent = 0.0; //保存上一次进度

    DLabel *m_tasklable;
    DLabel *m_filelable;
    DProgressBar *m_circleprogress;

private:
    DTitlebar *m_titlebar;
};

/**
 * @brief The CommentProgressDialog class  更新注释进度弹窗
 */
class CommentProgressDialog: public DAbstractDialog
{
    Q_OBJECT
public:
    explicit CommentProgressDialog(QWidget *parent = nullptr);
    void initUI();
    void showdialog();

    /**
     * @brief setProgress   设置进度值
     * @param value         进度值
     */
    void setProgress(double value);
    void setFinished();

private:
    DProgressBar *m_progressBar = nullptr;
};

#endif // PROGRESSDIALOG_H
