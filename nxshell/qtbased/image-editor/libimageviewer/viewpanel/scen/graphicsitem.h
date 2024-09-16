// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRAPHICSMOVIEITEM_H
#define GRAPHICSMOVIEITEM_H

#include <QGraphicsPixmapItem>
#include <QPointer>
#include <QMovie>
class QMovie;
class LibGraphicsMovieItem : public QGraphicsPixmapItem, QObject
{
public:
    explicit LibGraphicsMovieItem(const QString &fileName, const QString &suffix = nullptr, QGraphicsItem *parent = nullptr);
    ~LibGraphicsMovieItem();
    bool isValid() const;
    void start();
    void stop();

private:
    QPointer<QMovie> m_movie;
};

class LibGraphicsPixmapItem : public QGraphicsPixmapItem
{
public:
    explicit LibGraphicsPixmapItem(const QPixmap &pixmap);
    ~LibGraphicsPixmapItem() override;

    void setPixmap(const QPixmap &pixmap);

protected:
    //自绘函数
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPair<qreal, QPixmap> cachePixmap;
};

class LibGraphicsMaskItem : public QGraphicsRectItem
{
public:
    explicit LibGraphicsMaskItem(QGraphicsItem *parent = nullptr);
    ~LibGraphicsMaskItem();

    void onThemeChange(int theme);

private:
    QMetaObject::Connection conn;
};

#endif  // GRAPHICSMOVIEITEM_H
