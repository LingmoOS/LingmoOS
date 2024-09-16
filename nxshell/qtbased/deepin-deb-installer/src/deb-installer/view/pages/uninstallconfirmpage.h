// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNINSTALLCONFIRMPAGE_H
#define UNINSTALLCONFIRMPAGE_H

#include "view/widgets/infocontrolbutton.h"
#include "view/widgets/installprocessinfoview.h"

#include <DLabel>
#include <DPushButton>
#include <QWidget>

class UninstallConfirmPage : public QWidget
{
    Q_OBJECT

public:
    explicit UninstallConfirmPage(QWidget *parent = nullptr);

    /**
     * @brief setPackage 设置要卸载的包的名称
     * @param packageName 包的名称
     */
    void setPackage(const QString &packageName);

    /**
     * @brief setRequiredList 设置依赖当前包的依赖列表
     * @param requiredList 依赖列表
     */
    void setRequiredList(const QStringList &requiredList);

signals:

    /**
     * @brief signalUninstallAccepted 卸载确认
     */
    void signalUninstallAccepted() const;

    /**
     * @brief signalUninstallCanceled 卸载取消
     */
    void signalUninstallCanceled() const;

protected:
    /**
       @brief 展示时设置默认焦点
     */
    void showEvent(QShowEvent *e) override;

private slots:

    /**
     * @brief slotShowDetail 显示卸载信息
     */
    void slotShowDetail();

    /**
     * @brief slotHideDetail 隐藏卸载信息
     */
    void slotHideDetail();

private:
    DLabel *m_icon = nullptr;
    DLabel *m_tips = nullptr;
    QWidget *m_infoWrapperWidget = nullptr;
    InfoControlButton *m_infoControl = nullptr;
    InstallProcessInfoView *m_dependsInfomation = nullptr;
    DPushButton *m_cancelBtn = nullptr;
    DPushButton *m_confirmBtn = nullptr;

    QStringList m_requiredList = {};
    QString m_description = "";
};
#endif  // UNINSTALLCONFIRMPAGE_H
