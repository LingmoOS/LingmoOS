// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H


#include <QWidget>

class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);
    void setImage(const QImage &img);
    void setRectInImage(const QRect &r);
    void setAlwaysHidden(bool value);
    bool isAlwaysHidden() const;
    bool checkbgisdark(QImage &img) const;
    QPoint transImagePos(QPoint pos);

Q_SIGNALS:
    void requestMove(int x, int y);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    void tryMoveRect(const QPoint &p);
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    bool m_hide = false;
    qreal m_imageScale = 1.0;

    qreal m_widthScale {1.0};
    qreal m_heightScale {1.0};

    QImage m_img;
    QPixmap m_pix;
    QRectF m_r;          // Image visible rect
    QRect m_mainRect;
    QRect m_originRect;

    QRect imageDrawRect;

    QString m_bgImgUrl;
    QColor m_BgColor;
    QColor m_mrBgColor;
    QColor m_mrBorderColor;
    QColor m_imgRBorderColor;
};

#endif // NAVIGATIONWIDGET_H
