// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPACTDISKMONITOR_H
#define COMPACTDISKMONITOR_H

#include <QWidget>
#include <QPainterPath>

class CompactDiskMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit CompactDiskMonitor(QWidget *parent = nullptr);
    ~CompactDiskMonitor();

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
    void changeFont(const QFont &font);
    void getPainterPathByData(QList<double> *listData, QPainterPath &path, qreal maxVlaue);

private:
    QList<qreal> *readSpeeds;
    QList<qreal> *writeSpeeds;
    qreal m_readBps {};
    qreal m_writeBps {};

    int m_bulletSize = 6;

    QColor m_diskReadColor {"#8F88FF"};
    QColor m_diskWriteColor {"#6AD787"};

    QPainterPath readPath;
    QPainterPath writePath;

    int renderMaxHeight = 30;

    QFont m_tagFont;
    QFont m_statFont;
};

#endif
