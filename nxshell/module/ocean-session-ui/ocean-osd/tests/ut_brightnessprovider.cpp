// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "brightnessprovider.h"
#undef private

#include <QPainter>
#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>

class UT_BrightnessProvider : public testing::Test
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

    void setProvider(BrightnessProvider *provider)
    {
        m_provider = provider;
    }

public:
    BrightnessProvider *m_provider;
};


TEST_F(UT_BrightnessProvider, coverageTest)
{
    BrightnessProvider provider;
    provider.rowCount(QModelIndex());
    provider.data(QModelIndex(), Qt::DecorationRole);
    provider.data(QModelIndex(), Qt::DisplayRole);

    PaintWidget w;
    w.setProvider(&provider);
    w.show();

    QTest::qWait(100);
}
