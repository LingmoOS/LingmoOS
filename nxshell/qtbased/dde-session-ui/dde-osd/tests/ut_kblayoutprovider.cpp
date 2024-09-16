// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "kblayoutprovider.h"
#undef private

#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>

class UT_KBLayoutProvider : public testing::Test
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

    void setProvider(KBLayoutProvider *provider)
    {
        m_provider = provider;
    }

public:
    KBLayoutProvider *m_provider;
};

TEST_F(UT_KBLayoutProvider, coverageTest)
{
    KBLayoutProvider provider;

    Q_EMIT provider.m_keyboardInter->CurrentLayoutChanged("en");
    Q_EMIT provider.m_keyboardInter->UserLayoutListChanged(QStringList() << "en" << "cn");

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
    provider.describeLayout("en");

    PaintWidget w;
    w.setProvider(&provider);
    w.show();
    QTest::qWait(100);
}
