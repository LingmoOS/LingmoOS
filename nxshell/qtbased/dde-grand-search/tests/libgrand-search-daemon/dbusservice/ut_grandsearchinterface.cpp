// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "dbusservice/grandsearchinterface.h"
#include "dbusservice/grandsearchinterface_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(GrandSearchInterface, ut_init)
{
    GrandSearchInterface  gsi;
    stub_ext::StubExt st;
    bool inited = false;
    st.set_lamda(&MainController::init, [&inited]() {
        inited = true;
        return true;
    });

    EXPECT_TRUE(gsi.init());
    EXPECT_TRUE(inited);
    EXPECT_TRUE(gsi.d->m_deadline.isSingleShot());
    EXPECT_NE(gsi.d->m_main, nullptr);
}

TEST(GrandSearchInterface, ut_init_1)
{
    GrandSearchInterface gsi;
    stub_ext::StubExt st;
    bool inited = false;
    st.set_lamda(&MainController::init, [&inited]() {
        inited = true;
        return false;
    });

    EXPECT_FALSE(gsi.init());
    EXPECT_TRUE(inited);
    EXPECT_NE(gsi.d->m_main, nullptr);

    bool terminate = false;
    st.set_lamda(&GrandSearchInterfacePrivate::terminate, [&terminate]() {
        terminate = true;
    });

    emit gsi.d->m_deadline.timeout(QTimer::QPrivateSignal());
    EXPECT_TRUE(terminate);

    //测试直连
    bool matched = false;
    st.set_lamda(&GrandSearchInterfacePrivate::onMatched, [&matched]() {
        matched = true;
    });

    bool searchCompleted = false;
    st.set_lamda(&GrandSearchInterfacePrivate::onSearchCompleted, [&searchCompleted]() {
        searchCompleted = true;
    });

    emit gsi.d->m_main->matched();
    EXPECT_TRUE(matched);

    emit gsi.d->m_main->searchCompleted();
    EXPECT_TRUE(searchCompleted);
}

TEST(GrandSearchInterface, ut_search)
{
    GrandSearchInterface gsi;

    stub_ext::StubExt st;
    QDBusMessage msg;
    st.set_lamda(&GrandSearchInterface::message, [&msg]()->const QDBusMessage & {
        return msg;
    });

    bool accessable = false;
    st.set_lamda(&GrandSearchInterfacePrivate::isAccessable, [&accessable]() {
        return accessable;
    });

    QString str = QUuid::createUuid().toString(QUuid::WithoutBraces);
    EXPECT_EQ(str.size(), 36);
    EXPECT_FALSE(gsi.Search(str, "sss"));

    accessable = true;
    EXPECT_FALSE(gsi.Search(QUuid::createUuid().toString(), "sss"));

    EXPECT_FALSE(gsi.Search(str, ""));

    char maxStr[513] = {32};
    EXPECT_FALSE(gsi.Search(str, QString::fromLatin1(maxStr, 513)));
}

TEST(GrandSearchInterface, ut_search_1)
{
    GrandSearchInterface gsi;

    stub_ext::StubExt st;
    QDBusMessage msg;
    st.set_lamda(&GrandSearchInterface::message, [&msg]()->const QDBusMessage & {
        return msg;
    });
    st.set_lamda(&GrandSearchInterfacePrivate::isAccessable, []() {
        return true;
    });

    gsi.d->m_main = new MainController;
    bool newSearch = false;
    st.set_lamda(&MainController::newSearch, [&newSearch]() {
        newSearch = true;
        return true;
    });

    QString str = QUuid::createUuid().toString(QUuid::WithoutBraces);
    EXPECT_TRUE(gsi.Search(str, "sss"));

    EXPECT_TRUE(newSearch);
    EXPECT_EQ(gsi.d->m_session, str);
    EXPECT_TRUE(gsi.d->m_deadline.isActive());
}

TEST(GrandSearchInterface, ut_search_2)
{
    GrandSearchInterface gsi;

    stub_ext::StubExt st;
    QDBusMessage msg;
    st.set_lamda(&GrandSearchInterface::message, [&msg]()->const QDBusMessage & {
        return msg;
    });
    st.set_lamda(&GrandSearchInterfacePrivate::isAccessable, []() {
        return true;
    });

    gsi.d->m_main = new MainController;
    bool newSearch = false;
    st.set_lamda(&MainController::newSearch, [&newSearch]() {
        newSearch = true;
        return false;
    });

    QString str = QUuid::createUuid().toString(QUuid::WithoutBraces);
    gsi.d->m_deadline.start();
    gsi.d->m_session = str;

    EXPECT_FALSE(gsi.Search(str, "sss"));
    EXPECT_TRUE(newSearch);
    EXPECT_TRUE(gsi.d->m_session.isEmpty());
    EXPECT_FALSE(gsi.d->m_deadline.isActive());
}
