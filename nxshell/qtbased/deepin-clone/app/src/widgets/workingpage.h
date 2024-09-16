// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef WORKINGPAGE_H
#define WORKINGPAGE_H

#include <QWidget>

#include <DWaterProgress>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class CloneJob;
class WorkingPage : public QWidget
{
    Q_OBJECT
public:
    explicit WorkingPage(const QString &from, const QString &to, QWidget *parent = 0);

    bool isError() const;
    QString errorString() const;

    void cancel() const;

signals:
    void finished();

private:
    CloneJob *m_job;

    DWaterProgress *m_progress;
    QLabel *m_writtenSizeLabel;
    QLabel *m_timeRemainingLabel;
};

#endif // WORKINGPAGE_H
