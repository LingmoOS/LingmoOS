// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONDIALOG_H
#define COOPERATIONDIALOG_H

#include "global_defines.h"

#include <QDialog>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedLayout>

namespace cooperation_core {

class ConfirmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfirmWidget(QWidget *parent = nullptr);

    void setDeviceName(const QString &name);

Q_SIGNALS:
    void accepted();
    void rejected();

private:
    void init();

    CooperationLabel *msgLabel { nullptr };
    QPushButton *acceptBtn { nullptr };
    QPushButton *rejectBtn { nullptr };
};

class ProgressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressWidget(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setProgress(int value, const QString &msg);

Q_SIGNALS:
    void canceled();

private:
    void init();

    CooperationLabel *titleLabel { nullptr };
    CooperationLabel *msgLabel { nullptr };
    QProgressBar *progressBar { nullptr };
    QPushButton *cancelBtn { nullptr };
};

class ResultWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResultWidget(QWidget *parent = nullptr);

    void setResult(bool success, const QString &msg, bool view = true);

    bool getResult() const;

Q_SIGNALS:
    void completed();
    void viewed();

private:
    void init();

    CooperationLabel *iconLabel { nullptr };
    CooperationLabel *msgLabel { nullptr };
    QPushButton *okBtn { nullptr };
    QPushButton *viewBtn { nullptr };
    bool res;
};

class WaitConfirmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaitConfirmWidget(QWidget *parent = nullptr);

    void startMovie();

Q_SIGNALS:
    void canceled();

private:
    void init();

    CooperationSpinner *spinner { nullptr };
};

class CooperationTransDialog : public CooperationDialog
{
    Q_OBJECT
public:
    explicit CooperationTransDialog(QWidget *parent = nullptr);

    void showConfirmDialog(const QString &name);
    void showWaitConfirmDialog();
    void showResultDialog(bool success, const QString &msg, bool view = true);
    void showProgressDialog(const QString &title);

    void updateProgress(int value, const QString &msg);

protected:
    void closeEvent(QCloseEvent *e) override;

Q_SIGNALS:
    void cancel();
    void cancelApply();
    void viewed();
    void retry();

private:
    void init();

    QStackedLayout *mainLayout { nullptr };
    ConfirmWidget *confirmWidget { nullptr };
    WaitConfirmWidget *waitconfirmWidget { nullptr };
    ProgressWidget *progressWidget { nullptr };
    ResultWidget *resultWidget { nullptr };
};

}   // namespace cooperation_core

#endif   // COOPERATIONDIALOG_H
