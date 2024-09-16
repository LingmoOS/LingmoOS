// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPACTNETWORKMONITOR_H
#define COMPACTNETWORKMONITOR_H

#include <DApplicationHelper>

#include <QWidget>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

class CompactNetworkMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit CompactNetworkMonitor(QWidget *parent = nullptr);
    ~CompactNetworkMonitor();

signals:
    void clicked(QString msgCode);

public slots:
    void updateStatus();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;

    //!
    //! \brief mouseMoveEvent 目前的策略是屏蔽鼠标移动拖拽的响应操作
    //! \param event
    //!
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;


private:
    void changeTheme(DApplicationHelper::ColorType themeType);
    void changeFont(const QFont &font);
    void getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue);

private:
    QList<double> *downloadSpeeds;
    QList<double> *uploadSpeeds;
    QPainterPath downloadPath;
    QPainterPath uploadPath;

    QColor recvColor {"#E14300"};
    QColor sentColor {"#004EEF"};
    QColor m_recvIndicatorColor {"#E14300"};
    QColor m_sentIndicatorColor {"#004EEF"};
    QColor summaryColor;
    QColor textColor;
    QColor m_frameColor;

    QFont m_sectionFont;
    QFont m_contentFont;
    QFont m_subContentFont;

    int renderMaxHeight = 20;

    qreal m_recvBps {};
    qreal m_sentBps {};
    qulonglong m_totalRecvBytes {};
    qulonglong m_totalSentBytes {};
};

#endif
