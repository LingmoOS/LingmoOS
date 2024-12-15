// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <DButtonBox>
#include <DSearchEdit>
#include <DWidget>
#include "dialog/custombuttonbox.h"

DWIDGET_USE_NAMESPACE

class MainWindow;
class QTimer;
class QAction;

/**
 * @brief Toolbar class
 */
class Toolbar : public DWidget
{
    Q_OBJECT

public:
    /**
     * @brief Toolbar constructor
     * @param parent Parent object
     */
    explicit Toolbar(QWidget *parent = nullptr);
    // Destroctor
    ~Toolbar();

    /**
     * @brief Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filtered
     * @return To filter the event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event);
    /**
     * @brief Check if search text input is empty
     * @return Return true if search input is empty, otherwise return false
     */
    bool isSearchContentEmpty();
    /**
     * @brief Get search input's text
     * @return Return searched text if none empty, otherwise return empty string
     */
    inline QString searchContent()
    {
        if (searchEdit) {
            return searchEdit->text();
        }
        return {};
    }

    void setProcessButtonChecked(bool checked);

public Q_SLOTS:
    /**
     * @brief Handle search process
     */
    void handleSearch();
    /**
     * @brief Handle search text changed event
     */
    void handleSearchTextChanged();
    /**
     * @brief Set search input focus
     */
    void focusInput();
    /**
     * @brief Clear search input content
     */
    inline void clearSearchText()
    {
        if (searchEdit) {
            searchEdit->clearEdit();
        }
    }

Q_SIGNALS:
    /**
     * @brief Search signal
     * @param searchContent Search content to be handled
     */
    void search(const QString &searchContent);
    /**
     * @brief ESC pressed signal
     */
    void pressEsc();
    /**
     * @brief Tab pressed signal
     */
    void pressTab();

    /**
     * @brief Procss tab button triggered signal
     */
    void procTabButtonClicked();
    /**
     * @brief Sevice tab button triggered signal
     */
    void serviceTabButtonClicked();
    /**
     * @brief User Procss tab button triggered signal
     */
    void accountProcTabButtonClicked();

private:
    // Button group
    CustomButtonBox *m_switchFuncTabBtnGrp {nullptr};
    // Process tab button
    DButtonBoxButton *m_procBtn {nullptr};
    // Service tab button
    DButtonBoxButton *m_svcBtn {nullptr};
    // User Process tab button
    DButtonBoxButton *m_accountProcBtn {nullptr};
    // Search text input
    DSearchEdit *searchEdit {nullptr};

    // Start timer when search content changed, do the actual search only when timer timed out
    QTimer *searchTimer;

    // Searched content cache
    QString searchTextCache;

    QWidget *m_spaceItem {nullptr};
};

#endif
