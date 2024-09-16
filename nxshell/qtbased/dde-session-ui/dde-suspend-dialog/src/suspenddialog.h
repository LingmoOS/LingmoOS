// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUSPENDDIALOG_H
#define SUSPENDDIALOG_H

#include <ddialog.h>

#include <QTimer>

DWIDGET_USE_NAMESPACE

class SuspendDialog : public DDialog
{
    Q_OBJECT
public:
    explicit SuspendDialog(QRect screenGeometry);
    ~SuspendDialog() {}

protected:
    void showEvent(QShowEvent *event);

private:
    QRect m_screenGeometry;

    QTimer m_timer;
    int m_timerTick;
};

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager();
    ~Manager() {}

signals:
    void finished(const int code);

private:
    void setupDialogs();
};

#endif // SUSPENDDIALOG_H
