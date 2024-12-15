// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_SERVICE_TABLE_VIEW_H
#define SYSTEM_SERVICE_TABLE_VIEW_H

#include "base/base_table_view.h"
#include "common/error_context.h"

#include <DLabel>
#include <DSpinner>

DWIDGET_USE_NAMESPACE

class SystemServiceSortFilterProxyModel;
class SystemServiceTableModel;
class SystemServiceEntry;
class MainWindow;
class ServiceManager;
class QShortcut;

/**
 * @brief Service table view
 */
class SystemServiceTableView : public BaseTableView
{
    Q_OBJECT

public:
    /**
     * @brief Service table view constructor
     * @param parent Parent object
     */
    explicit SystemServiceTableView(DWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~SystemServiceTableView() override;

    /**
     * @brief Get source model
     * @return Service source model
     */
    inline SystemServiceTableModel *getSourceModel() const
    {
        return m_model;
    }

    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filtered
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

public Q_SLOTS:
    /**
     * @brief Start service handler
     */
    void startService();
    /**
     * @brief Stop service handler
     */
    void stopService();
    /**
     * @brief Restart service handler
     */
    void restartService();
    /**
     * @brief Set service startup mode handler
     * @param autoStart Auto startup mode
     */
    void setServiceStartupMode(bool autoStart);
    /**
     * @brief Refresh service table
     */
    void refresh();

protected Q_SLOTS:
    /**
     * @brief search Service table search handler
     * @param pattern Search pattern
     */
    virtual void search(const QString &pattern);
    /**
     * @brief Show header context menu
     * @param pos Where to show context menu
     */
    void displayHeaderContextMenu(const QPoint &pos);
    /**
     * @brief Show table context menu
     * @param pos Where to show context menu
     */
    void displayTableContextMenu(const QPoint &pos);
    /**
     * @brief Load backup settings
     * @return Backup settings loaded if or not flag
     */
    bool loadSettings();
    /**
     * @brief Save backup settings
     */
    void saveSettings();

    /**
     * @brief onLoadServiceDataList
     */
    void onLoadServiceDataList();

protected:
    /**
     * @brief Return hinted size for specified column
     * @param column Column to get hinted size for
     * @return Hinted size
     */
    int sizeHintForColumn(int column) const override;
    /**
     * @brief resizeEvent Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief selectionChanged Selection change event handler
     * @param selected Selected item
     * @param deselected Deselected item
     */
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

private:
    /**
     * @brief Initialize ui components
     * @param settingsLoaded Backup settings loaded if or not boolean flag
     */
    void initUI(bool settingsLoaded = false);
    /**
     * @brief Initialize connections
     */
    void initConnections();

    /**
     * @brief Adjust search result tip label visibility and position
     */
    void adjustInfoLabelVisibility();
    /**
     * @brief Refresh status of specified service
     * @param sname Servcie name
     */
    inline void refreshServiceStatus(const QString sname);

private:
    // Service data model
    SystemServiceTableModel *m_model                {};
    // Sort & filter proxy model build upon service data model
    SystemServiceSortFilterProxyModel *m_proxyModel {};

    // Selected service name
    QVariant m_selectedSName  {};

    // Service table context menu
    DMenu *m_contextMenu                {};
    // Servie table header context menu
    DMenu *m_headerContextMenu          {};
    // Search result tip label
    DLabel *m_noMatchingResultLabel     {};
    // Loading status wait spinner widget
    DSpinner *m_spinner                 {};

    // Start service action
    QAction *m_startServiceAction           {};
    // Stop service action
    QAction *m_stopServiceAction            {};
    // Restart service action
    QAction *m_restartServiceAction         {};
    // Set service startup mode menu
    QMenu *m_setServiceStartupModeMenu      {};
    // Set auto startup action
    QAction *m_setAutoStartAction           {};
    // Set manual startup action
    QAction *m_setManualStartAction         {};
    // Refresh service table action
    QAction *m_refreshAction                {};
    // Service load state action
    QAction *m_loadStateHeaderAction        {};
    // Service active state action
    QAction *m_activeStateHeaderAction      {};
    // Service sub state action
    QAction *m_subStateHeaderAction         {};
    // Service state action
    QAction *m_stateHeaderAction            {};
    // Service description action
    QAction *m_descriptionHeaderAction      {};
    // Service pid action
    QAction *m_pidHeaderAction              {};
    // Service startup mode action
    QAction *m_startupModeHeaderAction      {};

    // Refresh shortcut
    QShortcut *m_refreshKP      {};
    // Start service shortcut
    QShortcut *m_startKP        {};
    // Stop service shortcut
    QShortcut *m_stopKP         {};
    // Restart service shortcut
    QShortcut *m_restartKP      {};

    // Service loading status flag
    bool m_loading {false};
};

#endif  // SYSTEM_SERVICE_TABLE_VIEW_H
