// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONTASKDIALOG_H
#define COOPERATIONTASKDIALOG_H

#include "global_defines.h"

#include <QStackedLayout>

namespace cooperation_core {

class CooperationTaskDialog : public CooperationDialog
{
    Q_OBJECT
public:
    explicit CooperationTaskDialog(QWidget *parent = nullptr);

    void switchWaitPage(const QString &title);
    void switchFailPage(const QString &title, const QString &msg, bool retry);
    void switchConfirmPage(const QString &title, const QString &msg);
    void switchInfomationPage(const QString &title, const QString &msg);

Q_SIGNALS:
    void waitCanceled();
    void retryConnected();
    void rejectRequest();
    void acceptRequest();

protected:
    void init();
    void setTaskTitle(const QString &title);
    QWidget *createWaitPage();
    QWidget *createFailPage();
    QWidget *createConfirmPage();
    QWidget *createInfomationPage();

private:
    QStackedLayout *switchLayout { nullptr };

    CooperationLabel *failMsgLabel { nullptr };
    CooperationLabel *confirmMsgLabel { nullptr };
    CooperationLabel *infoLabel { nullptr };
    CooperationSuggestButton *retryBtn { nullptr };
};

}   // namespace cooperation_core

#endif   // COOPERATIONTASKDIALOG_H
