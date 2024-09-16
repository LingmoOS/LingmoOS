// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOUSESTATEWIDGET_H
#define MOUSESTATEWIDGET_H

#include <QWidget>

class MouseStateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MouseStateWidget(QWidget *parent = nullptr);

    void enableEnsureLeave(bool enable) { m_ensureLeave = enable; }
    void enableIgnoreEnterEvent(bool enable) { m_enableIgnoreEnterEvent = enable; }

    static void setIgnoreEnterEvent(bool ignore) { ignoreEnterEvent = ignore; }

    void restState();

protected:
    bool event(QEvent *event) override;

protected:
    static const int LEAVE =    0;
    static const int HOVERD =   1 << 0;
    static const int PRESSED =  1 << 1;

protected:
    int m_state;
    bool m_ensureLeave;
    bool m_enableIgnoreEnterEvent;

    static bool ignoreEnterEvent;
};

#endif