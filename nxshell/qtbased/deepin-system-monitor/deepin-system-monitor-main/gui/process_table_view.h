// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_TABLE_VIEW_H
#define PROCESS_TABLE_VIEW_H

#include "base/base_table_view.h"
#include "process/process_set.h"

#include <DLabel>
#include <DTreeView>




DWIDGET_USE_NAMESPACE
using namespace core::process;

class ProcessTableModel;
class ProcessSortFilterProxyModel;
class ProcessItemDelegate;
class ProcessTableHeaderView;
class QShortcut;

/**
 * @brief Process table view
 */
class ProcessTableView : public BaseTableView
{
    Q_OBJECT

public:
    /**
     * @brief Process table view constructor
     * @param parent Parent object
     */
        explicit ProcessTableView(DWidget *parent = nullptr, QString userName = nullptr);
    /**
     * @brief Destructor
     */
    ~ProcessTableView() override;

    /**
     * @brief get the process name through the process ID
     * @param pid process ID
     * @return process name of pid
     */
    QString getProcessName(int pid);

    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filterted
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;
    void setUserModeName(const QString &userName);
    inline qreal getUserCPUUsage() {return m_cpuUsage;}
    inline qreal getUserMemUsage() {return m_memUsage;}
    inline qreal getUserDownload() {return m_download;}
    inline qreal getUserUpload() {return m_upload;}
    inline qreal getUserSMemUsage() {return m_smemUsage;}
    inline qreal getUserVMemUsage() {return m_vmemUsage;}
    inline qreal getUserDiskRead() {return m_diskread;}
    inline qreal getUserDiskWrite() {return m_diskwrite;}

public Q_SLOTS:
    /**
     * @brief End process handler
     */
    void endProcess();
    /**
     * @brief Pause process handler
     */
    void pauseProcess();
    /**
     * @brief Resume process handler
     */
    void resumeProcess();
    /**
     * @brief Open process bin dir handler
     */
    void openExecDirWithFM();
    /**
     * @brief Show process attribute handler
     */
    void showProperties();
    /**
     * @brief Kill process handler
     */
    void killProcess();
    /**
     * @brief Filter process handler
     * @param text Text to be filtered out
     */
    void search(const QString &text);
    /**
     * @brief Switch process display mode
     * @param type Process display mode
     */
    void switchDisplayMode(FilterType type);
    /**
     * @brief Change process priority handler
     * @param priority Process priority
     */
    void changeProcessPriority(int priority);

    /**
     * @brief onThemeTypeChanged
     */
    void onThemeTypeChanged();
Q_SIGNALS:
    void signalModelUpdated();
    void signalHeadchanged();

protected:
    /**
     * @brief Load process table view backup settings
     * @return Settings load success or not
     */
    bool loadSettings(const QString &flag);
    /**
     * @brief Backup process table view settings
     */
    void saveSettings();
    /**
     * @brief Initialize ui components
     * @param Backup settings loaded or not flag
     */
    void initUI(bool settingsLoaded);
    /**
     * @brief Initialize connections
     * @param Backup settings loaded or not flag
     */
    void initConnections(bool settingsLoaded);
    /**
     * @brief Show process table view context menu
     * @param p Where to show context menu
     */
    void displayProcessTableContextMenu(const QPoint &p);
    /**
     * @brief Show process table header view context menu
     * @param p Where to show context menu
     */
    void displayProcessTableHeaderContextMenu(const QPoint &p);

    /**
     * @brief resizeEvent Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief Show event handler
     * @param event Show event
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief selectionChanged Selection changed event handler
     * @param selected Selected items
     * @param deselected Deselected items
     */
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;
    /**
     * @brief sizeHintForColumn Size hint for specific header view column
     * @param column Header view column
     * @return Hinted size for column
     */
    int sizeHintForColumn(int column) const override;

private:
    /**
     * @brief Adjust search result tip label's position & visibility
     */
    void adjustInfoLabelVisibility();
    /**
     * @brief Customize process priority handler
     */
    void customizeProcessPriority();

private:
    // Process model for process table view
    ProcessTableModel *m_model {};
    // Sort & filter model build upon process model
    ProcessSortFilterProxyModel *m_proxyModel {};

    // Process table view context menu
    DMenu *m_contextMenu {};
    // Process table header view context menu
    DMenu *m_headerContextMenu {};
    // Search result tip label
    DLabel *m_notFoundLabel {};
    // Item delegate for table view
    ProcessItemDelegate *m_itemDelegate {};
    // Header view for table view
    ProcessTableHeaderView *m_headerView {};

    // Currently selected PID
    QVariant m_selectedPID {};

    // End process shortcut
    QShortcut *m_endProcKP {};
    // Pause process shortcut
    QShortcut *m_pauseProcKP {};
    // Resume process shortcut
    QShortcut *m_resumeProcKP {};
    // View process property shortcut
    QShortcut *m_viewPropKP {};
    // Kill process shortcut
    QShortcut *m_killProcKP {};

    //User mode User Name
    QString m_useModeName {};
    qreal m_cpuUsage {};
    qreal m_memUsage {};
    qreal m_download {};
    qreal m_upload {};
    qreal m_smemUsage {};
    qreal m_vmemUsage {};
    qreal m_diskread {};
    qreal m_diskwrite {};
};

#endif  // PROCESS_TABLE_VIEW_H
