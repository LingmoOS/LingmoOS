// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "displaymodeprovider.h"
#include <QModelIndex>
#undef private

#include <QPainter>
#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>

class UT_DisplayModeProvider : public testing::Test
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

    void setProvider(DisplayModeProvider *provider)
    {
        m_provider = provider;
    }

public:
    DisplayModeProvider *m_provider;
};

TEST_F(UT_DisplayModeProvider, coverageTest)
{
    DisplayModeProvider provider;
    provider.highlightCurrent();
    provider.highlightNext();
    provider.rowCount(QModelIndex(0, 0, nullptr, nullptr));
    provider.data(QModelIndex(0, 0, nullptr, nullptr), Qt::DecorationRole);
    provider.data(QModelIndex(0, 0, nullptr, nullptr), Qt::DisplayRole);
    provider.checkConditions();
    provider.contentMargins();
    provider.contentSize();
    provider.flow();
    // FIXME 会导致息屏几秒钟
    provider.m_state = DisplayModeProvider::DISABLE;
    provider.sync(QModelIndex(0, 0, nullptr, nullptr));

    PaintWidget w;
    w.setProvider(&provider);
    w.show();
    QTest::qWait(100);
}
