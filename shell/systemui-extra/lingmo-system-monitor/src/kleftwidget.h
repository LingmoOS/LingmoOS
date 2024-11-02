/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KLEFTWIDGET_H
#define KLEFTWIDGET_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <qgsettings.h>
#include <QTimer>
#include <QScrollArea>

#include "kleftsideitem.h"
#include "sysresource/cpuhistorywidget.h"
#include "sysresource/memhistorywidget.h"
#include "sysresource/nethistorywidget.h"

#include "knavigationbar.h"
using namespace kdk;

class KLeftWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KLeftWidget(QWidget *parent = nullptr);
    virtual ~KLeftWidget();

    void initUI();
    void initConnections();
    int currentIndex();
    void setTabletMode(bool b);

public slots:
    void onSwichPanel(int nIndex);
    void onResourceUpdate();

signals:
    void switchPage(int nIndex);
    void updateCpuStatus(qreal percent);
    void updateMemStatus(qreal memUsed, qreal memTotal);
    void updateSwapStatus(qreal swapUsed, qreal swapTotal);
    void onUpdateNetStatus(qreal sendTotalBytes, qreal sendRate, qreal recvTotalBytes, qreal recvRate);

private:
    void paintEvent(QPaintEvent *e);
    void initThemeMode();
    bool getSwaponData(unsigned long long &swapSize, unsigned long long &swapUsed);

private:
    QButtonGroup *m_buttonGroup = nullptr;
    KLabel  *m_labelTitle = nullptr;
    QLabel  *m_labelTitleIcon = nullptr;
    QScrollArea *m_scrollOptions = nullptr;
    QFrame *m_frameOptions = nullptr;
    QVBoxLayout *m_layoutOptions = nullptr;

    KNavigationBar *m_buttonBar = nullptr;

    CpuHistoryWidget *m_cpuHisWidget = nullptr;
    MemHistoryWidget *m_memHisWidget = nullptr;
    NetHistoryWidget *m_netHisWidget = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QVBoxLayout *m_itemsLayout = nullptr;
    QVBoxLayout *m_resLayout = nullptr;

    QGSettings *m_styleSettings = nullptr;
    QTimer *m_updateStatusTimer = nullptr;
    //cpu
    unsigned long long m_prevCpuTotalTime = 0;
    unsigned long long m_prevCpuWorkTime = 0;
    unsigned long long m_cpuTotalTime = 0;
    unsigned long long m_cpuworkTime = 0;

    //network
    unsigned long long int m_totalRecvBytes = 0;
    unsigned long long int m_totalSentBytes = 0;
    unsigned long long int m_rateRecvBytes = 0;
    unsigned long long int m_rateSentBytes = 0;
    int transparency = 0;
    bool tabletMode = false;

public:
    CpuHistoryWidget *getCpuHisWidget() {
        return m_cpuHisWidget;
    }
    MemHistoryWidget *getMemHisWidget() {
        return m_memHisWidget;
    }
    NetHistoryWidget *getNetHisWidget() {
        return m_netHisWidget;
    }
};

#endif // KLEFTWIDGET_H
