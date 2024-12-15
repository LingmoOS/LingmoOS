// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "osdprovider.h"
#undef private

#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>

class UT_OSDProvider : public testing::Test
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

    void setProvider(OSDProvider *provider)
    {
        m_provider = provider;
    }

public:
    OSDProvider *m_provider;
};

TEST_F(UT_OSDProvider, coverageTest)
{
    OSDProvider provider;
    provider.highlightCurrent();
    provider.highlightNext();
    provider.rowCount(QModelIndex(0, 0, nullptr, nullptr));
    provider.data(QModelIndex(0, 0, nullptr, nullptr), Qt::DecorationRole);
    provider.data(QModelIndex(0, 0, nullptr, nullptr), Qt::DisplayRole);
    provider.checkConditions();
    provider.contentMargins();
    provider.contentSize();
    provider.flow();
    provider.sync();
    provider.match("SwitchWM2D");

    PaintWidget w;
    w.setProvider(&provider);
    w.show();
    QTest::qWait(100);
}
