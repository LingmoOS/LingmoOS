// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILITEM_H
#define DETAILITEM_H

#include "../previewplugin.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QColor>

#include <DLabel>

namespace GrandSearch {

class ReplicableLabel;

class DetailItem : public QWidget
{
    Q_OBJECT
public:
    explicit DetailItem(QWidget *parent = nullptr);
    ~DetailItem();

    void setDetailInfo(const DetailInfo &info);

    void setTopRound(bool round);
    bool topRound() const;

    void setBottomRound(bool round);
    bool bottomRound() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setRadius(int radius);
    int radius() const;

    void setTagWidth(int width);
    int tagWidth() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QColor getTagColor();
    QColor getContentColor();
protected:
    bool m_topRound = false;
    bool m_bottomRound = false;
    QColor m_backgroundColor;
    int m_radius = 0;

    Dtk::Widget::DLabel *m_tagLabel = nullptr;
    ReplicableLabel *m_contentLabel = nullptr;

    QHBoxLayout *m_mainLayout = nullptr;

};

}

#endif // DETAILITEM_H
