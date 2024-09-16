// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailwidget.h"
#include "detailitem.h"

#include <QLocale>

#include <DFontSizeManager>

using namespace GrandSearch;

DWIDGET_USE_NAMESPACE

DetailWidget::DetailWidget(QWidget *parent)
    : DWidget(parent)
{
    m_detailItems.clear();

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(1);

    this->setLayout(m_mainLayout);

    // 暂不考虑软件使用过程中语言切换，只获取一次以提升效率
    QString systemName = QLocale::system().name().simplified();

    // 中文环境下，标签与内容分别对齐
    if (systemName.startsWith("zh"))
        m_alignment = true;
}

void DetailWidget::setDetailInfoList(const DetailInfoList &list)
{
    qDeleteAll(m_detailItems);
    m_detailItems.clear();

    int maxTagWidth = 0;
    for (int i = 0; i < list.count(); i++) {

        auto item = new DetailItem(this);

        if (0 == i)                     // 第一行绘制上圆角
            item->setTopRound(true);
        if (list.count() - 1 == i)      // 最后一行绘制下圆角
            item->setBottomRound(true);

        item->setDetailInfo(list.at(i));

        // 获取最大标签宽度
        maxTagWidth = item->tagWidth() > maxTagWidth ? item->tagWidth() : maxTagWidth;

        m_detailItems.append(item);
        m_mainLayout->addWidget(item);
    }

    // 标签与内容分别对齐
    if (m_alignment) {
        for (auto item : m_detailItems) {
            // 固定标签宽度，内容宽度通过内部布局器自动计算
            item->setTagWidth(maxTagWidth);
        }
    }

}
