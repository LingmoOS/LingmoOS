// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ERROR_DIALOG_H
#define ERROR_DIALOG_H

#include <DDialog>
#include <DLabel>

DWIDGET_USE_NAMESPACE

/**
 * @brief Dialog to display error messages when error occured
 */
class ErrorDialog : public DDialog
{
    Q_OBJECT

public:
    /**
     * @brief show Show error dialog
     * @param parent Parent widget
     * @param errMsg Error message
     * @param detail Error detail
     */
    static void show(QWidget *parent, const QString &errMsg, const QString &detail);

private:
    /**
     * @brief ErrorDialog private constructor
     * @param errMsg Error message
     * @param detail Error detail
     * @param parent Parent widget
     */
    ErrorDialog(const QString &errMsg, const QString &detail, QWidget *parent = nullptr);
    /**
     * @brief initUI Initialize UI elements
     */
    void initUI();

private:
    // Error message
    QString  m_errMsg       {};
    // Error detail
    QString  m_detail       {};
    // Error detail label
    DLabel  *m_detailLabel  {};
};

#endif // ERROR_DIALOG_H
