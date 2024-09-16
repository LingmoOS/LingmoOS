// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_ATTRIBUTE_DIALOG_H
#define PROCESS_ATTRIBUTE_DIALOG_H

#include <DFrame>
#include <DLabel>
#include <DMainWindow>
#include <DShadowLine>
#include <DTextBrowser>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class Settings;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;

/**
 * @brief Dialog shown to user when process attribute requested by user
 */
class ProcessAttributeDialog : public DMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Dialog Constructor
     * @param pid Process id
     * @param procName Process name
     * @param displayName Process display name
     * @param cmdline Process command line
     * @param icon Process icon
     * @param startTime Process start time
     * @param parent Parent object
     */
    explicit ProcessAttributeDialog(pid_t pid,
                                    const QString &procName,
                                    const QString &displayName,
                                    const QString &cmdline,
                                    const QIcon &icon,
                                    time_t startTime,
                                    QWidget *parent = nullptr);

private:
    /**
     * @brief Initialize ui components
     */
    void initUI();
    void resizeItemWidget();

protected:
    /**
     * @brief resizeEvent Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event);
    /**
     * @brief closeEvent Close event handler
     * @param event Close event
     */
    void closeEvent(QCloseEvent *event);
    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object been watched
     * @param event Event to be filtered
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event);

private:
    // Global settings instance
    Settings *m_settings;

    // Process name
    QString     m_procName      {};
    // Process display name
    QString     m_displayName   {};
    // Process command line
    QString     m_cmdline       {};
    // Process start time
    time_t      m_startTime     {};
    // Process icon
    QIcon       m_icon          {};

    // Background frame
    DWidget *m_frame {};
    // Shadow widget under titlebar
    DShadowLine *m_tbShadow {};

    // Process display name label
    DLabel *m_appNameLabel {};
    // Process name label
    DLabel *m_procNameLabel {};
    // Process name text
    DTextBrowser *m_procNameText {};
    // Process command label
    DLabel *m_procCmdLabel {};
    // Process command line text
    DTextBrowser *m_procCmdText {};
    // Process start time label
    DLabel *m_procStartLabel {};
    // Process start time text
    DTextBrowser *m_procStartText {};

    // Max label width
    int m_maxLabelWidth {0};
    // Max text width
    int m_maxTextWidth {0};
    // Content manrgin
    int m_margin {0};
    // Command line prefered height
    int m_cmdh {0};

    // Process id
    pid_t m_pid;
};

#endif  // PROCESS_ATTRIBUTE_DIALOG_H
