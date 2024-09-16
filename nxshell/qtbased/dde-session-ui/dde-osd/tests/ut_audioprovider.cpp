// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "audioprovider.h"
#undef private

#include <QPainter>
#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>

class UT_AudioProvider : public testing::Test
{

};

class PaintWidget : public QWidget
{

public:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        Q_UNUSED(event)
        QPainter painter(this);
        QStyleOptionViewItem option;
        option.rect = QRect(0, 0, 100, 100);
        option.state |= QStyle::State_Selected;
        m_provider->paint(&painter, option, QModelIndex());
    }

    void setProvider(AudioProvider *provider)
    {
        m_provider = provider;
    }

public:
    AudioProvider *m_provider;
};


TEST_F(UT_AudioProvider, coverageTest)
{
    AudioProvider provider;
    provider.rowCount(QModelIndex());
    provider.data(QModelIndex(), Qt::DecorationRole);
    provider.data(QModelIndex(), Qt::EditRole);
    provider.data(QModelIndex(), Qt::DisplayRole);
    provider.pixmapName();

    PaintWidget w;
    w.setProvider(&provider);
    w.show();
    QTest::qWait(100);
}
