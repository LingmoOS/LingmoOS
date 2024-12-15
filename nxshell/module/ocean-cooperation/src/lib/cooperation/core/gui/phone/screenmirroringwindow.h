// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENMIRRIORINGWINDOW_H
#define SCREENMIRRIORINGWINDOW_H

#include "global_defines.h"

#include <QWindow>
#include <QPainter>
#include <QPixmap>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScreen>
#include <DMainWindow>
#include <DTitlebar>
#include <DIconButton>
#include <DButtonBox>

#include <QVBoxLayout>
#include <QApplication>
#include <QStackedLayout>

namespace cooperation_core {

class VncViewer;
class ScreenMirroringWindow : public CooperationMainWindow
{
    Q_OBJECT

public:
    explicit ScreenMirroringWindow(const QString &device, QWidget *parent = nullptr);
    ~ScreenMirroringWindow();

    void initTitleBar(const QString &device);
    void initBottom();
    void initWorkWidget();
    void initSizebyView(QSize &viewSize);

    void connectVncServer(const QString &ip, int port, const QString &password);

Q_SIGNALS:
    void buttonClicked(int opertion);

public slots:
    void handleSizeChange(const QSize &size);

protected:
    void changeEvent(QEvent *event);

private:
    QStackedLayout *stackedLayout { nullptr };
    QWidget *bottomWidget { nullptr };
    QWidget *workWidget { nullptr };
    QSize m_expectSize;  // record the size that vnc view expected
    QSize m_previousSize;  // record the normal window size

    QPoint lastPos;
    bool isDragging = false;
    bool initShow = false;
    VncViewer *m_vncViewer { nullptr };

    const int BOTTOM_HEIGHT = 56;
};

class LockScreenWidget : public QWidget
{
public:
    LockScreenWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}   // namespace cooperation_core

#endif   // SCREENMIRRIORINGWINDOW_H
