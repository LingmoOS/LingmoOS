// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "wm/wm_connection.h"
#include "wm/wm_info.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"
//Qt
#include <QPoint>
#include <QList>
#include <QDebug>
using namespace core::wm;

//*******************************tpye definition**********************************************//
enum wm_window_type_t {
    kUnknownWindowType              = -1,
    kNormalWindowType               = 0,
    kDesktopWindowType,
    kDockWindowType,
    kToolbarWindowType,
    kMenuWindowType,
    kUtilityWindowType,
    kSplashWindowType,
    kDialogWindowType,
    kDropdownMenuWindowType,
    kPopupMenuWindowType,
    kTooltipWindowType,
    kNotificationWindowType,
    kComboWindowType,
    kDNDWindowType
};

enum wm_state_t {
    kUnknownState                   = -1,
    kModalState                     = 0,
    kStickyState,
    kMaximizedVertState,
    kMaximizedHorzState,
    kShadedState,
    kSkipTaskbarState,
    kSkipPagerState,
    kHiddenState,
    kFullScreenState,
    kAboveState,
    kBelowState,
    kDemandsAttentionState,
    kFocusedState
};

enum wm_window_class_t {
    kUnknownClass                   = -1,
    kInputOutputClass               = XCB_WINDOW_CLASS_INPUT_OUTPUT,
    kInputOnlyClass                 = XCB_WINDOW_CLASS_INPUT_ONLY
};

enum wm_window_map_state_t {
    kUnknownMapState                = -1,
    kUnMappedState                  = XCB_MAP_STATE_UNMAPPED,
    kUnViewableState                = XCB_MAP_STATE_UNVIEWABLE,
    kViewableState                  = XCB_MAP_STATE_VIEWABLE
};

struct core::wm::wm_request_t {
    xcb_get_property_cookie_t           netNameCookie;
    xcb_get_property_cookie_t           nameCookie;
    xcb_get_geometry_cookie_t           geomCookie;
    xcb_translate_coordinates_cookie_t  transCoordsCookie;
    xcb_get_window_attributes_cookie_t  attrCookie;
    xcb_get_property_cookie_t           desktopCookie;
    xcb_get_property_cookie_t           windowTypeCookie;
    xcb_get_property_cookie_t           stateCookie;
    xcb_get_property_cookie_t           pidCookie;
    xcb_get_property_cookie_t           frameExtentsCookie;
    xcb_query_tree_cookie_t             treeCookie;
};

struct core::wm::wm_frame_extents_t {
    uint left;
    uint right;
    uint top;
    uint bottom;
};

struct core::wm::wm_window_ext_t {
    WMWId                           windowId;       // window id
    WMWId                           parent      {}; // window tree schema
    QList<WMWId>                    children    {};

    pid_t                           pid;            // _NET_WM_PID
    int                             desktop     {}; // desktop
    enum wm_window_class_t          wclass      {}; // class: InputOutput || InputOnly && win attrs: XCB_WINDOW_CLASS_INPUT_OUTPUT || XCB_WINDOW_CLASS_INPUT_ONLY
    enum wm_window_map_state_t      map_state   {}; // map state: unmapped || unviewable || viewable && win attrs: XCB_MAP_STATE_UNMAPPED || XCB_MAP_STATE_UNVIEWABLE || XCB_MAP_STATE_VIEWABLE
    QList<enum wm_state_t>          states      {}; // window states && // _NET_WM_STATE
    QList<enum wm_window_type_t>    types       {}; // window types (in order of preference) && _NET_WM_WINDOW_TYPE
    struct wm_frame_extents_t       extents     {}; // window extents && //_NET_FRAME_EXTENTS
    QRect                           rect        {}; // geometry

    std::unique_ptr<wm_request_t>   request;
};

// tree schema in bottom to top stacking order
struct core::wm::wm_tree_t {
    struct wm_window_ext_t         *root;  // root window
    std::map<WMWId, WMWindowExt>    cache; // [windowId : window extended info] mapping
};
//*******************************tpye definition**********************************************//

//*******************************stub begin**********************************************//
ACCESS_PRIVATE_FUN(WMInfo, void(void), findDockWindows);

void stub_WMInfo_findDockWindows(){
    return;
}
//*******************************stub end**********************************************//
class UT_WMInfo: public ::testing::Test
{
public:
    UT_WMInfo() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        auto WMInfo_findDockWindows= get_private_fun::WMInfofindDockWindows();
        Stub b;
        b.set(WMInfo_findDockWindows,stub_WMInfo_findDockWindows);

        m_tester = new WMInfo();

        WMTree tree(new struct wm_tree_t());
        m_tester->m_tree = std::move(tree);


