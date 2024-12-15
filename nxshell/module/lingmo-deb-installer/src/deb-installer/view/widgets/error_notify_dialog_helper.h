// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ERROR_NOTIFY_DIALOG_HELPER_H
#define ERROR_NOTIFY_DIALOG_HELPER_H

#include <QObject>

// popup error dialogs, support deb / uab packages
class ErrorNotifyDialogHelper : public QObject
{
    Q_OBJECT
public:
    explicit ErrorNotifyDialogHelper(QObject *parent = nullptr);
    ~ErrorNotifyDialogHelper() override = default;

    static void showHierarchicalVerifyWindow();

private:
    Q_DISABLE_COPY(ErrorNotifyDialogHelper)
};

#endif  // ERROR_NOTIFY_DIALOG_HELPER_H
