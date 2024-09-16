#ifndef USER_PAGE_WIDGET_H
#define USER_PAGE_WIDGET_H

/*
* Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd
*
* Author:      lishiqi <lishiqi@uniontech.com>
* Maintainer:  lishiqi <lishiqi@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "accounts_widget.h"
#include <DFrame>
#include <DWidget>
#include <DApplicationHelper>
#include <DLabel>

class ProcessTableView;


DWIDGET_USE_NAMESPACE


/**
 * @brief Process & performance monitor view frame
 */
class UserPageWidget : public DFrame
{
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent Parent object
     */
    explicit UserPageWidget(DWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~UserPageWidget();

    /**
     * @brief Initialize ui components
     */
    void initUI();
    /**
     * @brief Initialize connections
     */
    void initConnections();


protected:
    /**
     * @brief Paint event handler
     */
    void paintEvent(QPaintEvent *);

public Q_SLOTS:
    void onUserChanged();

    void onTextContentChanged();

    void onThemeChanged();

    void onHeaderChanged();
private:
    void setLabelFormat(DLabel *label, DPalette::ColorType corlorType);

private:
    ProcessTableView *m_procTable = nullptr;
    AccountsWidget *m_accountListWidget = nullptr;


    DLabel *m_CPULabel = nullptr;
    DLabel *m_MemeryLabel = nullptr;
    DLabel *m_SMemLabel = nullptr;
    DLabel *m_VMemLabel = nullptr;
    DLabel *m_UploadLabel = nullptr;
    DLabel *m_DownLoadLabel = nullptr;
    DLabel *m_DiskReadLabel = nullptr;
    DLabel *m_DiskWriteLabel = nullptr;

    DLabel *m_CPUUsageSummary = nullptr;
    DLabel *m_MemeryUsageSummary = nullptr;
    DLabel *m_SMemUsageSummary = nullptr;
    DLabel *m_VMemUsageSummary = nullptr;
    DLabel *m_UploadSummary = nullptr;
    DLabel *m_DownLoadSummary = nullptr;
    DLabel *m_DiskReadSummary = nullptr;
    DLabel *m_DiskWriteSummary = nullptr;

    DApplicationHelper *m_dAppHelper = nullptr;

    QString m_currentUser {};
};



#endif // USER_PAGE_WIDGET_H
