// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef ENDPAGE_H
#define ENDPAGE_H

#include <QWidget>
#include <QLabel>

class EndPage : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
        Success,
        Warning,
        Fail
    };

    explicit EndPage(Mode mode, QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setMessage(const QString &message);

private:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

    QLabel *m_title;
    QLabel *m_message;
};

#endif // ENDPAGE_H
