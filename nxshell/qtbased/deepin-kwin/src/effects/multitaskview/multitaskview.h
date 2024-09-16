/*
 * Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     zhangyu <zhangyud@uniontech.com>
 *
 * Maintainer: zhangyu <zhangyud@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KWIN_MULTITASKVIEW_H
#define KWIN_MULTITASKVIEW_H

#include "window.h"
#include "kwinglutils.h"
#include "scene/scene.h"
#include "multitask_effect.h"
#include "kwineffectsex.h"
#include <QHash>
//#include <utils.h>
#include <QMutex>
#include <map>
#include <QSettings>
#include <QThread>

namespace KWin
{

enum workspaceMoveDirection {
    mvNone      = 0x00,
    mvLeft      = 0x01,
    mvRight     = 0x02,
};

enum workspaceDragDirection {
    dragNone      = 0x00,
    dragUpDown    = 0x01,
    dragLeftRight = 0x02,
};

typedef struct screenNameSt {
    QString       name;
    QRect         rect;
    QRect         screenrect;
    EffectScreen *screen;
} ScreenInfo_st;

typedef struct scaleSt {
    float spacingHeight;
    float spacingWidth;
    float workspacingWidth;
    float workspaceMgrHeight;
    float workspaceWidth;
    float workspaceHeight;
    QRect workspaceMgrRect;
    QRect windowMgrRect;
    QRect fullArea;
} Scale_st;

typedef struct backgroundInfo {
    int     desktop;
    EffectScreen *screen;
    QString screenName;
    QSize   workspaceSize;
    QSize   desktopSize;
} BgInfo_st;

class CustomThread : public QThread
{
    Q_OBJECT
public:
    CustomThread(BgInfo_st &st);

protected:
    void run() override;

    BgInfo_st m_st;
};

class MultiViewBackgroundManager: public QObject
{
    Q_OBJECT

public:
    static MultiViewBackgroundManager *instance();
    ~MultiViewBackgroundManager();

    void updateDesktopCount(int n) {
        if (m_desktopCount != n) {
            m_desktopCount = n;
        }
    }

    void getWorkspaceBgPath(BgInfo_st &st, QPixmap &desktopBg, QPixmap &workspaceBg);
    void cacheWorkspaceBg(BgInfo_st &st);
    void getBackgroundList();
    void updateBackgroundList(const QString &file);
    void setNewBackground(BgInfo_st &st, QPixmap &desktopBg, QPixmap &workspaceBg);
    void getPreviewBackground(QSize size, QPixmap &workspaceBg, EffectScreen *screen);
    QPixmap cutBackgroundPix(const QSize &size, const QString &file);
    QPixmap getCachePix(const QSize &size, QPair<QSize, QPixmap> &pair);
    void clearCurrentBackgroundList();
    QString getRandBackground();

    void setMonitorInfo(QList<QMap<QString,QVariant>> monitorInfoList);

private:
    explicit MultiViewBackgroundManager();
    static MultiViewBackgroundManager *_instance;

    class CGarbo {
    public:
        ~CGarbo() {
            if (MultiViewBackgroundManager::_instance)
                delete MultiViewBackgroundManager::_instance;
        }
    };
    static CGarbo Garbo;

public:
    QHash<QString, QVector<QString>> m_allBackgroundList; // monitor name -> background list

private:
    QSet<QString>    m_backgroundAllList;
    QSet<QString>    m_currentBackgroundList;
    int              m_desktopCount {0};
    QList<QString>   m_screenNamelist;
    QString          m_previewFile = "";
    EffectScreen    *m_previewScreen = nullptr;
    QMutex          m_bgmutex;
    QSettings *m_deepinwmrcIni = nullptr;

    QHash<QString, QPair<QSize, QPixmap>> m_wpCachedPixmaps;
    QHash<QString, QPair<QSize, QPixmap>> m_bgCachedPixmaps;
    QList<QMap<QString,QVariant>> m_monitorInfoList;

};

class MultiViewAddButton : public QObject
{
    Q_OBJECT
public:
    MultiViewAddButton();
    ~MultiViewAddButton();

    void render();
    void setImage(const QString &btf, const QRect &rect);
    void setMaxHeight(int height) {m_maxHeight = height;}
    void setArea(QRect &rect, QRect &fullrect) {m_clientArea = rect; m_fullArea = fullrect;}
    void setRect(const QRect &rect);
    const QRect &getRect() const {return m_rect;}

private:
    std::unique_ptr<KWin::EffectFrameEx> m_button;

    int m_maxHeight;
    int m_scale;
    QRect m_rect;
    QRect m_clientArea;
    QRect m_fullArea;
};

class MultiViewWorkspace : public QObject
{
    Q_OBJECT
public:
    MultiViewWorkspace(bool flag=false);
    ~MultiViewWorkspace();

    void renderDesktopBackGround(float k);
    void renderWorkspaceBackGround(float t, int desktop = 1);

    void setImage(const QPixmap &bgPix, const QPixmap &wpPix, const QRect &rect);
    void setRect(const QRect rect);
    QRect getRect() {return m_rect;}
    QRect getCurrentRect() {return m_currentRect;}
    QRect getGeometry();
    void updateGeometry(QRect rect);
    void setMaxHeight(int height) {m_maxHeight = height;}
    void setArea(QRect &rect, QRect &fullrect) {m_clientArea = rect; m_fullArea = fullrect;}
    QRect getClientArea() {return m_clientArea;}
    QRect getfullArea() {return m_fullArea;}
    void setScreenDesktop(EffectScreen *screen, int desktop) { m_screen = screen; m_desktop = desktop;}
    EffectScreen *screen() {return m_screen;}

    int desktop() {return m_desktop;}
    void setPosition(QPoint pos);

private:
    std::unique_ptr<KWin::EffectFrameEx> m_backGroundFrame;
    std::unique_ptr<KWin::EffectFrameEx> m_workspaceBgFrame;
    std::unique_ptr<KWin::EffectFrameEx> m_hoverFrame;
    int m_desktop;
    EffectScreen *m_screen;
    int m_maxHeight;
    QRect m_rect;
    QRect m_currentRect;
    QRect m_clientArea;
    QRect m_fullArea;
    QString m_image;

public:
    workspaceMoveDirection m_posStatus = mvNone; // 0 restore; 1 left; 2 right;
};

class MultiViewWinFill : public QObject
{
public:
    MultiViewWinFill(EffectScreen *screen, QRect rect, float radius, int maxHeight);
    ~MultiViewWinFill();

    QRect getRect() {return m_rect;}
    void render();
private:
    std::unique_ptr<EffectFrameEx> m_fillFrame;
    QRect m_rect;
    EffectScreen *m_screen;
    int m_maxHeight;
};

class MultiViewWinManager : public QObject
{
    Q_OBJECT
public:
    explicit MultiViewWinManager() {};
    ~MultiViewWinManager() {
        for (auto it = m_winManager.begin(); it != m_winManager.end(); it++) {
            it.value().unmanageAll();
        }

        QHash<EffectWindow*, MultiViewWinFill *>::iterator itf = m_windowFill.begin();
        for (; itf != m_windowFill.end(); ++itf) {
            if (itf.value()) {
                delete itf.value();
                itf.value() = nullptr;
            }
        }
    };

    void setdesktop(int desktop) {m_desktop = desktop;}
    void setworkspaceCurrentRect(QRect rect) {m_currentShowRect = rect;}
    QRect getworkspaceCurrentRect() {return m_currentShowRect;}
    void setWinManager(EffectScreen *screen, WindowMotionManager &wm) {
        m_winManager[screen] = wm;
    }
    void manageWin(EffectScreen *screen, EffectWindow *w) {
        if (m_winManager.contains(screen)) {
            m_winManager[screen].manage(w);
        } else {
            WindowMotionManager wm;
            wm.manage(w);
            m_winManager[screen] = wm;
        }
    }
    void removeWin(EffectScreen *screen, EffectWindow *w) {
        if (m_winManager.contains(screen)) {
            m_winManager[screen].unmanage(w);
        }
    }
    void clearMotion() {
        QHash<EffectScreen *, WindowMotionManager>::iterator it;
        for (it = m_winManager.begin(); it != m_winManager.end();) {
            it.value().unmanageAll();
            ++it;
        }
    }
    void resetWindow() {
        QHash<EffectScreen *, WindowMotionManager>::iterator it;
        for (it = m_winManager.begin(); it != m_winManager.end();) {
            for (EffectWindow* w : it.value().managedWindows()) {
                it.value().moveWindow(w, w->frameGeometry().toRect());
            }
            ++it;
        }
    }
    void calculate(int tm, bool motionEffectEnable) {
        for (auto it = m_winManager.begin(); it != m_winManager.end(); it++) {
            it.value().calculate(tm, motionEffectEnable);
        }
    }

    bool getMotion(const int desktop, EffectScreen *screen, WindowMotionManager *&wm) {
        if (m_winManager.contains(screen)) {
            wm = &m_winManager[screen];
            return true;
        }
        return false;
    }
    void updatePos(EffectScreen *screen, QRect rect, QPoint diff) {
        if (m_winManager.contains(screen)) {
            for (EffectWindow* w : m_winManager[screen].orderManagedWindows()) {
                QRectF rect = m_winManager[screen].transformedGeometry(w);
                rect.translate(diff);
                m_winManager[screen].setTransformedGeometry(w, rect);
            }
        }
    }

    EffectWindow *getHoverWin(QPoint pos, EffectScreen *screen = nullptr) {
        if (screen) {
            if (m_winManager.contains(screen)) {
                WindowMotionManager wm = m_winManager[screen];
                for (const auto& w : wm.managedWindows()) {
                    auto geo = wm.transformedGeometry(w);
                    QSize btnSize(24 * effectsEx->getOsScale(), 24 * effectsEx->getOsScale());
                    QRect closeRect(QPoint(geo.topRight().x() - 12 * effectsEx->getOsScale(), geo.y() - 10 * effectsEx->getOsScale()), btnSize);
                    QRect topRect(QPoint(geo.x() - 12 * effectsEx->getOsScale(), geo.y() - 10 * effectsEx->getOsScale()), btnSize);
                    geo.adjust(-5, -5, 5, 5);
                    if (geo.contains(pos)
                        || closeRect.contains(pos)
                        || topRect.contains(pos)) {
                        return w;
                    } else if (wm.isWindowFill(w) && isHaveWinFill(w)) {
                        auto fillgeo = getWinFill(w)->getRect();
                        closeRect = QRect(QPoint(fillgeo.topRight().x() - 12 * effectsEx->getOsScale(), fillgeo.y() - 10 * effectsEx->getOsScale()), btnSize);
                        topRect = QRect(QPoint(fillgeo.x() - 12 * effectsEx->getOsScale(), fillgeo.y() - 10 * effectsEx->getOsScale()), btnSize);
                        fillgeo.adjust(-5, -5, 5, 5);
                        if (fillgeo.contains(pos)
                            || closeRect.contains(pos)
                            || topRect.contains(pos)) {
                            return w;
                        }
                    }
                }
            }
        } else {
            for (auto it = m_winManager.begin(); it != m_winManager.end();) {
                for (const auto& w : it.value().managedWindows()) {
                    auto geo = it.value().transformedGeometry(w);
                    geo.adjust(-5, -5, 5, 5);
                    if (geo.contains(pos)) {
                        return w;
                    }
                }
                ++it;
            }
        }
        return nullptr;
    }

    QRect getWindowGeometry(EffectWindow *w, EffectScreen *screen) {
        QRect rect;
        if (m_winManager.contains(screen)) {
            rect = m_winManager[screen].targetGeometry(w).toRect();
        }
        return rect;
    }

    EffectWindowList getDesktopWinList() {
        EffectWindowList list;
        for (auto it = m_winManager.begin(); it != m_winManager.end(); it++) {
            list.append(it.value().orderManagedWindows());
        }

        return list;
    }

    void getDesktopWinInfo(QHash<EffectScreen *, WindowMotionManager> &minfo) {
        minfo = m_winManager;
    }

    int getDesktopWinNumofScreen(EffectScreen *screen) {
        if (m_winManager.contains(screen)) {
            return m_winManager[screen].orderManagedWindows().size();
        }
        return 0;
    }

    bool isHaveWinFill(EffectWindow *w) {
        if (m_windowFill.contains(w)) {
            return true;
        }
        return false;
    }

    void setWinFill(EffectWindow *w, MultiViewWinFill *fill) {
        m_windowFill[w] = fill;
    }

    MultiViewWinFill *getWinFill(EffectWindow *w) {
        return m_windowFill[w];
    }

    void removeWinFill(EffectWindow *w) {
        delete m_windowFill[w];
        m_windowFill[w] = nullptr;
        m_windowFill.remove(w);
    }

private:
    QHash<EffectScreen *, WindowMotionManager> m_winManager;
    QHash<EffectWindow *, MultiViewWinFill *> m_windowFill;
    int m_desktop;
    QRect m_currentShowRect;
    QRect m_rect;
};

class MultitaskViewEffect : public Effect
{
    Q_OBJECT
public:
    MultitaskViewEffect();
    virtual ~MultitaskViewEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintScreen(int mask, const QRegion &region, ScreenPaintData &data) override;

    void postPaintScreen() override;

    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;

    void windowInputMouseEvent(QEvent* e) override;
    void grabbedKeyboardEvent(QKeyEvent* e) override;

    bool isActive() const override;
    void setActive(bool active);
    bool isReceiveEvent();

    bool touchDown(qint32 id, const QPointF &pos, std::chrono::microseconds time) override;
    bool touchMotion(qint32 id, const QPointF &pos, std::chrono::microseconds time) override;
    bool touchUp(qint32 id, std::chrono::microseconds time) override;

    void setMotionEffect(bool enable) { m_motionEffectEnable = enable; }
    bool motionEffectEnable() const { return m_motionEffectEnable; }

private Q_SLOTS:
    void toggle();

    void onWindowAdded(EffectWindow *w);
    void onWindowDeleted(EffectWindow *w);
    void onWindowClosed(EffectWindow *w);
    void onCloseEffect(bool);

    void onDockChange(const QString &key);
    void screenRecorderStart();
    void fontChanged(const QString &fontType, const QString &fontName);

private:
    void cleanup();
    bool isRelevantWithPresentWindows(EffectWindow *w) const;
    void calculateWindowTransformations(EffectWindowList windows, WindowMotionManager& wmm, int desktop, EffectScreen *screen, bool isReLayout = false);
    void calculateWindowTransformationsClosest(EffectWindowList windowlist, int desktop, EffectScreen *screen,
            WindowMotionManager& motionManager, bool isReLayout = false);

    void calculateWorkSpaceWinTransformations(EffectWindowList windows, WindowMotionManager &wm, int desktop);
    void precalculateWorkSpaceWinTransformations(EffectWindowList windows, WindowMotionManager &wm, int desktop);
    QRect calculateWorkspaceRect(int index, EffectScreen *screen, QRect maxRect, int customDesktopCount = 0);
    bool calculateSwitchPos(QPoint diffPoint);
    void restoreWorkspacePos(int index, int num);

    void initWorkspaceBackground();
    void createEffectFrame();
    void cacheWorkspaceBackground();
    void updateWorkspacePos();
    void updateWorkspacePos(int removedesktop);
    void getScreenInfo();
    void setWinLayout(int desktop, const EffectWindowList &windows);
    void updateWorkspaceWinLayout(int numDesktops);
    void updateWorkspaceWinLayout(int numDesktops, int desktop);
    void workspaceWinRelayout(int desktop, EffectScreen *screen = nullptr);
    void removeWinAndRelayout(EffectWindow *w);
    void createBackgroundFill(EffectWindow *w, QRect rect, float radius, int desktop);
    void removeBackgroundFill(EffectWindow *w, int desktop);
    void addNewDesktop();
    void removeDesktop(int desktop);
    void removeDesktopEx(int desktop);
    void startremoveDesktopEffect(int desktop);
    void cleanDesktopData(int desktop);
    void switchDesktop();
    void desktopSwitchPosition(int to, int from);
    void desktopAboutToRemoved(int d, int num);
    bool checkConfig(EffectWindow *w);
    bool isExtensionMode() const;
    int getNumScreens();
    QVector<int>desktopList(const EffectWindow *w) const;

    void renderWorkspaceMove(KWin::ScreenPaintData &data);
    void renderWindowMove(KWin::ScreenPaintData &data);
    void renderSlidingWorkspace(MultiViewWorkspace *wkobj, EffectScreen *screen, int desktop, KWin::ScreenPaintData &data);
    void renderHover(const EffectWindow *w, const QRect &rect, int order = 0, float radius = 0.0);
    void renderWorkspaceHover(EffectScreen *screen);
    void renderDragWorkspacePrompt(EffectScreen *screen);
    void drawDottedLine(const QRect &geo, EffectScreen *screen);
    void handlerAfterTimeLine();
    void setWinKeepAbove(EffectWindow *w);
    void showWorkspacePreview(EffectScreen *screen, QRect addButtonRect, bool isClear = false);
    void changeCurrentDesktop(int desktop);

    void relayDockEvent(QPoint pos, int button);
    void handlerWheelEvent(Qt::MouseButtons);

    bool checkHandlerWorkspace(QPoint pos, EffectScreen *screen, int &desktop);
    void moveWindowChangeDesktop(EffectWindow *w, int todesktop, EffectScreen *toscreen, bool isSwitch = false);
    bool closeWindow(EffectWindow *w);

private:
    MultiViewWorkspace *getWorkspaceObject(EffectScreen *screen, int secindex);
    MultiViewWinManager *getWinManagerObject(int index);
    MultiViewWinManager *getWorkspaceWinManagerObject(int index);
    EffectWindow *getNextWindow(EffectWindow *w);
    EffectWindow *getPreWindow(EffectWindow *w);
    EffectWindow *getHomeOrEndWindow(bool);
    EffectWindow *getNextSameTypeWindow(EffectWindow *w);
    EffectWindow *getPreSameTypeWindow(EffectWindow *w);
    void motionRepeat();

private:
    QPoint m_cursorPos{-1, -1};
    int m_buttonType{0};

    QAction *m_showActions = nullptr;
    QAction *m_showActionw = nullptr;
    QAction *m_showActiona = nullptr;
    QList<QKeySequence> shortcut;
    QList<QKeySequence> shortcutw;
    QList<QKeySequence> shortcuta;
    QList<EffectWindow *> m_notificationList;

    EffectWindow *m_windowMove = nullptr;
    EffectWindow *m_dock = nullptr;
    EffectWindow *m_hoverWinBtn = nullptr;
    EffectWindow *m_hoverWin = nullptr;
    std::unique_ptr<EffectFrameEx> m_hoverWinFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_closeWinFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_topWinFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_textWinFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_textWinBgFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_previewFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_closeWorkspaceFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_dragTipsFrame = nullptr;
    std::unique_ptr<EffectFrameEx> m_dragTipsFrameShadow = nullptr;

    bool m_activated = false;
    bool m_isShieldEvent = false;
    bool m_hasKeyboardGrab = false;
    bool m_wasWorkspaceMove = false;
    bool m_isShowWhole = true;
    bool m_isShowWin = true;
    bool m_isShowPreview = false;
    bool m_wasWindowMove = false;

    bool m_delayDbus = true;
    bool m_longPressTouch = false;
    bool m_closingWindow = false;

    bool m_motionEffectEnable = true;

    QPoint m_workspaceMoveStartPos;
    QPoint m_windowMoveStartPos;
    QPoint m_windowMoveDiff;
    QPoint m_lastWorkspaceMovePos;

    EffectWindowList                     m_flyingWinList;
    QHash<EffectScreen *, WindowMotionManager>      m_windowInfo;
    std::map<EffectScreen *, std::vector<std::unique_ptr<EffectFrameEx>>> m_tipFrames;
    QHash<EffectScreen *, MultiViewAddButton *>     m_addWorkspaceButton;
    QHash<QString, ScreenInfo_st>        m_screenInfoList;
    QHash<EffectScreen *, QList<MultiViewWorkspace *>> m_workspaceBackgrounds;
    QVector<MultiViewWinManager *>       m_motionManagers;
    QVector<MultiViewWinManager *>       m_workspaceWinMgr;
    QRect m_backgroundRect;
    QRect m_dockRect;
    QRect m_windowMoveGeometry;
    QRect m_allFullArea;

    EffectScreen *m_screen = nullptr;
    int m_aciveMoveDesktop = -1;
    int m_deleteWorkspaceDesktop = -1;
    int m_hoverDesktop = -1;
    int m_passiveMoveDesktop = -1;
    int m_moveWorkspaceNum = -1;
    int m_maxHeight = 0;
    qreal m_scalingFactor = 1;

    QHash<EffectScreen *, Scale_st> m_scale;
    QVector<QRect>    m_winBtnArea;
    QRect                m_workspaceCloseBtnArea;

    enum workspaceStatus {
        wpNone      = 0x00,
        wpSwitch    = 0x01,
        wpRestore   = 0x02,
        wpDelete    = 0x04,
    };

    int             paintingDesktop = 0;
    workspaceStatus m_workspaceStatus = wpNone;
    QMutex          m_mutex;

    TimeLine     m_bgSlidingTimeLine;
    bool         m_bgSlidingStatus;
    int          m_curDesktopIndex;
    int          m_lastDesktopIndex;
    int          m_bgSlidingState = 0;
    bool         m_isRemoveWorkspace = false;
    bool         m_windowEffectState = false;

    TimeLine     m_popTimeLine;
    bool         m_popStatus;

    TimeLine     m_opacityTimeLine;
    bool         m_opacityStatus;

    bool m_workspaceSlidingStatus;
    TimeLine m_workspaceSlidingTimeline;
    std::map <MultiViewWorkspace*, std::pair<int,int> > m_workspaceSlidingInfo;
    int m_previewFramePosX;
    int m_workspaceSlidingState = 0;

    MultiTaskEffectFlyingBack m_effectFlyingBack;
    MultiTaskWindowEffect     m_windowEffect;

    workspaceMoveDirection m_moveWorkspacedirection = mvNone;
    workspaceDragDirection m_dragWorkspacedirection = dragNone;
    float                  m_radius = 0.0;

    QTimer m_addingDesktopTimer;

    struct {
        quint32 id = 0;
        bool active = false;
        bool isMotion = false;
        QPointF pos;
        bool isPress = false;
    } m_touch;
    QTimer *m_timer;
    QTimer *m_timerCheckWindowClose;
    bool m_isScreenRecorder = false;
    bool m_isCloseScreenRecorder = false;
    EffectWindow *m_screenRecorderMenu = nullptr;

    QString m_fontFamily;
    Qt::MouseButton m_sendButton = Qt::NoButton; // send to outside dock or osd pointer button
};

} // namespace KWin

#endif