        struct wm_window_ext_t *wmExt = new struct wm_window_ext_t();
        wmExt->pid = 100000;
        wmExt->rect = QRect(0,0,0,0);
        wmExt->windowId = 1000;
        wmExt->parent = 500;
        wmExt->children << 2000;


        m_rootWinExtInfo.reset(wmExt);
        m_tester->m_tree->root = m_rootWinExtInfo.get();
        m_tester->m_tree->cache[m_rootWinExtInfo->windowId] = std::move(m_rootWinExtInfo);


    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester=nullptr;
        }
        if(m_rootWinExtInfo){
            m_rootWinExtInfo.reset();
            m_rootWinExtInfo.reset(nullptr);
        }
        if(m_childRootWinExtInfo){
            m_childRootWinExtInfo.reset();
            m_childRootWinExtInfo.reset(nullptr);
        }
    }

protected:
    WMInfo *m_tester;
    WMWindowExt m_rootWinExtInfo;
    WMWindowExt m_childRootWinExtInfo;
};

TEST_F(UT_WMInfo, initTest)
{

}

TEST_F(UT_WMInfo, test_selectWindow_001)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->selectWindow(QPoint(0,0));

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_selectWindow_002)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = -1;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->selectWindow(QPoint(0,0));

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_selectWindow_003)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    wmExtchild->types << kDockWindowType;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->selectWindow(QPoint(0,0));

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_selectWindow_004)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    wmExtchild->states << kHiddenState;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->selectWindow(QPoint(0,0));

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_selectWindow_005)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->selectWindow(QPoint(0,0));

    m_childRootWinExtInfo.reset();
}


TEST_F(UT_WMInfo, test_getRootWindow_001)
{
    m_tester->getRootWindow();
}

TEST_F(UT_WMInfo, test_isCursorHoveringDocks_001)
{
    QPoint pos(0,0);
    bool flag = m_tester->isCursorHoveringDocks(pos);
    EXPECT_FALSE(flag);
}

TEST_F(UT_WMInfo, test_getHoveredByWindowList_001)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(2000,rect);

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_getHoveredByWindowList_002)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(1000,rect);

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_getHoveredByWindowList_003)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(1000,rect);

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_getHoveredByWindowList_004)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = -1;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(1000,rect);

    m_childRootWinExtInfo.reset();
}


TEST_F(UT_WMInfo, test_getHoveredByWindowList_005)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    wmExtchild->types << kDesktopWindowType;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(1000,rect);

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_getHoveredByWindowList_006)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    wmExtchild->states << kHiddenState;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(1000,rect);

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_getHoveredByWindowList_007)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    QRect rect(0,0,0,0);
    m_tester->getHoveredByWindowList(1000,rect);

    m_childRootWinExtInfo.reset();
}


TEST_F(UT_WMInfo, test_findDockWindows_001)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->findDockWindows();

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_findDockWindows_002)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->findDockWindows();

    m_childRootWinExtInfo.reset();
}


TEST_F(UT_WMInfo, test_findDockWindows_003)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = -1;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->findDockWindows();

    m_childRootWinExtInfo.reset();
}


TEST_F(UT_WMInfo, test_findDockWindows_004)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 10001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;

    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->findDockWindows();

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_findDockWindows_005)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    wmExtchild->states << kHiddenState;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->findDockWindows();

    m_childRootWinExtInfo.reset();
}

TEST_F(UT_WMInfo, test_findDockWindows_006)
{
    struct wm_window_ext_t *wmExtchild = new struct wm_window_ext_t();
    wmExtchild->pid = 100001;
    wmExtchild->rect = QRect(0,0,0,0);
    wmExtchild->windowId = 2000;
    wmExtchild->parent = 1000;
    wmExtchild->map_state = kViewableState;
    wmExtchild->wclass = kInputOutputClass;
    wmExtchild->types << kDockWindowType;
    m_childRootWinExtInfo.reset(wmExtchild);

    m_tester->m_tree->cache[2000] = std::move(m_childRootWinExtInfo);
    m_tester->findDockWindows();

    m_childRootWinExtInfo.reset();
}


TEST_F(UT_WMInfo, test_initAtomCache_001)
{
    WMConnection *connection = new WMConnection;
    auto *conn = connection->xcb_connection();
    m_tester->initAtomCache(conn);
    delete connection;
}


TEST_F(UT_WMInfo, test_getAtomName_001)
{
    xcb_atom_t atom = XCB_ATOM_NONE;
    WMConnection *connection = new WMConnection;
    auto *conn = connection->xcb_connection();
    m_tester->getAtomName(conn,atom);
    delete connection;
}

TEST_F(UT_WMInfo, test_requestWindowExtInfo_001)
{
    xcb_window_t window = m_tester->getRootWindow();
    WMConnection *connection = new WMConnection;
    auto *conn = connection->xcb_connection();
    m_tester->requestWindowExtInfo(conn,window);
    delete connection;
}
