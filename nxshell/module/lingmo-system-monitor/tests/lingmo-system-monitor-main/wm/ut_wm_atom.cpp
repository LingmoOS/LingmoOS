// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "wm/wm_atom.h"
#include "wm/wm_info.h"
#include "wm/wm_connection.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//system
#include "xcb/xcb.h"
//Qt
#include <QDebug>
using namespace core::wm;
struct XDisconnector {
    void operator()(xcb_connection_t *conn)
    {
        if (conn && !xcb_connection_has_error(conn))
            xcb_disconnect(conn);
    }
};
using XConnection = std::unique_ptr<xcb_connection_t, XDisconnector>;
class UT_WMAtom: public ::testing::Test
{
public:
    UT_WMAtom() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new WMAtom();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester=nullptr;
        }
    }

protected:
    WMAtom *m_tester;
};

TEST_F(UT_WMAtom, initTest)
{

}

TEST_F(UT_WMAtom, test_initialize_001)
{
    int m_screenNumber {};
    XConnection xconn {};
    QByteArray display {};
    xconn = XConnection(xcb_connect(display.isEmpty() ? nullptr : display.constData(), &m_screenNumber));
    auto *conn = xconn.get();
    if (conn && xcb_connection_has_error(conn)) {
        qDebug() << "Unable to connect to X server";
        return;
    }
    m_tester->initialize(conn);
}

TEST_F(UT_WMAtom, test_atom_001)
{
    WMAtom::atom_t iatom = WMAtom::atom_t::WM_NAME;
    xcb_atom_t atom_t = m_tester->atom(iatom);

    EXPECT_EQ(atom_t,m_tester->m_atoms[iatom]);
}
