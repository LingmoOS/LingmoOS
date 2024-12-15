// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLISTDELEGATE_H
#define APPLISTDELEGATE_H

#include <QAbstractItemDelegate>

class CalculateUtil;
class AppListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AppListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    static QPixmap dropShadow(QPixmap pixmap, int radius, const QColor &color, const QPoint &offset);

public slots:
    void setActived(bool active);

private:
    bool m_actived;
    CalculateUtil *m_calcUtil;
    QPixmap m_blueDotPixmap;
    QPixmap m_autoStartPixmap;
};

#endif // APPLISTDELEGATE_H
