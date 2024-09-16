// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QLabel>
#include <QWidget>

class SystemMonitor : public QWidget
{
    Q_OBJECT
public:
    enum State {
        Enter,
        Leave,
        Press,
        Release
    };

    explicit SystemMonitor(QWidget *parent = nullptr);

    inline State state() { return m_state; }
    void setState(const State state);

signals:
    void requestShowSystemMonitor();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();

private:
    QLabel *m_icon;
    QLabel *m_text;
    State m_state;
};

#endif // SYSTEMMONITOR_H
