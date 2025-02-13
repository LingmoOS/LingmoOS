/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef PUSHPULLDIALOG_H
#define PUSHPULLDIALOG_H

#include "quickdialog.h"

class PushPullDialog : public HUDDialog
{
    Q_OBJECT
public:
    PushPullDialog(KTextEditor::MainWindow *mainWindow, const QString &repo);

    enum Mode { Push, Pull };
    void openDialog(Mode m);

    Q_SIGNAL void runGitCommand(const QStringList &args);

private:
    QString buildPushString();
    QString buildPullString();
    void loadLastExecutedCommands();
    void saveCommand(const QString &command);

    QString getLastPushPullCmd(Mode m) const;

    QString m_repo;
    QStringList m_lastExecutedCommands;

protected Q_SLOTS:
    void slotReturnPressed(const QModelIndex &index) override;
};

#endif // PUSHPULLDIALOG_H
