// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KILL_PROCESS_CONFIRM_DIALOG_H
#define KILL_PROCESS_CONFIRM_DIALOG_H

#include <DDialog>

DWIDGET_USE_NAMESPACE

/**
 * @brief Dialog shown when process kill operation requested
 */
class KillProcessConfirmDialog : public DDialog
{
    Q_OBJECT

public:
    /**
     * @brief KillProcessConfirmDialog Default constructor
     * @param parent Parent object
     */
    explicit KillProcessConfirmDialog(QWidget *parent = nullptr);

    /**
     * @brief result Get standard button enum result
     * @return Standard button enum result
     */
    inline int result() const { return m_result; }
    /**
     * @brief setResult Set standard button enum result
     * @param r Standard button enum result
     */
    inline void setResult(int r) { m_result = r; }

public Q_SLOTS:
    /**
     * @brief onButtonClicked Button click event handler
     * @param index Button index
     * @param text Button text
     */
    void onButtonClicked(int index, const QString &text);

private:
    // Standard button enum result
    int m_result {0};
};

#endif  // KILL_PROCESS_CONFIRM_DIALOG_H
