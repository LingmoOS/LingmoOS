// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPORTPROGRESSDLG_H
#define EXPORTPROGRESSDLG_H
#include <DDialog>
#include <DProgressBar>
#include <DWidget>
DWIDGET_USE_NAMESPACE
/**
 * @brief The ExportProgressDlg class 导出进度框
 */
class ExportProgressDlg : public DDialog
{
    Q_OBJECT
public:
    explicit ExportProgressDlg(DWidget *parent = nullptr);

    void setProgressBarRange(int minValue, int maxValue);
    void updateProgressBarValue(int curValue);
signals:
    /**
     * @brief sigCloseBtnClicked 关闭或取消按钮触发时发出,用来取消与此相关的导出出逻辑
     */
    void sigCloseBtnClicked();
protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    /**
     * @brief m_pExportProgressBar 进度条
     */
    DProgressBar *m_pExportProgressBar;
};

#endif // EXPORTPROGRESSDLG_H
