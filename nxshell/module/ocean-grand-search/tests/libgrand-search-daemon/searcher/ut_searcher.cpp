// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searcher/searcher.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

class TestSearcher : public Searcher
{
public:
    TestSearcher(QObject *parent = nullptr) : Searcher(parent) {}
    QString name() const override { return ""; }
    bool isActive() const override { return false; }
    ProxyWorker *createWorker() const override { return nullptr; }
    bool action (const QString &, const QString &) override { return false; }
};

TEST(SearcherTest, ut_activate)
{
    TestSearcher ts;
    EXPECT_FALSE(ts.activate());
}
