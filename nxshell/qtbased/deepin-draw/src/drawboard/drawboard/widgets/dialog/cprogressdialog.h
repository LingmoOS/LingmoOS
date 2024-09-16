// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPROGRESSDIALOG_H
#define CPROGRESSDIALOG_H

#include <globaldefine.h>

#include <QWidget>
#include <QProgressBar>

#include <QWidget>
#include <QLabel>

#ifdef USE_DTK
#include <DDialog>
DWIDGET_USE_NAMESPACE
#define PROGRESSSDIALOG_INHERIT DDialog
#define CABSTRACTPROCESSDIALOG_INHERIT DAbstractDialog
#else
#include <QDialog>
#define PROGRESSSDIALOG_INHERIT QDialog
#define CABSTRACTPROCESSDIALOG_INHERIT QDialog
#endif

class DRAWLIB_EXPORT ProgressDialog : PROGRESSSDIALOG_INHERIT
{
    Q_OBJECT
public:
    explicit ProgressDialog(const QString &text = "", QWidget *parent = nullptr);
    int exec() override;

public slots:
    void setText(const QString &text);
    void setProcess(int process, int total);

private:
    QProgressBar *m_progressBar;

private:
    void initUI();

private:
    QLabel *_titleLabel  = nullptr;
};

class CAbstractProcessDialog: public CABSTRACTPROCESSDIALOG_INHERIT
{
    Q_OBJECT
public:
    explicit CAbstractProcessDialog(QWidget *parent = nullptr);

public slots:
    void     setTitle(const QString &title);
    void     setProcess(int process);

protected:
    void     paintEvent(QPaintEvent *event);

protected:
    QLabel       *_titleLabel  = nullptr;
    QProgressBar *_progressBar = nullptr;
};

#endif // CPROGRESSDIALOG_H
