// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRAMMERITEMWIDGET_H
#define PROGRAMMERITEMWIDGET_H

#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>

#include <DGuiApplicationHelper>
#include <DIconButton>


DWIDGET_USE_NAMESPACE

class ProgrammerItemWidget : public QWidget
{
    Q_OBJECT
public:
    ProgrammerItemWidget(QString label, const int width, const QString path, QWidget *parent = nullptr);
    ProgrammerItemWidget(QString label, QWidget *parent = nullptr);
    ~ProgrammerItemWidget() override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void cleanHoverState();
    void cleanFocusState();
    void isMarkHide(bool ishide);

public slots:
    void themetypechanged(int type);
    void setFocus();
    void resetWidth(const int width); // 重设宽度，用于字体改变的时候

private:
    void drawMark(QPainter *painter) const;
    void initMark();
    void drawIcon(QPainter *painter) const;

private:
    DIconButton *m_iconbtn;
    QLabel *m_label;
    bool m_ispress = false;
    bool m_ishover = false;
    bool m_isfocus = false;
    int m_themetype = 1;
    QSvgRenderer *m_iconRenderer;
    bool m_isshift = false; //判断是数据的列表还是移位的列表
    int m_width = 0;//宽度

    //画一个勾
    QPoint m_markstart;
    QPoint m_markmid;
    QPoint m_markend;
    QVector<QPoint> m_pointVec;
    QPen m_markPen;
    bool m_isMarkHide = true;

    //画一张图
    QPixmap m_pixmap;
    QString m_normalUrl = QString();
    QString m_hoverUrl;
    QString m_pressUrl;
    QString m_currentUrl = QString();
};

#endif // PROGRAMMERITEMWIDGET_H
