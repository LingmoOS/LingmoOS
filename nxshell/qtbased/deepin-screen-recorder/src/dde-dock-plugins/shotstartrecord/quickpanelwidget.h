// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUICKPANELWIDGET_H
#define QUICKPANELWIDGET_H

#include "commoniconbutton.h"

#include <DLabel>
#include <QWidget>
#include <QTime>
#include <QTimer>

DWIDGET_USE_NAMESPACE

// 录屏的快捷面板控件，录屏过程中将切换显示图标并记录录制时长
class QuickPanelWidget : public QWidget
{
    Q_OBJECT

public:
    enum WidgetState {
        WS_NORMAL,
        WS_ACTIVE,
    };

    enum FuctionType {
        RECORD,
        RECORDING,
    };

    QuickPanelWidget(QWidget *parent = nullptr);
    virtual ~QuickPanelWidget() override;

    void setIcon(const QIcon &icon);
    void setDescription(const QString &description);
    void changeType(FuctionType type);

    void start();
    void stop();
    void pause();

    Q_SIGNAL void clicked();

protected:
    Q_SLOT void onTimeout();
    Q_SLOT void refreshIcon();
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initUI();

private:
    CommonIconButton *m_icon;
    DLabel *m_description;

    QTimer *m_timer;
    QTime m_baseTime;
    QString m_showTimeStr;
    FuctionType m_type = RECORD;
};

#endif  // QUICKPANELWIDGET_H
