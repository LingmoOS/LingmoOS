// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHLISTDELEGATE_H
#define GRANDSEARCHLISTDELEGATE_H

#include <DListView>
#include <DHiDPIHelper>

#include <QScopedPointer>
#include <DStyledItemDelegate>

namespace GrandSearch {

class GrandSearchListDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GrandSearchListDelegate(QAbstractItemView *parent = Q_NULLPTR);
    ~GrandSearchListDelegate() override;

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawTailText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int textMaxWidth, int actualStartX) const;
    void drawTailDetailedInfo(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, const QColor &color, const QFont &font, const QFontMetrics &fontMetrics, int &startX) const;

    void calcTailShowInfo(const int totalTailWidth, int &tailCount, int &averageWidth) const;
    QMap<int, QString> calcTailShowData(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
    QMap<int, QString> calcTailShowDataByMaxWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
    QMap<int, QString> calcTailShowDataByOptimalWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const;
};

}

#endif // GRANDSEARCHLISTDELEGATE_H
