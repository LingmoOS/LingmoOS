// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISKHEALTHHEADERVIEW_H
#define DISKHEALTHHEADERVIEW_H

#include <DHeaderView>
#include <DStyle>

DWIDGET_USE_NAMESPACE

class DiskHealthHeaderView : public DHeaderView
{
    Q_OBJECT
public:
    explicit DiskHealthHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    int sectionSizeHint(int logicalIndex) const;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *e) override;
    void paintSection(QPainter *painter, const QRect &rect,
                      int logicalIndex) const override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    int m_spacing {1};
};

#endif // DISKHEALTHHEADERVIEW_H
