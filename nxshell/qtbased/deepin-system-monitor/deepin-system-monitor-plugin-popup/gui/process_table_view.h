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
    explicit ProcessTableView(DWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~ProcessTableView() override;

    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filterted
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

public Q_SLOTS:
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
     * @brief onThemeTypeChanged
     */
    void onThemeTypeChanged();

protected:
    /**
     * @brief Initialize ui components
     * @param Backup settings loaded or not flag
     */
    void initUI();
    /**
     * @brief Initialize connections
     * @param Backup settings loaded or not flag
     */
    void initConnections();

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

private:
    // Process model for process table view
    ProcessTableModel *m_model {};
    // Sort & filter model build upon process model
    ProcessSortFilterProxyModel *m_proxyModel {};
    // Search result tip label
    DLabel *m_notFoundLabel {};
    // Item delegate for table view
    ProcessItemDelegate *m_itemDelegate {};

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
};

#endif  // PROCESS_TABLE_VIEW_H
