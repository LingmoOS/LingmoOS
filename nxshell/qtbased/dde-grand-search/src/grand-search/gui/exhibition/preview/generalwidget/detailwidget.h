// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include "../previewplugin.h"

#include <DWidget>

#include <QVBoxLayout>
#include <QScrollArea>

namespace GrandSearch {

class DetailItem;

class DetailWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit DetailWidget(QWidget *parent = nullptr);

    void setDetailInfoList(const DetailInfoList &list);

private:
    QList<DetailItem *> m_detailItems;

    QVBoxLayout *m_mainLayout = nullptr;

    bool m_alignment = false;
};

}

#endif // DETAILWIDGET_H
