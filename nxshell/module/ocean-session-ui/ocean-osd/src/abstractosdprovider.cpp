// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractosdprovider.h"

AbstractOSDProvider::AbstractOSDProvider(QObject *parent)
    : QObject(parent)
    , m_isVisible(false)
{

}

bool AbstractOSDProvider::match(const QString &param)
{
    if (param.contains("Ash")) {
        m_isVisible = false;
    } else {
        m_isVisible = true;
    }
    return m_suitableParams.contains(param);
}

bool AbstractOSDProvider::checkConditions() const
{
    return true;
}

QSize AbstractOSDProvider::contentSize() const
{
    return QSize(ImageTextItemWidth, ImageTextItemHeight);
}

QMargins AbstractOSDProvider::contentMargins() const
{
    return QMargins();
}

QListView::Flow AbstractOSDProvider::flow() const
{
    return QListView::TopToBottom;
}

void AbstractOSDProvider::highlightCurrent()
{

}

void AbstractOSDProvider::highlightNext()
{

}

void AbstractOSDProvider::sync()
{

}

void AbstractOSDProvider::sync(const QModelIndex &index)
{
    Q_UNUSED(index);
}

int AbstractOSDProvider::currentRow()
{
    return -1;
}
