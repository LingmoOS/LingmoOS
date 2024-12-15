// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include <QObject>

/**
 * @brief The SaveDialog class
 * 保存提示窗口
 */
class SaveDialog : public QObject
{
    Q_DISABLE_COPY(SaveDialog)
    Q_OBJECT
public:
    explicit SaveDialog(QObject *parent = nullptr);

    /**
     * @brief showExitDialog
     * 退出保存提示窗口
     * @return
     */
    static int showExitDialog(QString fileName, QWidget *parent = nullptr);

    /**
     * @brief showTipDialog
     * 提示窗口
     * @param content
     * @return
     */
    static int showTipDialog(const QString &content, QWidget *parent = nullptr);
};

#endif // SAVEDIALOG_H
