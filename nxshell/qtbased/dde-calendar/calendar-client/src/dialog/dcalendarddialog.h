// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALENDARDDIALOG_H
#define DCALENDARDDIALOG_H

#include <DDialog>

DWIDGET_USE_NAMESPACE

class DCalendarDDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DCalendarDDialog(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    virtual void updateDateTimeFormat();
signals:

public slots:
    void setTimeFormat(int value);
    void setDateFormat(int value);
protected:
    QString m_timeFormat = "hh:mm";
    QString m_dateFormat = "yyyy-MM-dd";
};

#endif // DCALENDARDDIALOG_H
