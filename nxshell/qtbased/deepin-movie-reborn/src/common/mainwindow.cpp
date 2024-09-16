// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include "mainwindow.h"
#include "toolbox_proxy.h"
#include "actions.h"
#include "event_monitor.h"
#include "shortcut_manager.h"
#include "dmr_settings.h"
#include "movieinfo_dialog.h"
#include "burst_screenshots_dialog.h"
#include "playlist_widget.h"
#include "notification_widget.h"
#include "player_engine.h"
#include "url_dialog.h"
#include "movie_progress_indicator.h"
#include "options.h"
#include "titlebar.h"
#include "utils.h"
#include "dvd_utils.h"
#include "dbus_adpator.h"
#include "threadpool.h"
#include "vendor/movieapp.h"
#include "vendor/presenter.h"
#include "filefilter.h"
#include "eventlogutils.h"

//#include <QtWidgets>
#include <QtDBus>
#include <QtX11Extras>
#include <QX11Info>
#include <DLabel>
#include <DApplication>
#include <DTitlebar>
#include <DSettingsDialog>
#include <DThemeManager>
#include <DAboutDialog>
#include <DInputDialog>
#include <DImageButton>
#include <DWidgetUtil>
#include <DSettingsWidgetFactory>
#include <DLineEdit>
#include <DFileDialog>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include "moviewidget.h"
//#include <qpa/qplatformnativeinterface.h>
#include <QtConcurrent>

#include "../accessibility/ac-deepin-movie-define.h"

#define XCB_Platform     //to distinguish xcb or wayland
#ifdef XCB_Platform
#include "utility.h"
#endif

//add by heyi
//#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
//#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

#define XATOM_MOVE_RESIZE "_NET_WM_MOVERESIZE"
#define XDEEPIN_BLUR_REGION "_NET_WM_DEEPIN_BLUR_REGION"
#define XDEEPIN_BLUR_REGION_ROUNDED "_NET_WM_DEEPIN_BLUR_REGION_ROUNDED"

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

const char kAtomNameHidden[] = "_NET_WM_STATE_HIDDEN";
const char kAtomNameFullscreen[] = "_NET_WM_STATE_FULLSCREEN";
const char kAtomNameMaximizedHorz[] = "_NET_WM_STATE_MAXIMIZED_HORZ";
const char kAtomNameMaximizedVert[] = "_NET_WM_STATE_MAXIMIZED_VERT";
const char kAtomNameMoveResize[] = "_NET_WM_MOVERESIZE";
const char kAtomNameWmState[] = "_NET_WM_STATE";
const char kAtomNameWmStateAbove[] = "_NET_WM_STATE_ABOVE";
const char kAtomNameWmStateStaysOnTop[] = "_NET_WM_STATE_STAYS_ON_TOP";
const char kAtomNameWmSkipTaskbar[] = "_NET_WM_STATE_SKIP_TASKBAR";
const char kAtomNameWmSkipPager[] = "_NET_WM_STATE_SKIP_PAGER";

#define AUTOHIDE_TIMEOUT 2000
#define AUTOHIDE_TIME_PAD 5000   //time of the toolbox auto hide
#include <DToast>
DWIDGET_USE_NAMESPACE

using namespace dmr;

#define MOUSE_MARGINS 6

int MainWindow::m_nRetryTimes = 0;

static void workaround_updateStyle(QWidget *pParent, const QString &sTheme)
{
    pParent->setStyle(QStyleFactory::create(sTheme));
    for (auto obj : pParent->children()) {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget) {
            workaround_updateStyle(pWidget, sTheme);
        }
    }
}

static QString ElideText(const QString &sText, const QSize &size,
                         QTextOption::WrapMode wordWrap, const QFont &font,
                         Qt::TextElideMode mode, int nLineHeight, int nLastLineWidth)
{
    int nHeight = 0;

    QTextLayout textLayout(sText);
    QString sElideText = nullptr;
    QFontMetrics fontMetrics(font);

    textLayout.setFont(font);
    const_cast<QTextOption *>(&textLayout.textOption())->setWrapMode(wordWrap);

    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();

    while (line.isValid()) {
        nHeight += nLineHeight;

        if (nHeight + nLineHeight >= size.height()) {
            sElideText += fontMetrics.elidedText(sText.mid(line.textStart() + line.textLength() + 1),
                                                 mode, nLastLineWidth);
            break;
        }

        line.setLineWidth(size.width());
        const QString &sTmpText = sText.mid(line.textStart(), line.textLength());

        if (sTmpText.indexOf('\n'))
            nHeight += nLineHeight;

        sElideText += sTmpText;
        line = textLayout.createLine();

        if (line.isValid())
            sElideText.append("\n");
    }

    textLayout.endLayout();

    if (textLayout.lineCount() == 1) {
        sElideText = fontMetrics.elidedText(sElideText, mode, nLastLineWidth);
    }

    return sElideText;
}

static QWidget *createSelectableLineEditOptionHandle(QObject *pObj)
{
    DSettingsOption *pSettingOption = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(pObj);

    DLineEdit *pLineEdit = new DLineEdit();
    DWidget *pMainWid = new DWidget;
    QHBoxLayout *pLayout = new QHBoxLayout;

    static QString sNameLast = nullptr;

    pMainWid->setLayout(pLayout);
    DPushButton *pPushButton = new DPushButton;
    pPushButton->setAutoDefault(false);
    pLineEdit->setFixedHeight(21);
    pLineEdit->setObjectName("OptionSelectableLineEdit");
    pLineEdit->setText(pSettingOption->value().toString());
    QFontMetrics fontMetrics = pLineEdit->fontMetrics();
    QString sElideText = ElideText(pLineEdit->text(), {285, fontMetrics.height()}, QTextOption::WrapAnywhere,
                                   pLineEdit->font(), Qt::ElideMiddle, fontMetrics.height(), 285);
    pSettingOption->connect(pLineEdit, &DLineEdit::focusChanged, [ = ](bool bRet) {
        if (bRet)
            pLineEdit->setText(pSettingOption->value().toString());

    });
    pLineEdit->setText(sElideText);
    sNameLast = sElideText;
    pPushButton->setIcon(QIcon(":resources/icons/select-normal.svg"));
    pPushButton->setFixedHeight(21);
    pLayout->addWidget(pLineEdit);
    pLayout->addWidget(pPushButton);

    QWidget *pOptionWidget = new QWidget;
    pOptionWidget->setObjectName("OptionFrame");

    QFormLayout *pOptionLayout = new QFormLayout(pOptionWidget);
    pOptionLayout->setContentsMargins(0, 0, 0, 0);
    pOptionLayout->setSpacing(0);

    pMainWid->setMinimumWidth(240);
    pOptionLayout->addRow(new DLabel(QObject::tr(pSettingOption->name().toStdString().c_str())), pMainWid);

    //auto optionWidget = settingWidget->createWidget(option);
    workaround_updateStyle(pOptionWidget, "light");

    DDialog *pPrompt = new DDialog(pMainWid);
    pPrompt->setIcon(QIcon(":/resources/icons/warning.svg"));
    //pPrompt->setTitle(QObject::tr("Permissions pPrompt"));
    pPrompt->setMessage(QObject::tr("You don't have permission to operate this folder"));
    pPrompt->setWindowFlags(pPrompt->windowFlags() | Qt::WindowStaysOnTopHint);
    pPrompt->addButton(QObject::tr("OK"), true, DDialog::ButtonRecommend);

    auto validate = [ = ](QString sName, bool bAlert = true) -> bool {
        sName = sName.trimmed();
        if (sName.isEmpty()) return false;

        if (sName.size() && sName[0] == '~')
        {
            sName.replace(0, 1, QDir::homePath());
        }

        QFileInfo fi(sName);
        QDir dir(sName);
        if (fi.exists())
        {
            if (!fi.isDir()) {
                if (bAlert) pLineEdit->showAlertMessage(QObject::tr("Invalid folder"));
                return false;
            }

            if (!fi.isReadable() || !fi.isWritable()) {
                return false;
            }
        } else {
            if (dir.cdUp()) {
                QFileInfo ch(dir.path());
                if (!ch.isReadable() || !ch.isWritable())
                    return false;
            }
        }

        return true;
    };

    pSettingOption->connect(pPushButton, &DPushButton::clicked, [ = ]() {
#ifndef USE_TEST
        QString sName = DFileDialog::getExistingDirectory(nullptr, QObject::tr("Open folder"),
                                                          MainWindow::lastOpenedPath(),
                                                          DFileDialog::ShowDirsOnly | DFileDialog::DontResolveSymlinks);
#else
        QString sName = "/data/source/deepin-movie-reborn/movie/DMovie";
#endif
        if (validate(sName, false)) {
            pSettingOption->setValue(sName);
            sNameLast = sName;
        }
        QFileInfo fileinfo(sName);
        if ((!fileinfo.isReadable() || !fileinfo.isWritable()) && !sName.isEmpty()) {
            pPrompt->show();
        }
    });

    pSettingOption->connect(pLineEdit, &DLineEdit::editingFinished, pSettingOption, [ = ]() {

        QString name = pLineEdit->text();
        QDir dir(name);

        auto pn = ElideText(name, {285, fontMetrics.height()}, QTextOption::WrapAnywhere,
                            pLineEdit->font(), Qt::ElideMiddle, fontMetrics.height(), 285);
        auto nmls = ElideText(sNameLast, {285, fontMetrics.height()}, QTextOption::WrapAnywhere,
                              pLineEdit->font(), Qt::ElideMiddle, fontMetrics.height(), 285);

        if (!validate(pLineEdit->text(), false)) {
            QFileInfo fn(dir.path());
            if ((!fn.isReadable() || !fn.isWritable()) && !name.isEmpty()) {
                pPrompt->show();
            }
        }
        if (!pLineEdit->lineEdit()->hasFocus()) {
            if (validate(pLineEdit->text(), false)) {
                pSettingOption->setValue(pLineEdit->text());
                pLineEdit->setText(pn);
                sNameLast = name;
            } else if (pn == sElideText) {
                pLineEdit->setText(sElideText);
            } else {
                pSettingOption->setValue(sNameLast);
                pLineEdit->setText(nmls);
            }
        }
    });

    pSettingOption->connect(pLineEdit, &DLineEdit::textEdited, pSettingOption, [ = ](const QString & sNewStr) {
        validate(sNewStr);
    });

    pSettingOption->connect(pSettingOption, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, pLineEdit,
    [ = ](const QVariant & value) {
        auto pi = ElideText(value.toString(), {285, fontMetrics.height()}, QTextOption::WrapAnywhere,
                            pLineEdit->font(), Qt::ElideMiddle, fontMetrics.height(), 285);
        pLineEdit->setText(pi);
        pLineEdit->update();
    });

    return  pOptionWidget;
}

#ifdef USE_DXCB
class MainWindowFocusMonitor: public QAbstractNativeEventFilter
{
public:
    explicit MainWindowFocusMonitor(MainWindow *src) : QAbstractNativeEventFilter(), _source(src)
    {
        qApp->installNativeEventFilter(this);
    }

    ~MainWindowFocusMonitor()
    {
        qApp->removeNativeEventFilter(this);
    }

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *)
    {
        if (Q_LIKELY(eventType == "xcb_generic_event_t")) {
            xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
            switch (event->response_type & ~0x80) {
            case XCB_LEAVE_NOTIFY: {
                xcb_leave_notify_event_t *dne = (xcb_leave_notify_event_t *)event;
                auto w = _source->windowHandle();
                if (dne->event == w->winId()) {
                    qInfo() << "---------  leave " << dne->event << dne->child;
                    emit _source->windowLeaved();
                }
                break;
            }

            case XCB_ENTER_NOTIFY: {
                xcb_enter_notify_event_t *dne = (xcb_enter_notify_event_t *)event;
                auto w = _source->windowHandle();
                if (dne->event == w->winId()) {
                    qInfo() << "---------  enter " << dne->event << dne->child;
                    emit _source->windowEntered();
                }
                break;
            }
            default:
                break;
            }
        }
        return false;
    }

    MainWindow *_source;
};
#endif

class MainWindowEventListener : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowEventListener(QWidget *pTarget)
        : QObject(pTarget)
    {
        lastCornerEdge = CornerEdge::NoneEdge;
        m_pMainWindow = static_cast<MainWindow *>(pTarget);
        m_pWindow = pTarget->windowHandle();
    }

    void setEnabled(bool bEnale)
    {
        m_bEnabled = bEnale;
    }

protected:
    bool eventFilter(QObject *pObj, QEvent *pEvent) Q_DECL_OVERRIDE {
        QWindow *pWindow = qobject_cast<QWindow *>(pObj);
        if (!pWindow) return false;

        MainWindow *pMainWindow = static_cast<MainWindow *>(parent());

        switch (static_cast<int>(pEvent->type()))
        {
        case QEvent::MouseMove+1: { //响应tab按钮
            QKeyEvent *pKeyEvent = static_cast<QKeyEvent *>(pEvent);
            //根据需求迷你模式不响应tab键交互
            if (pKeyEvent->key() == Qt::Key_Tab) {
                if (!m_pMainWindow->getMiniMode()) {
                    pMainWindow->capturedKeyEvent(pKeyEvent);
                    //Only the tab key interactive response is set to the first
                    if (m_pMainWindow->playlist()->isFocusInPlaylist()) {
                        bool bFocusAttribute = true;
                        m_pMainWindow->playlist()->resetFocusAttribute(bFocusAttribute);
                    }
                } else {
                    return true;
                }
            }
            break;
        }
        case QEvent::MouseButtonPress: {
            if (!m_pMainWindow->playlist()) {
                return true;
            }
            if (m_pMainWindow->playlist()->state() == PlaylistWidget::State::Opened) {
                m_pMainWindow->toolbox()->clearPlayListFocus();
            }
            //Mouse operation does not respond to the first item
            bool bFocusAttribute = false;
            m_pMainWindow->playlist()->resetFocusAttribute(bFocusAttribute);
            if (!m_bEnabled) return false;
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(pEvent);
            setLeftButtonPressed(true);
            if (pMainWindow->insideResizeArea(pMouseEvent->globalPos()) && lastCornerEdge != CornerEdge::NoneEdge)
                m_bStartResizing = true;

            pMainWindow->capturedMousePressEvent(pMouseEvent);
            if (m_bStartResizing) {
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            if (!m_bEnabled)
                return false;
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(pEvent);
            setLeftButtonPressed(false);
            qApp->setOverrideCursor(pWindow->cursor());

            pMainWindow->capturedMouseReleaseEvent(pMouseEvent);
            if (m_bStartResizing) {
                m_bStartResizing = false;
                return true;
            }
            m_bStartResizing = false;
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(pEvent);
            pMainWindow->resumeToolsWindow();

            /* If the focus is on the playlist button, move the mouse to cancel the focus
             * In order to avoid the enter key to expand and the mouse click to expand the playlist
             * There is a problem here, if the mouse does not move, click directly,
             * Will cause focus to appear on the clear list button
             * Please refer to the maintainer whether to add an event filter to the ListBtn
             */
            if (m_pMainWindow->toolbox()->getListBtnFocus()) {
                m_pMainWindow->setFocus();
            }
            //If window is maximized ,need quit maximize state when resizing
            if (m_bStartResizing && (pMainWindow->windowState() & Qt::WindowMaximized)) {
                pMainWindow->setWindowState(pMainWindow->windowState() & (~Qt::WindowMaximized));
            } else if (m_bStartResizing && (pMainWindow->windowState() & Qt::WindowFullScreen)) {
                pMainWindow->setWindowState(pMainWindow->windowState() & (~Qt::WindowFullScreen));
            }

            if (!m_bEnabled) return false;
            const QRect window_visible_rect = m_pWindow->frameGeometry() - pMainWindow->dragMargins();

            if (!m_bLeftButtonPressed) {
                //add by heyi  拦截鼠标移动事件
                pMainWindow->judgeMouseInWindow(QCursor::pos());
                CornerEdge mouseCorner = CornerEdge::NoneEdge;
                QRect cornerRect;

                /// begin set cursor corner type
                cornerRect.setSize(QSize(MOUSE_MARGINS * 2, MOUSE_MARGINS * 2));
                cornerRect.moveTopLeft(m_pWindow->frameGeometry().topLeft());
                if (cornerRect.contains(pMouseEvent->globalPos())) {
                    mouseCorner = CornerEdge::TopLeftCorner;
                    goto set_cursor;
                }

                cornerRect.moveTopRight(m_pWindow->frameGeometry().topRight());
                if (cornerRect.contains(pMouseEvent->globalPos())) {
                    mouseCorner = CornerEdge::TopRightCorner;
                    goto set_cursor;
                }

                cornerRect.moveBottomRight(m_pWindow->frameGeometry().bottomRight());
                if (cornerRect.contains(pMouseEvent->globalPos())) {
                    mouseCorner = CornerEdge::BottomRightCorner;
                    goto set_cursor;
                }

                cornerRect.moveBottomLeft(m_pWindow->frameGeometry().bottomLeft());
                if (cornerRect.contains(pMouseEvent->globalPos())) {
                    mouseCorner = CornerEdge::BottomLeftCorner;
                    goto set_cursor;
                }

                goto skip_set_cursor; // disable edges

                /// begin set cursor edge type
                if (pMouseEvent->globalX() <= window_visible_rect.x()) {
                    mouseCorner = CornerEdge::LeftEdge;
                } else if (pMouseEvent->globalX() < window_visible_rect.right()) {
                    if (pMouseEvent->globalY() <= window_visible_rect.y()) {
                        mouseCorner = CornerEdge::TopEdge;
                    } else if (pMouseEvent->globalY() >= window_visible_rect.bottom()) {
                        mouseCorner = CornerEdge::BottomEdge;
                    } else {
                        goto skip_set_cursor;
                    }
                } else if (pMouseEvent->globalX() >= window_visible_rect.right()) {
                    mouseCorner = CornerEdge::RightEdge;
                } else {
                    goto skip_set_cursor;
                }
set_cursor:
#ifdef USE_DXCB
#ifdef __mips__
                if (pWindow->property("_d_real_winId").isValid()) {
                    auto real_wid = pWindow->property("_d_real_winId").toUInt();
                    Utility::setWindowCursor(real_wid, mouseCorner);
                } else {
                    Utility::setWindowCursor(static_cast<quint32>(pWindow->winId()), mouseCorner);
                }
#endif
#endif

                if (qApp->mouseButtons() == Qt::LeftButton) {
                    updateGeometry(mouseCorner, pMouseEvent);
                }
                lastCornerEdge = mouseCorner;
                return true;

skip_set_cursor:
                lastCornerEdge = mouseCorner = CornerEdge::NoneEdge;
                return false;
            } else {
                if (m_bStartResizing) {
                    updateGeometry(lastCornerEdge, pMouseEvent);
                    return true;
                }
            }
            break;
        }

        default:
            break;
        }

        return false;
    }

private:
    void setLeftButtonPressed(bool bPressed)
    {
        if (m_bLeftButtonPressed == bPressed)
            return;

        if (!bPressed) {
#ifdef USE_DXCB
            Utility::cancelWindowMoveResize(static_cast<quint32>(_window->winId()));
#endif
        }

        m_bLeftButtonPressed = bPressed;
    }

    void updateGeometry(CornerEdge edge, QMouseEvent *pEvent)
    {
        MainWindow *pMainWindow = static_cast<MainWindow *>(parent());
        pMainWindow->updateGeometry(edge, pEvent->globalPos());
    }

    bool m_bLeftButtonPressed = false;
    bool m_bStartResizing = false;
    bool m_bEnabled {true};
    CornerEdge lastCornerEdge;
    QWindow *m_pWindow;
    MainWindow *m_pMainWindow;
};

#ifdef USE_DXCB
/// shadow
#define SHADOW_COLOR_NORMAL QColor(0, 0, 0, 255 * 0.35)
#define SHADOW_COLOR_ACTIVE QColor(0, 0, 0, 255 * 0.6)
#endif

struct SessionInfo {
    QString sessionId;
    uint userId;
    QString userName;
    QString seatId;
    QDBusObjectPath sessionPath;
};
typedef QList<SessionInfo> SessionInfoList;

Q_DECLARE_METATYPE(SessionInfoList);
Q_DECLARE_METATYPE(SessionInfo);

inline QDBusArgument &operator<<(QDBusArgument &argument, const SessionInfo &sessionInfo)
{
    argument.beginStructure();
    argument << sessionInfo.sessionId;
    argument << sessionInfo.userId;
    argument << sessionInfo.userName;
    argument << sessionInfo.seatId;
    argument << sessionInfo.sessionPath;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, SessionInfo &sessionInfo)
{
    argument.beginStructure();
    argument >> sessionInfo.sessionId;
    argument >> sessionInfo.userId;
    argument >> sessionInfo.userName;
    argument >> sessionInfo.seatId;
    argument >> sessionInfo.sessionPath;
    argument.endStructure();

    return argument;
}

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(nullptr)
{
    initMember();

    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"mode", 1}, //冷启动
        {"version", VERSION}
    };
    EventLogUtils::get().writeLogs(obj);

    //add bu heyi
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    m_mousePressTimer.setInterval(1300);
    connect(&m_mousePressTimer, &QTimer::timeout, this, &MainWindow::slotmousePressTimerTimeOut);

#ifdef USE_DXCB
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint |
                   Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
#else
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint |
                   Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
#ifdef Q_OS_MACOS
    setWindowFlags(Qt::WindowFullscreenButtonHint);
#endif
#endif
    setAcceptDrops(true);
    setAttribute(Qt::WA_NoSystemBackground, false);

#ifdef USE_DXCB
    if (DApplication::isDXcbPlatform()) {
        _handle = new DPlatformWindowHandle(this, this);
        _handle->setEnableSystemResize(false);
        _handle->setEnableSystemMove(false);
        _handle->setWindowRadius(4);
        connect(qApp, &QGuiApplication::focusWindowChanged, this, &MainWindow::updateShadow);
        updateShadow();
    }
#endif

    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setContentsMargins(0, 0, 0, 0);
    setupTitlebar();

    CommandLineManager &commanLineManager = dmr::CommandLineManager::get();
    if (commanLineManager.debug()) {
        Backend::setDebugLevel(Backend::DebugLevel::Debug);
    } else if (commanLineManager.verbose()) {
        Backend::setDebugLevel(Backend::DebugLevel::Verbose);
    }
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
    m_pEngine = new PlayerEngine(this);

#ifndef USE_DXCB
    m_pEngine->move(0, 0);
#endif
    //初始化显示音量与音量条控件一致
    m_nDisplayVolume = 100;
    m_pToolbox = new ToolboxProxy(this, m_pEngine);
    m_pToolbox->setObjectName(BOTTOM_TOOL_BOX);

    titlebar()->deleteLater();

    connect(m_pToolbox, &ToolboxProxy::sigUnsupported, this, &MainWindow::slotUnsupported);
    connect(m_pEngine, &PlayerEngine::stateChanged, this, &MainWindow::slotPlayerStateChanged);
    connect(m_pEngine, &PlayerEngine::sigInvalidFile, this, &MainWindow::slotInvalidFile);
    connect(ActionFactory::get().mainContextMenu(), &DMenu::triggered, this, &MainWindow::menuItemInvoked);
    connect(ActionFactory::get().playlistContextMenu(), &DMenu::triggered, this, &MainWindow::menuItemInvoked);
    connect(this, &MainWindow::frameMenuEnable, &ActionFactory::get(), &ActionFactory::frameMenuEnable);
    connect(this, &MainWindow::playSpeedMenuEnable, &ActionFactory::get(), &ActionFactory::playSpeedMenuEnable);
    connect(this, &MainWindow::subtitleMenuEnable, &ActionFactory::get(), &ActionFactory::subtitleMenuEnable);
    connect(this, &MainWindow::soundMenuEnable, &ActionFactory::get(), &ActionFactory::soundMenuEnable);
    connect(qApp, &QGuiApplication::focusWindowChanged, this, &MainWindow::slotFocusWindowChanged);

    connect(m_pToolbox, &ToolboxProxy::sigVolumeChanged, this, &MainWindow::slotVolumeChanged);
    connect(m_pToolbox, &ToolboxProxy::sigMuteStateChanged, this, &MainWindow::slotMuteChanged);
    connect(m_pEngine, &PlayerEngine::sigMediaError, this, &MainWindow::slotMediaError);
    connect(m_pEngine, &PlayerEngine::finishedAddFiles, this, &MainWindow::slotFinishedAddFiles);

    // mini ui
    QSignalMapper *pSignalMapper = new QSignalMapper(this);
    connect(pSignalMapper, static_cast<void(QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), this, &MainWindow::miniButtonClicked);

    m_pMiniPlayBtn = new DIconButton(this);
    m_pMiniQuitMiniBtn = new DIconButton(this);
    m_pMiniCloseBtn = new DIconButton(this);

    m_pMiniPlayBtn->setFlat(true);
    m_pMiniCloseBtn->setFlat(true);
    m_pMiniQuitMiniBtn->setFlat(true);

    m_pMiniPlayBtn->setIcon(QIcon(":/resources/icons/light/mini/play-normal-mini.svg"));
    m_pMiniPlayBtn->setIconSize(QSize(30, 30));
    m_pMiniPlayBtn->setFixedSize(QSize(35, 35));
    m_pMiniPlayBtn->setObjectName("MiniPlayBtn");
    connect(m_pMiniPlayBtn, SIGNAL(clicked()), pSignalMapper, SLOT(map()));
    pSignalMapper->setMapping(m_pMiniPlayBtn, "play");

    connect(m_pEngine, &PlayerEngine::stateChanged, [ = ]() {
        qInfo() << __func__ << m_pEngine->state();

        if (m_pEngine->state() == PlayerEngine::CoreState::Playing
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
            emit subtitleMenuEnable(false);
        } else {
            emit subtitleMenuEnable(true);
        }

        if (m_pEngine->state() == PlayerEngine::CoreState::Idle) {
            //播放切换时，更新音量dbus 当前的sinkInputPath
            emit frameMenuEnable(false);
            emit playSpeedMenuEnable(false);
        }

        if (m_pEngine->state() == PlayerEngine::CoreState::Playing) {
            m_pMiniPlayBtn->setIcon(QIcon(":/resources/icons/light/mini/pause-normal-mini.svg"));
            m_pMiniPlayBtn->setObjectName("MiniPauseBtn");

            if (m_pEngine->playlist().count() > 0 && !m_pEngine->currFileIsAudio()) {
                emit frameMenuEnable(true);
                setMusicShortKeyState(true);
            } else {
                emit frameMenuEnable(false);
                setMusicShortKeyState(false);
            }
            emit playSpeedMenuEnable(true);
            if (m_nLastCookie > 0) {
                utils::UnInhibitStandby(m_nLastCookie);
                qInfo() << "uninhibit cookie" << m_nLastCookie;
                m_nLastCookie = 0;
            }
            if (m_nPowerCookie > 0) {
                utils::UnInhibitPower(m_nPowerCookie);
                m_nPowerCookie = 0;
            }
            m_nLastCookie = utils::InhibitStandby();
            m_nPowerCookie = utils::InhibitPower();
        } else {
            if (m_pMircastShowWidget->isVisible())
                return;
            m_pMiniPlayBtn->setIcon(QIcon(":/resources/icons/light/mini/play-normal-mini.svg"));
            m_pMiniPlayBtn->setObjectName("MiniPlayBtn");

            if (m_nLastCookie > 0) {
                utils::UnInhibitStandby(m_nLastCookie);
                qInfo() << "uninhibit cookie" << m_nLastCookie;
                m_nLastCookie = 0;
            }
            if (m_nPowerCookie > 0) {
                utils::UnInhibitPower(m_nPowerCookie);
                m_nPowerCookie = 0;
            }
        }
    });

    m_pMiniCloseBtn->setIcon(QIcon(":/resources/icons/light/mini/close-normal.svg"));
    m_pMiniCloseBtn->setIconSize(QSize(30, 30));
    m_pMiniCloseBtn->setFixedSize(QSize(35, 35));
    m_pMiniCloseBtn->setObjectName("MiniCloseBtn");
    connect(m_pMiniCloseBtn, SIGNAL(clicked()), pSignalMapper, SLOT(map()));
    pSignalMapper->setMapping(m_pMiniCloseBtn, "close");

    m_pMiniQuitMiniBtn->setIcon(QIcon(":/resources/icons/light/mini/restore-normal-mini.svg"));
    m_pMiniQuitMiniBtn->setIconSize(QSize(30, 30));
    m_pMiniQuitMiniBtn->setFixedSize(QSize(35, 35));
    m_pMiniQuitMiniBtn->setObjectName("MiniQuitMiniBtn");
    connect(m_pMiniQuitMiniBtn, SIGNAL(clicked()), pSignalMapper, SLOT(map()));
    pSignalMapper->setMapping(m_pMiniQuitMiniBtn, "quit_mini");

    m_pMiniPlayBtn->setVisible(m_bMiniMode);
    m_pMiniCloseBtn->setVisible(m_bMiniMode);
    m_pMiniQuitMiniBtn->setVisible(m_bMiniMode);

    updateProxyGeometry();

    connect(&ShortcutManager::get(), &ShortcutManager::bindingsChanged,
            this, &MainWindow::onBindingsChanged);
    ShortcutManager::get().buildBindings();          //绑定要放在connect后
    connect(m_pEngine, SIGNAL(stateChanged()), this, SLOT(update()));
    connect(m_pEngine, &PlayerEngine::tracksChanged, this, &MainWindow::updateActionsState);
    connect(m_pEngine, &PlayerEngine::stateChanged, this, &MainWindow::updateActionsState);
    updateActionsState();

    //勾选右键菜单默认选项
    reflectActionToUI(ActionFactory::ActionKind::OneTimes);
    reflectActionToUI(ActionFactory::ActionKind::ChangeSubCodepage);
    reflectActionToUI(ActionFactory::ActionKind::DefaultFrame);
    reflectActionToUI(ActionFactory::ActionKind::Stereo);

    prepareSplashImages();

    connect(m_pEngine, &PlayerEngine::sidChanged, [ = ]() {
        reflectActionToUI(ActionFactory::ActionKind::SelectSubtitle);
    });
    //NOTE: mpv does not always send a aid-change signal the first time movie is loaded.
    connect(m_pEngine, &PlayerEngine::aidChanged, [ = ]() {
        reflectActionToUI(ActionFactory::ActionKind::SelectTrack);
    });
    connect(m_pEngine, &PlayerEngine::fileLoaded, this, &MainWindow::slotFileLoaded);

    connect(m_pEngine, &PlayerEngine::videoSizeChanged, [ = ]() {
        this->resizeByConstraints();
    });
    connect(m_pEngine, &PlayerEngine::stateChanged, this, &MainWindow::animatePlayState);

    connect(m_pEngine, &PlayerEngine::loadOnlineSubtitlesFinished,
            [this](const QUrl & url, bool success) {//不能去掉 url参数
        m_pCommHintWid->updateWithMessage(success ? tr("Load successfully") : tr("Load failed"));
    });

    connect(&m_autoHideTimer, &QTimer::timeout, this, &MainWindow::suspendToolsWindow);
    m_autoHideTimer.setSingleShot(true);

    connect(&m_delayedMouseReleaseTimer, &QTimer::timeout, this, &MainWindow::delayedMouseReleaseHandler);
    m_delayedMouseReleaseTimer.setSingleShot(true);

    m_pCommHintWid = new NotificationWidget(this);
    m_pCommHintWid->setFixedHeight(30);
    m_pCommHintWid->setAnchor(NotificationWidget::ANCHOR_NORTH_WEST);
    m_pCommHintWid->setAnchorPoint(QPoint(30, 58));
    m_pCommHintWid->hide();
    m_pDVDHintWid = new NotificationWidget(this);
    m_pDVDHintWid->setFixedHeight(30);
    m_pDVDHintWid->setAnchor(NotificationWidget::ANCHOR_NORTH_WEST);
    m_pDVDHintWid->setAnchorPoint(QPoint(30, 58));
    m_pDVDHintWid->hide();

#ifdef USE_DXCB
    m_pEventListener = new MainWindowEventListener(this);
    this->windowHandle()->installEventFilter(m_pEventListener);

    //auto mwfm = new MainWindowFocusMonitor(this);
    auto mwpm = new MainWindowPropertyMonitor(this);

    connect(this, &MainWindow::windowEntered, &MainWindow::resumeToolsWindow);
    connect(this, &MainWindow::windowLeaved, &MainWindow::suspendToolsWindow);

#else
    winId();
    m_pEventListener = new MainWindowEventListener(this);
    QTimer::singleShot(500, [this](){
        this->windowHandle()->installEventFilter(m_pEventListener);

        connect(this, &MainWindow::windowEntered, &MainWindow::resumeToolsWindow);
        connect(this, &MainWindow::windowLeaved, &MainWindow::suspendToolsWindow);
        qInfo() << "event listener";
    } );

#endif

    m_pWMDBus = new QDBusInterface("com.deepin.WMSwitcher", "/com/deepin/WMSwitcher", "com.deepin.WMSwitcher", QDBusConnection::sessionBus());
    QDBusReply<QString> reply_string = m_pWMDBus->call("CurrentWM");
    m_bIsWM = reply_string.value().contains("deepin wm");
    m_pCommHintWid->setWM(m_bIsWM);
    connect(m_pWMDBus, SIGNAL(WMChanged(QString)), this, SLOT(slotWMChanged(QString)));
    m_pAnimationlable = new AnimationLabel(this, this);
    m_pAnimationlable->setWM(m_bIsWM);

    m_pPopupWid = new MessageWindow(this);
    m_pPopupWid->hide();
    defaultplaymodeinit();

    connect(&Settings::get(), &Settings::defaultplaymodechanged, this, &MainWindow::slotdefaultplaymodechanged);
    connect(&Settings::get(), &Settings::setDecodeModel, this, &MainWindow::onSetDecodeModel,Qt::DirectConnection);
    connect(&Settings::get(), &Settings::refreshDecode, this, &MainWindow::onRefreshDecode,Qt::DirectConnection);
    connect(m_pEngine, &PlayerEngine::onlineStateChanged, this, &MainWindow::checkOnlineState);
    connect(&OnlineSubtitle::get(), &OnlineSubtitle::onlineSubtitleStateChanged, this, &MainWindow::checkOnlineSubtitle);
    connect(m_pEngine, &PlayerEngine::mpvErrorLogsChanged, this, &MainWindow::checkErrorMpvLogsChanged);
    connect(m_pEngine, &PlayerEngine::mpvWarningLogsChanged, this, &MainWindow::checkWarningMpvLogsChanged);
    connect(m_pEngine, &PlayerEngine::urlpause, this, &MainWindow::slotUrlpause);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, [ = ] {
        this->activateWindow();
    });

    ThreadPool::instance()->moveToNewThread(&m_diskCheckThread);
    m_diskCheckThread.start();
    connect(&m_diskCheckThread, &Diskcheckthread::diskRemove, this, &MainWindow::diskRemoved);

    QTimer::singleShot(300, [this]() {
        loadPlayList();
    });

    m_pDBus = new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    connect(m_pDBus, SIGNAL(PrepareForSleep(bool)), this, SLOT(sleepStateChanged(bool)));

    QDBusConnection::sessionBus().connect("com.deepin.dde.shutdownFront", "/com/deepin/dde/lockFront",
                                          "com.deepin.dde.lockFront", "Visible", this,
                                          SLOT(lockStateChanged(bool)));

    m_pMovieWidget = new MovieWidget(this);
    m_pMovieWidget->hide();

    m_pMircastShowWidget = new MircastShowWidget(this);
    m_pMircastShowWidget->hide();
    connect(m_pToolbox, &ToolboxProxy::sigMircastState, this, &MainWindow::slotUpdateMircastState);
    connect(m_pMircastShowWidget, &MircastShowWidget::exitMircast, this, &MainWindow::slotExitMircast);

    qDBusRegisterMetaType<SessionInfo>();
    qDBusRegisterMetaType<SessionInfoList>();
    QDBusPendingReply<SessionInfoList> reply = m_pDBus->call("ListSessions");
    QString path = reply.value().last().sessionPath.path();

    QDBusConnection::systemBus().connect("org.freedesktop.login1", path,
                                         "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                                         SLOT(slotProperChanged(QString, QVariantMap, QStringList)));
    qInfo() << "session Path is :" << path;
    connect(dynamic_cast<MpvProxy *>(m_pEngine->getMpvProxy()),&MpvProxy::crashCheck,&Settings::get(),&Settings::crashCheck);
    //解码初始化
    decodeInit();
}

void MainWindow::setupTitlebar()
{
    m_pTitlebar = new Titlebar(this);
    m_pTitlebar->move(0, 0);
    m_pTitlebar->setFixedHeight(50);
    setTitlebarShadowEnabled(false);
    m_pTitlebar->titlebar()->setMenu(ActionFactory::get().titlebarMenu());
    connect(m_pTitlebar->titlebar()->menu(), &DMenu::triggered, this, &MainWindow::menuItemInvoked);

    m_fsTitlebar = new FullScreenTitlebar(this);
    m_fsTitlebar->move(0, 0);
}

void MainWindow::updateContentGeometry(const QRect &rect)
{
#ifdef USE_DXCB
    auto frame = QWindow::fromWinId(windowHandle()->winId());

    QRect frame_rect = rect;
    if (_handle) {
        frame_rect += _handle->frameMargins();
    }

    const uint32_t values[] = { (uint32_t)frame_rect.x(), (uint32_t)frame_rect.y(),
                                (uint32_t)frame_rect.width(), (uint32_t)frame_rect.height()
                              };
    // manually configure frame window which will in turn update content window
    xcb_configure_window(QX11Info::connection(),
                         windowHandle()->winId(),
                         XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT |
                         XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_X,
                         values);

#else
    move(rect.x(), rect.y());
    resize(rect.width(), rect.height());
#endif
}

#ifdef USE_DXCB
void MainWindow::updateShadow()
{
    if (isActiveWindow()) {
        _handle->setShadowRadius(60);
        _handle->setShadowColor(SHADOW_COLOR_ACTIVE);
    } else {
        _handle->setShadowRadius(60);
        _handle->setShadowColor(SHADOW_COLOR_NORMAL);
    }
}
#endif

bool MainWindow::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::UpdateRequest || pEvent->type() == QEvent::Paint)
        return DMainWindow::event(pEvent);

    if (pEvent->type() == QEvent::TouchBegin) {
        //判定是否是触屏
        this->m_posMouseOrigin = mapToGlobal(QCursor::pos());
        m_bIsTouch = true;
    }

    //add by heyi
    //判断是否停止右键菜单定时器
    if (m_bMousePressed) {
        if (qAbs(m_nLastPressX - mapToGlobal(QCursor::pos()).x()) > 50 || qAbs(m_nLastPressY - mapToGlobal(QCursor::pos()).y()) > 50) {
            if (m_mousePressTimer.isActive()) {
                qInfo() << "结束定时器";
                m_mousePressTimer.stop();
                m_bMousePressed = false;
            }
        }
    }

    if (pEvent->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent *pWindowStateChangeEvent = dynamic_cast<QWindowStateChangeEvent *>(pEvent);
        m_lastWindowState = pWindowStateChangeEvent->oldState();
        qInfo() << "------------ m_lastWindowState" << m_lastWindowState
                << "current " << windowState();
        //NOTE: windowStateChanged won't be emitted if by dragging to restore. so we need to
        //check window state here.
        if (windowState() & Qt::WindowMinimized) {   //fix bug 53683
            if (Settings::get().isSet(Settings::PauseOnMinimize)) {
                if (m_pEngine && m_pEngine->state() == PlayerEngine::Playing) {
                    requestAction(ActionFactory::TogglePause);
                    m_bQuitfullscreenflag = true;
                }
                QList<QAction *> listActs = ActionFactory::get().findActionsByKind(ActionFactory::TogglePlaylist);
                listActs.at(0)->setChecked(false);
            }
        } else if (m_lastWindowState & Qt::WindowMinimized /*&& windowState() == Qt::WindowNoState*/) {
            if (Settings::get().isSet(Settings::PauseOnMinimize)) {
                if (m_bQuitfullscreenflag) {
                    requestAction(ActionFactory::TogglePause);
                    m_bQuitfullscreenflag = false;
                }
            }
        }
        onWindowStateChanged();
    }
    return DMainWindow::event(pEvent);
}

void MainWindow::leaveEvent(QEvent *)
{
    m_autoHideTimer.stop();
    this->suspendToolsWindow();
}

void MainWindow::onWindowStateChanged()
{
    qInfo() << __func__ << windowState();
    if (!m_bMiniMode && !isFullScreen()) {
        m_pTitlebar->setVisible(m_pToolbox->isVisible());
    } else {
        if (isFullScreen())
            m_fsTitlebar->setVisible(true);
        m_pTitlebar->setVisible(false);
    }

#ifndef USE_DXCB
    m_pTitlebar->move(0, 0);
    m_fsTitlebar->move(0, 0);
    m_pEngine->move(0, 0);
#endif

    if (!isFullScreen() && !isMaximized()) {
        if (m_bMovieSwitchedInFsOrMaxed || !m_lastRectInNormalMode.isValid()) {
            if (m_bMousePressed || m_bMouseMoved) {
                m_bDelayedResizeByConstraint = true;
            } else {
                setMinimumSize({0, 0});
                resizeByConstraints(true);
            }
        }

        if (m_lastRectInNormalMode.isValid() && !m_bMiniMode) {
            setGeometry(m_lastRectInNormalMode);
        }

        m_bMovieSwitchedInFsOrMaxed = false;
    }
    update();

    if (isMinimized()) {
        if (m_pPlaylist->state() == PlaylistWidget::Opened) {
            m_pPlaylist->togglePopup(false);
        }
    }
}

#ifdef USE_DXCB
static QPoint lastm_pEngine_pos;
static QPoint last_wm_pos;
static bool bClicked = false;
void MainWindow::onMonitorButtonPressed(int nX, int nY)
{
    QPoint pos(nX, nY);
    int nPoint = 2;
    QMargins m(nPoint, nPoint, nPoint, nPoint);
    if (geometry().marginsRemoved(m).contains(pos)) {
        QWidget *pWidget = qApp->topLevelAt(pos);
        if (pWidget && pWidget == this) {
            qInfo() << __func__ << "click inside main window";
            last_wm_pos = QPoint(nX, nY);
            lastm_pEngine_pos = windowHandle()->framePosition();
            bClicked = true;
        }
    }
}

void MainWindow::onMonitorButtonReleased(int nX, int nY)
{
    if (bClicked) {
        qInfo() << __func__;
        bClicked = false;
    }
}

void MainWindow::onMonitorMotionNotify(int nX, int nY)
{
    if (bClicked) {
        QPoint pos = QPoint(nX, nY) - last_wm_pos;
        windowHandle()->setFramePosition(lastm_pEngine_pos + pos);
    }
}
#endif

bool MainWindow::judgeMouseInWindow(QPoint pos)
{
    bool bRet = false;
    QRect rect = frameGeometry();
    QPoint topLeft = rect.topLeft();
    QPoint bottomRight = rect.bottomRight();
    pos = mapToGlobal(pos);
    topLeft = mapToGlobal(topLeft);
    bottomRight = mapToGlobal(bottomRight);

    if ((pos.x() == topLeft.x()) || (pos.x() == bottomRight.x()) || (pos.y() == topLeft.y()) || (pos.y() == bottomRight.y())) {
        leaveEvent(nullptr);
    }

    return bRet;
}

#ifdef USE_DXCB
void MainWindow::onApplicationStateChanged(Qt::ApplicationState e)
{
    switch (e) {
    case Qt::ApplicationActive:
        if (qApp->focusWindow())
            qInfo() << QString("focus window 0x%1").arg(qApp->focusWindow()->winId(), 0, 16);
        qApp->setActiveWindow(this);
        _evm->resumeRecording();
        resumeToolsWindow();
        break;

    case Qt::ApplicationInactive:
        _evm->suspendRecording();
        suspendToolsWindow();
        break;

    default:
        break;
    }
}
#endif

void MainWindow::animatePlayState()
{
    if (m_bMiniMode) {
        return;
    }

    if (!m_bInBurstShootMode && m_pEngine->state() == PlayerEngine::CoreState::Paused
            && !m_bMiniMode && !m_pMircastShowWidget->isVisible()) {
        m_pAnimationlable->pauseAnimation();
    }
}

void MainWindow::onBindingsChanged()
{
    qInfo() << __func__;
    {
        QList<QAction *> listActions = this->actions();
        this->actions().clear();
        for (auto *pAct : listActions) {
            delete pAct;
        }
    }

    ShortcutManager &shortcutManager = ShortcutManager::get();
    vector<QAction *> vecActions = shortcutManager.actionsForBindings();
    for (auto *pAct : vecActions) {
        this->addAction(pAct);
        connect(pAct, &QAction::triggered, [ = ]() {
            ActionFactory::ActionKind actionKind = ActionFactory::actionKind(pAct);
            //正在投屏时，某些快捷键设置为不能用
            if(m_pMircastShowWidget && m_pMircastShowWidget->isVisible() ){
                if(actionKind == ActionFactory::ToggleFullscreen  //全屏 alt+enter
                        || actionKind == ActionFactory::QuitFullscreen //退出全屏/迷你模式esc
                        || actionKind == ActionFactory::AccelPlayback //加速播放 ctrl+right
                        || actionKind == ActionFactory::DecelPlayback //减速播放 ctrl+left
                        || actionKind == ActionFactory::ResetPlayback //还原播放 R
                        || actionKind == ActionFactory::ToggleMiniMode //迷你模式 F2
                        || actionKind == ActionFactory::VolumeUp //增大音量 ctrl+alt+up
                        || actionKind == ActionFactory::VolumeDown //减少音量 ctrl+alt+down
                        || actionKind == ActionFactory::ToggleMute //静音 M
                        || actionKind == ActionFactory::PreviousFrame //上一帧 ctrl+shift+left
                        || actionKind == ActionFactory::NextFrame //下一帧 ctrl+shift+right
                        || actionKind == ActionFactory::Screenshot //影片截图 alt+a
                        || actionKind == ActionFactory::BurstScreenshot //连拍截图 alt+s
                        || actionKind == ActionFactory::SubForward //字幕提前0.5s shift+right
                        || actionKind == ActionFactory::SubDelay //字幕延迟0.5s shift+left
                        || actionKind == ActionFactory::ViewShortcut //显示快捷键 ctrl + shift + ?
                  ){
                    return;
                }
            }
            this->menuItemInvoked(pAct);
        });
    }
}

void MainWindow::updateActionsState()
{
    //投屏时不处理播放状态切换菜单项是否可用，由右键菜单入口统一处理。
    if(m_pMircastShowWidget && m_pMircastShowWidget->isVisible()) return;
    PlayingMovieInfo movieInfo = m_pEngine->playingMovieInfo();
    auto update = [ = ](QAction * pAct) {
        ActionFactory::ActionKind actionKind = ActionFactory::actionKind(pAct);
        bool bRet = true;
        switch (actionKind) {
        case ActionFactory::ActionKind::Screenshot:
        case ActionFactory::ActionKind::MatchOnlineSubtitle:
        case ActionFactory::ActionKind::ToggleMiniMode:
        case ActionFactory::ActionKind::ToggleFullscreen:
        case ActionFactory::ActionKind::WindowAbove:
            bRet = m_pEngine->state() != PlayerEngine::Idle;
            break;
        case ActionFactory::ActionKind::BurstScreenshot:
            if(!CompositingManager::isMpvExists()) {
                bRet = false;
            } else {
                bRet = m_pEngine->duration() > 40;
            }
            break;
        case ActionFactory::ActionKind::MovieInfo:
            bRet = m_pEngine->state() != PlayerEngine::Idle;
            if (bRet) {
                bRet = bRet && m_pEngine->playlist().count();
                if (bRet) {
                    PlayItemInfo playItemInfo = m_pEngine->playlist().currentInfo();
                    bRet = bRet && playItemInfo.loaded;
                }
            }
            break;
        case ActionFactory::ActionKind::SelectSubtitle:
            bRet = movieInfo.subs.size() > 0;
            break;
        case ActionFactory::ActionKind::GotoPlaylistNext:
        case ActionFactory::ActionKind::GotoPlaylistPrev:
            bRet = m_pEngine->state() != PlayerEngine::CoreState::Idle &&
                    m_pEngine->getplaylist()->items().size() > 1;
            break;
        default:
            break;
        }
        pAct->setEnabled(bRet);
    };

    ActionFactory::get().updateMainActionsForMovie(movieInfo);
    ActionFactory::get().forEachInMainMenu(update);

    //NOTE: mpv does not always send a aid-change signal the first time movie is loaded.
    //so we need to workaround it.
    reflectActionToUI(ActionFactory::ActionKind::SelectTrack);
    reflectActionToUI(ActionFactory::ActionKind::SelectSubtitle);
}

/*void MainWindow::syncStaysOnTop()
{
#ifdef USE_DXCB
    static xcb_atom_t atomStateAbove = Utility::internAtom("_NET_WM_STATE_ABOVE");
    auto atoms = Utility::windowNetWMState(static_cast<quint32>(windowHandle()->winId()));

#ifndef __mips__
    bool window_is_above = atoms.contains(atomStateAbove);
    if (window_is_above != m_bWindowAbove) {
        qInfo() << "syncStaysOnTop: window_is_above" << window_is_above;
        requestAction(ActionFactory::WindowAbove);
    }
#endif
#endif
}*/

void MainWindow::reflectActionToUI(ActionFactory::ActionKind actionKind)
{
    QList<QAction *> listActs;
    listActs = ActionFactory::get().findActionsByKind(actionKind);
    if(listActs.size()<=0) {
        return;
    }

    switch (actionKind) {
    case ActionFactory::ActionKind::WindowAbove:
    case ActionFactory::ActionKind::ToggleFullscreen:
    case ActionFactory::ActionKind::TogglePlaylist:
    case ActionFactory::ActionKind::HideSubtitle: {
        qInfo() << __func__ << actionKind;
        auto p = listActs.begin();
        while (p != listActs.end()) {
            bool bOld = (*p)->isEnabled();
            (*p)->setEnabled(false);
            if (actionKind == ActionFactory::TogglePlaylist) {
                // here what we read is the last state of playlist
                if (m_pPlaylist->state() != PlaylistWidget::Opened) {
                    (*p)->setChecked(false);
                } else {
                    (*p)->setChecked(true);
                }
            } else {
                (*p)->setChecked(!(*p)->isChecked());
            }
            (*p)->setEnabled(bOld);
            ++p;
        }
        break;
    }

    case ActionFactory::ActionKind::ToggleMiniMode: {
        auto p = listActs[0];

        p->setEnabled(false);
        p->setChecked(!p->isChecked());
        p->setEnabled(true);
        break;
    }

    case ActionFactory::ActionKind::ChangeSubCodepage: {
        //mpv未初始化时返回默认值auto
        QString sCodePage;
        sCodePage = m_pEngine->subCodepage();
        qInfo() << "codepage" << sCodePage;
        auto p = listActs.begin();
        while (p != listActs.end()) {
            auto args = ActionFactory::actionArgs(*p);
            if (args[0].toString() == sCodePage) {
                (*p)->setEnabled(false);
                if (!(*p)->isChecked())
                    (*p)->setChecked(true);
                (*p)->setEnabled(true);
                break;
            }
            ++p;
        }
        break;
    }

    case ActionFactory::ActionKind::SelectTrack:
    case ActionFactory::ActionKind::SelectSubtitle: {
        if (m_pEngine->state() == PlayerEngine::Idle)
            break;

        PlayingMovieInfo pmf = m_pEngine->playingMovieInfo();
        int nId = -1;
        int nIdx = -1;
        if (actionKind == ActionFactory::ActionKind::SelectTrack) {
            nId = m_pEngine->aid();
            for (nIdx = 0; nIdx < pmf.audios.size(); nIdx++) {
                if (nId == pmf.audios[nIdx]["id"].toInt()) {
                    break;
                }
            }
        } else if (actionKind == ActionFactory::ActionKind::SelectSubtitle) {
            nId = m_pEngine->sid();
            for (nIdx = 0; nIdx < pmf.subs.size(); nIdx++) {
                if (nId == pmf.subs[nIdx]["id"].toInt()) {
                    nIdx++;
                    break;
                }
            }
        }

        qInfo() << __func__ << actionKind << "idx = " << nIdx;
        auto p = listActs.begin();
        while (p != listActs.end()) {
            auto args = ActionFactory::actionArgs(*p);
            (*p)->setEnabled(false);
            if (args[0].toInt() == nIdx) {
                if (!(*p)->isChecked())(*p)->setChecked(true);
            } else {
                (*p)->setChecked(false);
            }
            (*p)->setEnabled(true);

            ++p;
        }
        break;
    }

    case ActionFactory::ActionKind::Stereo:
    case ActionFactory::ActionKind::OneTimes: {
        auto p = listActs.begin();
        (*p)->setChecked(true);
        break;
    }
    case ActionFactory::ActionKind::DefaultFrame: {
        qInfo() << __func__ << actionKind;
        auto p = listActs.begin();
        bool bOld = (*p)->isEnabled();
        (*p)->setEnabled(false);
        (*p)->setChecked(!(*p)->isChecked());
        (*p)->setEnabled(bOld);
        break;
    }
    case ActionFactory::ActionKind::OrderPlay:
    case ActionFactory::ActionKind::ShufflePlay:
    case ActionFactory::ActionKind::SinglePlay:
    case ActionFactory::ActionKind::SingleLoop:
    case ActionFactory::ActionKind::ListLoop: {
        qInfo() << __func__ << actionKind;
        auto p = listActs.begin();
        (*p)->setChecked(true);
        break;
    }
    default:
        break;
    }
}

/*NotificationWidget *MainWindow::getm_pCommHintWid()
{
    return m_pCommHintWid;
}*/

//排列判断(主要针对光驱)
static bool compareBarData(const QUrl &url1, const QUrl &url2)
{
    QString sFileName1 = QFileInfo(url1.path()).fileName();
    QString sFileName2 = QFileInfo(url2.path()).fileName();
    if (sFileName1.length() > 0 && sFileName2.length() > 0) {
        if (sFileName1[0] < sFileName2[0]) {
            return true;
        }
    }
    return false;
}

bool MainWindow::addCdromPath()
{
    QStringList strCDMountlist;

    QFile mountFile("/proc/mounts");
    if (mountFile.open(QIODevice::ReadOnly) == false) {
        return false;
    }
    do {
        QString strLine = mountFile.readLine();
        if (strLine.indexOf("/dev/sr") != -1 || strLine.indexOf("/dev/cdrom") != -1) {  //说明存在光盘的挂载。
            strCDMountlist.append(strLine.split(" ").at(1));        //A B C 这样的格式，取中间的
        }
    } while (!mountFile.atEnd());
    mountFile.close();

    if (strCDMountlist.size() == 0)
        return false;

    play({strCDMountlist[0]});

    return true;
}

void MainWindow::loadPlayList()
{
    m_pPlaylist = nullptr;
    m_pPlaylist = new PlaylistWidget(this, m_pEngine);
    m_pPlaylist->hide();
    m_pToolbox->setPlaylist(m_pPlaylist);
    m_pEngine->getplaylist()->loadPlaylist();

    if(CompositingManager::isMpvExists()) {
        m_pToolbox->initThumbThread();
    }

    if (!m_listOpenFiles.isEmpty()) {
        play(m_listOpenFiles);
    }
}

void MainWindow::setOpenFiles(QStringList &list)
{
    m_listOpenFiles = list;
}

QString MainWindow::padLoadPath()
{
    QString sLoadPath = Settings::get().generalOption("pad_load_path").toString();
    QDir lastDir(sLoadPath);
    if (sLoadPath.isEmpty() || !lastDir.exists()) {
        sLoadPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        QDir newLastDir(sLoadPath);
        if (!newLastDir.exists()) {
            sLoadPath = QDir::currentPath();
        }
    }

    return sLoadPath;
}

#ifdef USE_TEST
void MainWindow::testCdrom()
{
    this->addCdromPath();
    diskRemoved("sd3/uos");
    sleepStateChanged(true);
    sleepStateChanged(false);
    subtitleMatchVideo("/data/home/uos/Videos/subtitle/Hachiko.A.Dog's.Story.ass");
}
void MainWindow::setCurrentHwdec(QString str)
{
    m_sCurrentHwdec = str;
}
#endif

void MainWindow::mipsShowFullScreen()
{
    QPropertyAnimation *pAn = new QPropertyAnimation(this, "windowOpacity");
    pAn->setDuration(100);
    pAn->setEasingCurve(QEasingCurve::Linear);
    pAn->setEndValue(1);
    pAn->setStartValue(0);
    pAn->start(QAbstractAnimation::DeleteWhenStopped);

    setWindowState(windowState() | Qt::WindowFullScreen);
}

void MainWindow::menuItemInvoked(QAction *pAction)
{
    ActionFactory::ActionKind actionKind = ActionFactory::actionKind(pAction);
    if (actionKind == dmr::ActionFactory::Invalid || !m_pEngine || !m_pPlaylist) {  //如果未初始化触发快捷键会导致崩溃
        return;
    }
    bool bIsShortcut = ActionFactory::isActionFromShortcut(pAction);
    if (ActionFactory::actionHasArgs(pAction)) {
        requestAction(actionKind, !bIsShortcut, ActionFactory::actionArgs(pAction), bIsShortcut);
    } else {
        QVariant var = pAction->property("kind");
        if (var == ActionFactory::ActionKind::Settings) {
            requestAction(actionKind, !bIsShortcut, {0}, bIsShortcut);
        } else {
            if (m_pPlaylist->state() == PlaylistWidget::State::Opened) {
                BindingMap bdMap = ShortcutManager::get().map();
                QHash<QKeySequence, ActionFactory::ActionKind>::const_iterator iter = bdMap.constBegin();
                bool bIsiter = false;
                while (iter != bdMap.constEnd()) {
                    if (iter.value() == actionKind) {
                        bIsiter = true;
                        if ((iter.key() == QKeySequence("Return")
                                || iter.key() == QKeySequence("Enter")
                                || iter.key() == QKeySequence("Up")
                                || iter.key() == QKeySequence("Down")) && bIsShortcut) {
                            if (iter.key() == QKeySequence("Up") || iter.key() == QKeySequence("Down")) {
                                int key;
                                if (iter.key() == QKeySequence("Up")) {
                                    key = Qt::Key_Up;
                                } else {
                                    key = Qt::Key_Down;
                                }
                                m_pPlaylist->updateSelectItem(key);
                            }
                            break;
                        }
                        requestAction(actionKind, !bIsShortcut, {0}, bIsShortcut);
                        break;
                    }
                    ++iter;
                }
                if (bIsiter == false) {
                    requestAction(actionKind, !bIsShortcut, {0}, bIsShortcut);
                }
            } else {
                requestAction(actionKind, !bIsShortcut, {0}, bIsShortcut);
            }
        }
    }
    //菜单操作完成后，标题栏获取焦点
    if (isFullScreen())
        m_fsTitlebar->setFocus();
    else
        m_pTitlebar->setFocus();
}

bool MainWindow::isActionAllowed(ActionFactory::ActionKind actionKind, bool fromUI, bool isShortcut)
{
    if (m_bInBurstShootMode) {
        return false;
    }

    if (m_bMiniMode) {
        if (fromUI || isShortcut) {
            switch (actionKind) {
            case ActionFactory::ToggleFullscreen:
            case ActionFactory::TogglePlaylist:
            case ActionFactory::BurstScreenshot:
                return false;

            case ActionFactory::ToggleMiniMode:
                return true;

            default:
                break;
            }
        }
    }

    if (isMaximized()) {
        switch (actionKind) {
        case ActionFactory::ToggleMiniMode:
            return true;
        default:
            break;
        }
    }

    if (isShortcut) {
        PlayingMovieInfo pmf = m_pEngine->playingMovieInfo();
        bool bRet = true;//cppcheck 误报
        switch (actionKind) {
        case ActionFactory::Screenshot:
        case ActionFactory::ToggleMiniMode:
        case ActionFactory::MatchOnlineSubtitle:
        case ActionFactory::BurstScreenshot:
            bRet = m_pEngine->state() != PlayerEngine::Idle;
            break;

        case ActionFactory::MovieInfo:
            bRet = m_pEngine->state() != PlayerEngine::Idle;
            if (bRet) {
                bRet = bRet && m_pEngine->playlist().count();
                if (bRet) {
                    auto pif = m_pEngine->playlist().currentInfo();
                    bRet = bRet && pif.loaded;
                }
            }
            break;

        case ActionFactory::HideSubtitle:
        case ActionFactory::SelectSubtitle:
            bRet = pmf.subs.size() > 0;
            break;
        default:
            break;
        }
        if (!bRet) return bRet;
    }

    return true;
}

void MainWindow::requestAction(ActionFactory::ActionKind actionKind, bool bFromUI,
                               QList<QVariant> args, bool bIsShortcut)
{
    qInfo() << "actionKind = " << actionKind << "fromUI " << bFromUI << (bIsShortcut ? "shortcut" : "");

    if (m_bStartAnimation) {
        return;
    }

    if (!isActionAllowed(actionKind, bFromUI, bIsShortcut)) {
        qInfo() << actionKind << "disallowed";
        return;
    }

    switch (actionKind) {
    case ActionFactory::ActionKind::Exit:
        qApp->quit();
        break;

    case ActionFactory::ActionKind::OpenCdrom: {
        QString sDev = dmr::CommandLineManager::get().dvdDevice();
        if (sDev.isEmpty()) {
            sDev = probeCdromDevice();
        }
        if (sDev.isEmpty()) {
            m_pCommHintWid->updateWithMessage(tr("Cannot play the disc"));
            break;
        }

        if (addCdromPath() == false) {
            play({QString("dvd:///%1").arg(sDev)});
        }
        break;
    }

    case ActionFactory::ActionKind::OpenUrl: {
        UrlDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            QUrl url = dlg.url();
            if (url.isValid()) {
                play({url.toString()});
            } else {
                m_pCommHintWid->updateWithMessage(tr("Parse failed"));
            }
        }
        break;
    }

    case ActionFactory::ActionKind::OpenDirectory: {
#ifndef USE_TEST
        QString name = DFileDialog::getExistingDirectory(this, tr("Open folder"),
                                                         lastOpenedPath(),
                                                         DFileDialog::DontResolveSymlinks);
#else
        QString name("/data/source/deepin-movie-reborn/movie");
#endif

        QFileInfo fi(name);
        if (fi.isDir() && fi.exists()) {
            Settings::get().setGeneralOption("last_open_path", fi.path());

            play({name});
        }
        break;
    }

    case ActionFactory::ActionKind::OpenFileList: {
        if (QDateTime::currentMSecsSinceEpoch() - m_pToolbox->getMouseTime() < 500) {
            return;
        }
        if (m_pEngine->getplaylist()->items().isEmpty() && m_pEngine->getplaylist()->getThumanbilRunning()) {
            return;
        }
        //允许影院打开音乐文件进行播放
#ifndef USE_TEST
        DFileDialog fileDialog;
        QStringList filenames;

        QString strVideoTypes = m_pEngine->video_filetypes.join(" ");
        QString strAudioTypes = m_pEngine->audio_filetypes.join(" ");
        if(!CompositingManager::isMpvExists()) {
            strVideoTypes = QString("*.ogg *.dv *.avi *.webm");
            strAudioTypes = QString("*.wv *.flac *.mp3");
        }

        fileDialog.setNameFilters({tr("All (*)"), QString("Video (%1)").arg(strVideoTypes),
                                   QString("Audio (%1)").arg(strAudioTypes)});
        fileDialog.selectNameFilter(QString("Video (%1)").arg(strVideoTypes));
        fileDialog.setDirectory(lastOpenedPath());
        fileDialog.setFileMode(QFileDialog::ExistingFiles);

        if (fileDialog.exec() == QDialog::Accepted) {
            filenames = fileDialog.selectedFiles();
        } else {
            break;
        }
#else
        QStringList filenames;
        filenames << QString("/data/source/deepin-movie-reborn/movie/demo.mp4")\
                  << QString("/data/source/deepin-movie-reborn/movie/bensound-sunny.mp3");
#endif
        if (filenames.size()) {
            QFileInfo fileInfo(filenames[0]);
            if (fileInfo.exists()) {
                Settings::get().setGeneralOption("last_open_path", fileInfo.path());
            }
            play(filenames);
        }
        break;
    }

    case ActionFactory::ActionKind::OpenFile: {
        DFileDialog fileDialog(this);
        QStringList filename;
        QString strVideoTypes = m_pEngine->video_filetypes.join(" ");
        QString strAudioTypes = m_pEngine->audio_filetypes.join(" ");
        if(!CompositingManager::isMpvExists()) {
            strVideoTypes = QString("ogg dv avi webm");
            strAudioTypes = QString("wv flac mp3");
        }

        fileDialog.setParent(this);
        fileDialog.setNameFilters({tr("All (*)"), QString("Video (%1)").arg(strVideoTypes),
                                   QString("Audio (%1)").arg(strAudioTypes)});
        fileDialog.selectNameFilter(QString("Video (%1)").arg(strVideoTypes));
        fileDialog.setDirectory(lastOpenedPath());
        fileDialog.setFileMode(QFileDialog::ExistingFiles);

        if (fileDialog.exec() == QDialog::Accepted) {
            filename = fileDialog.selectedFiles();
        } else {
            break;
        }
        QFileInfo fileInfo(filename[0]);
        if (fileInfo.exists()) {
            Settings::get().setGeneralOption("last_open_path", fileInfo.path());

            play({filename[0]});
        }
        break;
    }

    case ActionFactory::ActionKind::StartPlay: {
        if (m_pEngine->playlist().count() == 0) {
            requestAction(ActionFactory::ActionKind::OpenFileList);
        } else {
            if (m_pEngine->state() == PlayerEngine::CoreState::Idle  && m_bIsFree) {
                //先显示分辨率，再显示静音
                QSize sz = geometry().size();
                auto msg = QString("%1x%2").arg(sz.width()).arg(sz.height());
                QTimer::singleShot(500, [ = ]() {
                    if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
                        m_pCommHintWid->updateWithMessage(msg);
                    }
                });
                QVariant panscan = m_pEngine->getBackendProperty("panscan");
                if ((panscan.isNull() || !CompositingManager::isMpvExists()) && Settings::get().isSet(Settings::ResumeFromLast)) {
                    int restore_pos = Settings::get().internalOption("playlist_pos").toInt();
                    //Playback when the playlist is not loaded, this will result in the 
                    //last exit item without playing, because the playlist has not been 
                    //loaded into that file, so adding a thread waiting here.  
                    //TODO(xxxxp):It will cause direct opening of the cartoon? May need to optimize Model View
                    while (m_pEngine->getplaylist()->getThumanbilRunning()) {
                        QCoreApplication::processEvents();
                    }
                    restore_pos = qMax(qMin(restore_pos, m_pEngine->playlist().count() - 1), 0);
                    requestAction(ActionFactory::ActionKind::GotoPlaylistSelected, false, {restore_pos});
                } else {
                    m_pEngine->play();
                }
            }
        }
        break;
    }

    case ActionFactory::ActionKind::EmptyPlaylist: {
        //play list context menu empty playlist
        m_pEngine->clearPlaylist();
        break;
    }

    case ActionFactory::ActionKind::TogglePlaylist: {
        if (m_bStartMini || m_bMiniMode) {
            return;
        }
        //快捷键操作不置回焦点
        if (bIsShortcut) {
            m_pToolbox->clearPlayListFocus();
        }
        /* The focus of the clear list button when the playlist is raised is also handled here.
         * Cancel the focus of the shortcut key when it is raised to avoid this problem
         */
        m_bStartAnimation = true;
        QTimer::singleShot(150, [ = ]() {    //延时是为了解决在窗口变化同时操作时，因窗口size未确定导致显示异常
            m_bStartAnimation = false;
            if (bIsShortcut && toolbox()->getListBtnFocus()) {
                setFocus();
            }
            if (m_pPlaylist && m_pPlaylist->state() == PlaylistWidget::Closed && !m_pToolbox->isVisible()) {
                m_pToolbox->show();
            }
            m_pPlaylist->togglePopup(bIsShortcut);
            if (!bFromUI) {
                reflectActionToUI(actionKind);
            }
            this->resumeToolsWindow();
        });

        break;
    }

    case ActionFactory::ActionKind::ToggleMiniMode: {
        if (m_bMouseMoved) { // can't toggle minimode,when window is moving
            break;
        }

        int nDelayTime = 0;
        if (m_pPlaylist->state() == PlaylistWidget::Opened) {
            requestAction(ActionFactory::TogglePlaylist);
            nDelayTime = 500;
        }

        m_bStartMini = true;

        QTimer::singleShot(nDelayTime, this, [ = ] {
            if (!bFromUI)
            {
                reflectActionToUI(actionKind);
            }
            toggleUIMode();
        });
        //Prevent abnormal focus position due to window state changes
        setFocus();
        break;
    }

    case ActionFactory::ActionKind::MovieInfo: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            //fix 107355
            //Add a mouse display to prevent the target is hidden
            qApp->setOverrideCursor(Qt::ArrowCursor);
            MovieInfoDialog mid(m_pEngine->playlist().currentInfo(), this);
            mid.exec();
        }
        break;
    }

    case ActionFactory::ActionKind::WindowAbove: {
        m_bWindowAbove = !m_bWindowAbove;
        if (!utils::check_wayland_env()) {
            my_setStayOnTop(this, m_bWindowAbove);
            show();
        } else {
            //wayland 置顶实现
            QFunctionPointer setWindowProperty = qApp->platformFunction("_d_setWindowProperty");
            if (m_bWindowAbove) { //置顶
                reinterpret_cast<void(*)(QWindow *, const char *, const QVariant &)>(setWindowProperty)(windowHandle(), "_d_dwayland_staysontop", true);
            } else {//取消置顶
                reinterpret_cast<void(*)(QWindow *, const char *, const QVariant &)>(setWindowProperty)(windowHandle(), "_d_dwayland_staysontop", false);
            }
        }
        if (!bFromUI) {
            reflectActionToUI(actionKind);
        }
        break;
    }

    case ActionFactory::ActionKind::QuitFullscreen: {
        if (!m_pToolbox->getVolSliderIsHided()) {
            m_pToolbox->setVolSliderHide();       // esc降下音量条
            break;
        }

        if (m_bMiniMode) {
            if (!bFromUI) {
                reflectActionToUI(ActionFactory::ToggleMiniMode);
            }
            toggleUIMode();
        } else if (isFullScreen()) {
            requestAction(ActionFactory::ToggleFullscreen);
        } else {
            //当焦点在播放列表上按下Esc键，播放列表收起，焦点回到列表按钮上
            if (m_pPlaylist->state() == PlaylistWidget::Opened) {
                m_pToolbox->playlistClosedByEsc();
            }
        }
        break;
    }

    case ActionFactory::ActionKind::ToggleFullscreen: {
        if (QDateTime::currentMSecsSinceEpoch() - m_nFullscreenTime < 600) {
            return;
        } else {
            m_nFullscreenTime = QDateTime::currentMSecsSinceEpoch();
        }

        //音量条控件打开时全屏位置异常，全屏时关掉音量条
        m_pAnimationlable->hide();
        m_pToolbox->closeAnyPopup();

        if (isFullScreen()) {
            m_fsTitlebar->hide();
            setWindowState(windowState() & ~Qt::WindowFullScreen);
            if (m_bMaximized) {
                showMaximized();
            } else {
                if (m_lastRectInNormalMode.isValid() && !m_bMiniMode && !isMaximized()) {
                    setGeometry(m_lastRectInNormalMode);
                    move(m_lastRectInNormalMode.x(), m_lastRectInNormalMode.y());
                    resize(m_lastRectInNormalMode.width(), m_lastRectInNormalMode.height());
                    if (utils::check_wayland_env())
                        m_pTitlebar->setFixedWidth(m_lastRectInNormalMode.width());             //bug 39991
                }
            }
        } else {
            if (utils::check_wayland_env()) {
                m_pToolbox->setVolSliderHide();
                m_pToolbox->setButtonTooltipHide();
            }
            m_bMaximized = isMaximized();  // 记录全屏前是否是最大化窗口
            mipsShowFullScreen();
            if (isFullScreen()) {
                //The X86 platform draws on GiWidget, and the MIPS platform does not need to draw
                QRect deskRect = QApplication::desktop()->availableGeometry();
                m_fsTitlebar->setVisible(true);
                m_fsTitlebar->raise();
                m_fsTitlebar->setFixedWidth(deskRect.width());
            }
        }
        if (!bFromUI) {
            reflectActionToUI(actionKind);
        }

        activateWindow();
        //Set focus back to main window after full screen, Prevent focus from going to the toolbar
        setFocus();

        // fixed bug 103560
        // the window state change signal is not sent under wayland, so call directly here
        // if the problem is fixed in the future, please remove this code
        if (utils::check_wayland_env()) {
            m_pToolbox->updateFullState();
        }

        break;
    }

    case ActionFactory::ActionKind::PlaylistRemoveItem: {
        m_pPlaylist->removeClickedItem(bIsShortcut);
        break;
    }

    case ActionFactory::ActionKind::PlaylistOpenItemInFM: {
        m_pPlaylist->openItemInFM();
        break;
    }

    case ActionFactory::ActionKind::PlaylistItemInfo: {
        m_pPlaylist->showItemInfo();
        break;
    }

    case ActionFactory::ActionKind::ClockwiseFrame: {
        auto old = m_pEngine->videoRotation();
        m_pEngine->setVideoRotation((old + 90) % 360);
        break;
    }
    case ActionFactory::ActionKind::CounterclockwiseFrame: {
        auto old = m_pEngine->videoRotation();
        m_pEngine->setVideoRotation(((old - 90) + 360) % 360);
        break;
    }

    case ActionFactory::ActionKind::OrderPlay: {
        Settings::get().setInternalOption("playmode", 0);
        m_pEngine->playlist().setPlayMode(PlaylistModel::PlayMode::OrderPlay);
        break;
    }
    case ActionFactory::ActionKind::ShufflePlay: {
        Settings::get().setInternalOption("playmode", 1);
        m_pEngine->playlist().setPlayMode(PlaylistModel::PlayMode::ShufflePlay);
        break;
    }
    case ActionFactory::ActionKind::SinglePlay: {
        Settings::get().setInternalOption("playmode", 2);
        m_pEngine->playlist().setPlayMode(PlaylistModel::PlayMode::SinglePlay);
        break;
    }
    case ActionFactory::ActionKind::SingleLoop: {
        Settings::get().setInternalOption("playmode", 3);
        m_pEngine->playlist().setPlayMode(PlaylistModel::PlayMode::SingleLoop);
        break;
    }
    case ActionFactory::ActionKind::ListLoop: {
        Settings::get().setInternalOption("playmode", 4);
        m_pEngine->playlist().setPlayMode(PlaylistModel::PlayMode::ListLoop);
        break;
    }

    case ActionFactory::ActionKind::ZeroPointFiveTimes: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_dPlaySpeed = 0.5;
            m_pEngine->setPlaySpeed(m_dPlaySpeed);
            m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
        }
        break;
    }
    case ActionFactory::ActionKind::OneTimes: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_dPlaySpeed = 1.0;
            m_pEngine->setPlaySpeed(m_dPlaySpeed);
            m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
        }
        break;
    }
    case ActionFactory::ActionKind::OnePointTwoTimes: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_dPlaySpeed = 1.2;
            m_pEngine->setPlaySpeed(m_dPlaySpeed);
            m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
        }
        break;
    }
    case ActionFactory::ActionKind::OnePointFiveTimes: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_dPlaySpeed = 1.5;
            m_pEngine->setPlaySpeed(m_dPlaySpeed);
            m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
        }
        break;
    }
    case ActionFactory::ActionKind::Double: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_dPlaySpeed = 2.0;
            m_pEngine->setPlaySpeed(m_dPlaySpeed);
            m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
        }
        break;
    }

    case ActionFactory::ActionKind::Stereo: {
        m_pEngine->changeSoundMode(Backend::SoundMode::Stereo);
        m_pCommHintWid->updateWithMessage(tr("Stereo"));
        break;
    }
    case ActionFactory::ActionKind::LeftChannel: {
        m_pEngine->changeSoundMode(Backend::SoundMode::Left);
        m_pCommHintWid->updateWithMessage(tr("Left channel"));
        break;
    }
    case ActionFactory::ActionKind::RightChannel: {
        m_pEngine->changeSoundMode(Backend::SoundMode::Right);
        m_pCommHintWid->updateWithMessage(tr("Right channel"));
        break;
    }

    case ActionFactory::ActionKind::DefaultFrame: {
        m_pEngine->setVideoAspect(-1.0);
        break;
    }
    case ActionFactory::ActionKind::Ratio4x3Frame: {
        m_pEngine->setVideoAspect(4.0 / 3.0);
        break;
    }
    case ActionFactory::ActionKind::Ratio16x9Frame: {
        m_pEngine->setVideoAspect(16.0 / 9.0);
        break;
    }
    case ActionFactory::ActionKind::Ratio16x10Frame: {
        m_pEngine->setVideoAspect(16.0 / 10.0);
        break;
    }
    case ActionFactory::ActionKind::Ratio185x1Frame: {
        m_pEngine->setVideoAspect(1.85);
        break;
    }
    case ActionFactory::ActionKind::Ratio235x1Frame: {
        m_pEngine->setVideoAspect(2.35);
        break;
    }

    case ActionFactory::ActionKind::ToggleMute: {
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()
                && !m_pEngine->currFileIsAudio()) {
            slotUnsupported();
        } else {
            m_pToolbox->changeMuteState();
        }
        break;
    }

    case ActionFactory::ActionKind::VolumeUp: {
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()
                && !m_pEngine->currFileIsAudio()) {
            slotUnsupported();
        } else {
            //使用鼠标滚轮调节音量时会执行此步骤
            if (m_iAngleDelta != 0) m_pToolbox->calculationStep(m_iAngleDelta);
            m_pToolbox->volumeUp();
            m_iAngleDelta = 0;
        }
        break;
    }

    case ActionFactory::ActionKind::VolumeDown: {
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()
                && !m_pEngine->currFileIsAudio()) {
            slotUnsupported();
        } else {
            //使用鼠标滚轮调节音量时会执行此步骤
            if (m_iAngleDelta != 0) m_pToolbox->calculationStep(m_iAngleDelta);
            m_pToolbox->volumeDown();
            m_iAngleDelta = 0;
        }
        break;
    }

    case ActionFactory::ActionKind::GotoPlaylistSelected: {
        m_pEngine->playSelected(args[0].toInt());
        break;
    }

    case ActionFactory::ActionKind::GotoPlaylistNext: {
        //防止焦点在上/下一曲按钮上切换时焦点跳到下一个按钮上
        //下同
        setFocus();
        if (m_bIsFree == false)
            return ;

        m_bIsFree = false;
        if (isFullScreen() || isMaximized()) {
            m_bMovieSwitchedInFsOrMaxed = true;
        }
        m_pEngine->next();

        break;
    }

    case ActionFactory::ActionKind::GotoPlaylistPrev: {
        setFocus();
        if (m_bIsFree == false)
            return ;

        m_bIsFree = false;
        if (isFullScreen() || isMaximized()) {
            m_bMovieSwitchedInFsOrMaxed = true;
        }
        m_pEngine->prev();
        break;
    }

    case ActionFactory::ActionKind::SelectTrack: {
        Q_ASSERT(args.size() == 1);
        m_pEngine->selectTrack(args[0].toInt());
        m_pCommHintWid->updateWithMessage(tr("Track: %1").arg(args[0].toInt() + 1));
        if (!bFromUI) {
            reflectActionToUI(actionKind);
        }
        break;
    }

    case ActionFactory::ActionKind::MatchOnlineSubtitle: {
        m_pEngine->loadOnlineSubtitle(m_pEngine->playlist().currentInfo().url);
        break;
    }

    case ActionFactory::ActionKind::SelectSubtitle: {
        Q_ASSERT(args.size() == 1);
        m_pEngine->selectSubtitle(args[0].toInt() - 1);
        if (!m_pEngine->isSubVisible())
            m_pEngine->toggleSubtitle();
        if (!bFromUI) {
            reflectActionToUI(actionKind);
        }
        break;
    }

    case ActionFactory::ActionKind::ChangeSubCodepage: {
        Q_ASSERT(args.size() == 1);
        m_pEngine->setSubCodepage(args[0].toString());
        if (!bFromUI) {
            reflectActionToUI(actionKind);
        }
        break;
    }

    case ActionFactory::ActionKind::HideSubtitle: {
        m_pEngine->toggleSubtitle();
        break;
    }

    case ActionFactory::ActionKind::SubDelay: {
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
            slotUnsupported();
            break;
        }
        if (m_pEngine->playingMovieInfo().subs.isEmpty()) {
            m_pCommHintWid->updateWithMessage(tr("Unable to adjust the subtitle"));
            break;
        }
        m_pEngine->setSubDelay(0.5);
        double dDelay = m_pEngine->subDelay();
        m_pCommHintWid->updateWithMessage(tr("Subtitle %1: %2s")
                                          .arg(dDelay > 0.0 ? tr("delayed") : tr("advanced")).arg(dDelay > 0.0 ? dDelay : -dDelay));
        break;
    }

    case ActionFactory::ActionKind::SubForward: {
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
            slotUnsupported();
            break;
        }
        if (m_pEngine->playingMovieInfo().subs.isEmpty()) {
            m_pCommHintWid->updateWithMessage(tr("Unable to adjust the subtitle"));
            break;
        }
        m_pEngine->setSubDelay(-0.5);
        double dDelay = m_pEngine->subDelay();
        m_pCommHintWid->updateWithMessage(tr("Subtitle %1: %2s")
                                          .arg(dDelay > 0.0 ? tr("delayed") : tr("advanced")).arg(dDelay > 0.0 ? dDelay : -dDelay));
        break;
    }

    case ActionFactory::ActionKind::AccelPlayback: {
        adjustPlaybackSpeed(ActionFactory::ActionKind::AccelPlayback);
        break;
    }

    case ActionFactory::ActionKind::DecelPlayback: {
        adjustPlaybackSpeed(ActionFactory::ActionKind::DecelPlayback);
        break;
    }

    case ActionFactory::ActionKind::ResetPlayback: {
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_dPlaySpeed = 1.0;
            m_pEngine->setPlaySpeed(m_dPlaySpeed);
            setPlaySpeedMenuChecked(ActionFactory::ActionKind::OneTimes);
            m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
        }
        break;
    }

    case ActionFactory::ActionKind::LoadSubtitle: {
        QStringList filename;
#ifndef USE_TEST
        DFileDialog fileDialog(this);
        fileDialog.setNameFilter(tr("Subtitle (*.ass *.aqt *.jss *.gsub *.ssf *.srt *.sub *.ssa *.smi *.usf *.idx)","All (*)"));
        fileDialog.setDirectory(lastOpenedPath());

        if (fileDialog.exec() == QDialog::Accepted) {
            filename = fileDialog.selectedFiles();
        } else {
            break;
        }
#else
        filename = QStringList({"/data/source/deepin-movie-reborn/Hachiko.A.Dog's.Story.ass"});
#endif
        if (QFileInfo(filename[0]).exists()) {
            if (m_pEngine->state() == PlayerEngine::Idle)
                subtitleMatchVideo(filename[0]);
            else {
                auto success = m_pEngine->loadSubtitle(QFileInfo(filename[0]));
                m_pCommHintWid->updateWithMessage(success ? tr("Load successfully") : tr("Load failed"));
            }
        } else {
            m_pCommHintWid->updateWithMessage(tr("Load failed"));
        }
        break;
    }

    case ActionFactory::ActionKind::TogglePause: {
        if(m_pMircastShowWidget && m_pMircastShowWidget->isVisible() ) {
            m_pToolbox->getMircast()->slotPauseDlnaTp();
            break;
        }
        if (windowState() == Qt::WindowFullScreen && QDateTime::currentMSecsSinceEpoch() - m_nFullscreenTime < 500) {
            return;
        } else if(windowState() == Qt::WindowFullScreen) {
            m_nFullscreenTime = QDateTime::currentMSecsSinceEpoch();
        }
        if (m_pEngine->state() == PlayerEngine::Idle && bIsShortcut) {
            if (m_pEngine->getplaylist()->getthreadstate()) {
                qInfo() << "playlist loadthread is running";
                break;
            }
            requestAction(ActionFactory::StartPlay);
        } else {
            if (m_pEngine->state() == PlayerEngine::Paused) {
                //startPlayStateAnimation(true);
                if (!m_bMiniMode) {
                    m_pAnimationlable->playAnimation();
                }
                QTimer::singleShot(160, [ = ]() {
                    m_pEngine->pauseResume();
                });
            } else {
                m_pEngine->pauseResume();
            }
        }
        break;
    }

    case ActionFactory::ActionKind::SeekBackward: {
        if(m_pMircastShowWidget && m_pMircastShowWidget->isVisible() ) {
            m_pToolbox->getMircast()->seekMircast(-5);
            break;
        }
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
            slotUnsupported();
        } else {
            m_pEngine->seekBackward(5);
        }
        break;
    }

    case ActionFactory::ActionKind::SeekForward: {
        if(m_pMircastShowWidget && m_pMircastShowWidget->isVisible() ) {
            m_pToolbox->getMircast()->seekMircast(5);
            break;
        }
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
            slotUnsupported();
        } else {
            m_pEngine->seekForward(5);
        }
        break;
    }

    case ActionFactory::ActionKind::SeekAbsolute: {
        Q_ASSERT(args.size() == 1);
        m_pEngine->seekAbsolute(args[0].toInt());
        break;
    }

    case ActionFactory::ActionKind::Settings: {
        handleSettings(initSettings());
        break;
    }

    case ActionFactory::ActionKind::Screenshot: {
        QImage img = m_pEngine->takeScreenshot();

        QString filePath = Settings::get().screenshotNameTemplate();
        bool bSuccess = false;   //条件编译产生误报(cppcheck)
        if (img.isNull())
            qInfo() << __func__ << "pixmap is null";
        else
            bSuccess = img.save(filePath);
#define POPUP_ADAPTER(icon, text)  do { \
    m_pPopupWid->setIcon(icon);\
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);\
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);\
    QFontMetrics fm(font);\
    auto w = fm.boundingRect(text).width();\
    m_pPopupWid->setMessage(text);\
    m_pPopupWid->resize(w + 112, 52);\
    m_pPopupWid->move((width() - m_pPopupWid->width()) / 2, height() - 127);\
    m_pPopupWid->show();\
        } while (0)
        if (bSuccess) {
            const QIcon icon = QIcon(":/resources/icons/short_ok.svg");
            QString sText = QString(tr("The screenshot is saved"));
            popupAdapter(icon, sText, filePath);
        } else {
            const QIcon icon = QIcon(":/resources/icons/short_fail.svg");
            QString sText = QString(tr("Failed to save the screenshot"));
            popupAdapter(icon, sText);
        }

#undef POPUP_ADAPTER
        break;
    }

    case ActionFactory::ActionKind::GoToScreenshotSolder: {
        QString filePath = Settings::get().screenshotLocation();
        qInfo() << __func__ << filePath;
        QUrl displayUrl = QUrl(filePath);
        QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                                     QStringLiteral("/org/freedesktop/FileManager1"),
                                     QStringLiteral("org.freedesktop.FileManager1"));
        if (interface.isValid()) {
            QStringList list;
            list << displayUrl.toString();
            interface.call("ShowFolders", list, "");
        }
        break;
    }

    case ActionFactory::ActionKind::BurstScreenshot: {
        if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
            slotUnsupported();
        } else {
            startBurstShooting();
        }
        break;
    }

    case ActionFactory::ActionKind::ViewShortcut: {
        QRect rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        QStringList shortcutString;
        QString param1 = "-j=" + ShortcutManager::get().toJson();
        param1.replace("Return", "Enter");
        param1.replace("PgDown", "PageDown");
        param1.replace("PgUp", "PageUp");
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
        shortcutString << param1 << param2;

        if (!m_pShortcutViewProcess) {
            m_pShortcutViewProcess = new QProcess();
        }
        m_pShortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

        connect(m_pShortcutViewProcess, SIGNAL(finished(int)),
                m_pShortcutViewProcess, SLOT(deleteLater()));
        break;
    }

    case ActionFactory::ActionKind::NextFrame: {
        m_pEngine->nextFrame();

        break;
    }

    case ActionFactory::ActionKind::PreviousFrame: {
        m_pEngine->previousFrame();

        break;
    }

    default:
        break;
    }
}

void MainWindow::onBurstScreenshot(const QImage &frame, qint64 timestamp)
{
#define POPUP_ADAPTER(icon, text)  do { \
        m_pPopupWid->setIcon(icon);\
        DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);\
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);\
        QFontMetrics fm(font);\
        auto w = fm.boundingRect(text).width();\
        m_pPopupWid->setMessage(text);\
        m_pPopupWid->resize(w + 70, 52);\
        m_pPopupWid->move((width() - m_pPopupWid->width()) / 2, height() - 127);\
        m_pPopupWid->show();\
    } while (0)

    qInfo() << m_listBurstShoots.size();
    if (!frame.isNull()) {
        QString sMsg = QString(tr("Taking the screenshots, please wait..."));
        m_pCommHintWid->updateWithMessage(sMsg);

        m_listBurstShoots.append(qMakePair(frame, timestamp));
    }

    if (m_listBurstShoots.size() >= 15 || frame.isNull()) {
        disconnect(m_pEngine, &PlayerEngine::notifyScreenshot, this, &MainWindow::onBurstScreenshot);
        m_pEngine->stopBurstScreenshot();
        m_bInBurstShootMode = false;
        m_pToolbox->setEnabled(true);
        //fix bug:129205
//        m_pTitlebar->titlebar()->setEnabled(true);
        if (m_pEventListener) m_pEventListener->setEnabled(!m_bMiniMode);

        if (frame.isNull()) {
            m_listBurstShoots.clear();
            if (!m_bPausedBeforeBurst)
                m_pEngine->pauseResume();
            return;
        }

        int nRet = -1;
        BurstScreenshotsDialog burstScreenshotsDialog(m_pEngine->playlist().currentInfo());
        burstScreenshotsDialog.updateWithFrames(m_listBurstShoots);
#ifdef USE_TEST
        burstScreenshotsDialog.show();
#else
        nRet = burstScreenshotsDialog.exec();
#endif
        qInfo() << "BurstScreenshot done";

        m_listBurstShoots.clear();
        if (!m_bPausedBeforeBurst)
            m_pEngine->pauseResume();

        if (nRet == QDialog::Accepted) {
            QString sPosterPath = burstScreenshotsDialog.savedPosterPath();
            if (QFileInfo::exists(sPosterPath)) {
                const QIcon icon = QIcon(":/resources/icons/short_ok.svg");
                QString sText = QString(tr("The screenshot is saved"));
                popupAdapter(icon, sText, sPosterPath);
            } else {
                const QIcon icon = QIcon(":/resources/icons/short_fail.svg");
                QString sText = QString(tr("Failed to save the screenshot"));
                popupAdapter(icon, sText);
            }
        }
    }
}

void MainWindow::startBurstShooting()
{
    //Repair 40S video corresponding to the corresponding connected screenshot
    if (m_pEngine->duration() <= 40) return;
    m_bInBurstShootMode = true;
    m_pToolbox->setEnabled(false);
//    m_pTitlebar->titlebar()->setEnabled(false);
    if (m_pEventListener) m_pEventListener->setEnabled(false);

    m_bPausedBeforeBurst = m_pEngine->paused();

    connect(m_pEngine, &PlayerEngine::notifyScreenshot, this, &MainWindow::onBurstScreenshot);
    m_pEngine->burstScreenshot();
}

void MainWindow::handleSettings(DSettingsDialog *dsd)
{
#ifndef USE_TEST
    dsd->exec();
    delete dsd;
#else
    dsd->setObjectName("DSettingsDialog");
    dsd ->show();
#endif

    Settings::get().settings()->sync();
}

DSettingsDialog *MainWindow::initSettings()
{
    DSettingsDialog *pDSettingDilog = new DSettingsDialog(this);
    pDSettingDilog->widgetFactory()->registerWidget("selectableEdit", createSelectableLineEditOptionHandle);

    pDSettingDilog->setProperty("_d_QSSThemename", "dark");
    pDSettingDilog->setProperty("_d_QSSFilename", "DSettingsDialog");
    pDSettingDilog->updateSettings(Settings::get().settings());

    //hack:
    QSpinBox *pSpinBox = pDSettingDilog->findChild<QSpinBox *>("OptionDSpinBox");
    if (pSpinBox) {
        pSpinBox->setMinimum(8);
    }

    // hack: reset is set to default by QDialog, which makes lineedit's enter
    // press is responded by reset button
    QPushButton *pPushButton = pDSettingDilog->findChild<QPushButton *>("SettingsContentReset");
    pPushButton->setDefault(false);
    pPushButton->setAutoDefault(false);
    return pDSettingDilog;
}

void MainWindow::play(const QList<QString> &listFiles)
{
    QList<QUrl> lstValid;
    QList<QString> lstDir;
    QList<QString> lstFile;

    if (listFiles.isEmpty())
        m_pEngine->play();

    if (listFiles.count() == 1 && QUrl(listFiles[0]).scheme().startsWith("dvd")) {
        m_dvdUrl = QUrl(listFiles[0]);
        if (!m_pEngine->addPlayFile(m_dvdUrl)) {
            auto msg = QString(tr("Cannot play the disc"));
            m_pCommHintWid->updateWithMessage(msg);
            return;
        } else {
            // todo: Disable toolbar buttons
            auto msg = QString(tr("Reading DVD files..."));
            m_pDVDHintWid->updateWithMessage(msg, true);
        }

        m_pEngine->playByName(m_dvdUrl);
        return;
    }

    for (QString strFile : listFiles) {
        if(QFileInfo(QUrl(strFile).toString()).isDir()){
            lstDir << strFile;
        } else {
            lstFile << strFile;
        }
    }

    lstValid = m_pEngine->addPlayFiles(lstFile);  // 先添加到播放列表再播放

    m_bHaveFile = !lstValid.isEmpty();
    if (m_bHaveFile) {
        //The disposal is false here to prevent the introduction of the folder from blocking
        m_pEngine->playByName(lstValid[0]);
    }

    if (!lstDir.isEmpty()) {
        m_pEngine->blockSignals(true);
        QtConcurrent::run(m_pEngine, &PlayerEngine::addPlayFs, lstDir);
    } else {
        m_bHaveFile = false;
    }
}

void MainWindow::slotFinishedAddFiles(QList<QUrl> lstValid)
{
    if(lstValid.count() > 0 && !m_bHaveFile) {
        if (!isHidden()) {
            activateWindow();
        }
        m_pEngine->playByName(lstValid[0]);
    }
    //The disposal is false here to prevent the introduction of the folder from blocking
    m_bHaveFile = false;
}

void MainWindow::updateProxyGeometry()
{
    QRect view_rect = rect();

    m_pEngine->resize(view_rect.size());

    if (!m_bMiniMode) {
        if (m_pTitlebar) {
            m_pTitlebar->setFixedWidth(view_rect.width());
            m_fsTitlebar->setFixedWidth(view_rect.width());
        }

        if (m_pToolbox) {
            QRect rfs = QRect(5, height() - TOOLBOX_HEIGHT - TOOLBOX_TOP_EXTENT - rect().top() - 5,
                        rect().width() - TOOLBOX_TOP_EXTENT, TOOLBOX_HEIGHT + TOOLBOX_TOP_EXTENT);
            m_pToolbox->setGeometry(rfs);
            m_pToolbox->updateMircastWidget(rfs.topRight());
        }

        if (m_pPlaylist && !m_pPlaylist->toggling()) {
            QRect fixed(view_rect.width() - m_pPlaylist->width() - 10, 76, 344, view_rect.height() - 138);
            m_pPlaylist->setGeometry(fixed);
        }
    }
}

void MainWindow::suspendToolsWindow()
{
    if (!m_bMiniMode) {
        if (m_pPlaylist && m_pPlaylist->state() == PlaylistWidget::Opened)
            return;

//        if (qApp->applicationState() == Qt::ApplicationInactive) {

//        } else {
        // menus  are popped up
        // NOTE: menu keeps focus while hidden, so focusWindow is not used
        if (ActionFactory::get().mainContextMenu()->isVisible() ||
                ActionFactory::get().titlebarMenu()->isVisible())
            return;

        QPoint cursor = mapFromGlobal(QCursor::pos());
        if (m_pToolbox->isVisible()) {
            if (m_pToolbox->getMircast()->isVisible() &&
                    m_pToolbox->getMircast()->geometry().contains(cursor) && !m_bLastIsTouch)
                return;
            if (insideToolsArea(cursor) && !m_bLastIsTouch)
                return;
        } else {
            if (m_pToolbox->geometry().contains(cursor)) {
                return;
            }
        }
//        }

        if (m_pToolbox->anyPopupShown())
            return;

        if (m_pEngine->state() == PlayerEngine::Idle)
            return;

        if (m_autoHideTimer.isActive())
            return;

        if (isFullScreen()) {
            if (qApp->focusWindow() == this->windowHandle()) {
                qApp->setOverrideCursor(Qt::BlankCursor);
            } else {
                qApp->setOverrideCursor(Qt::ArrowCursor);
            }
        }

        m_pToolbox->hide();

        if (m_pToolbox->getMircast()->isVisible())
            m_pToolbox->hideMircastWidget();

        //reset focus to mainWindow when the titlebar and toolbox is hedden
        //the tab focus will be re-executed in the order set
        m_pTitlebar->setFocus();
        m_pTitlebar->hide();        //隐藏操作应放在设置焦点后
        m_fsTitlebar->hide();
    } else {
        if (m_autoHideTimer.isActive())
            return;

        m_pMiniPlayBtn->hide();
        m_pMiniCloseBtn->hide();
        m_pMiniQuitMiniBtn->hide();
    }
}

void MainWindow::resumeToolsWindow()
{
    if (m_pEngine->state() != PlayerEngine::Idle &&
            qApp->applicationState() == Qt::ApplicationActive) {
        // playlist's previous state was Opened
        if (m_pPlaylist && m_pPlaylist->state() != PlaylistWidget::Closed &&
                !frameGeometry().contains(QCursor::pos())) {
            goto _finish;
        }
    }

    qApp->restoreOverrideCursor();
    setCursor(Qt::ArrowCursor);

    if (!m_bMiniMode) {
        if (!m_bTouchChangeVolume) {
            m_pTitlebar->setVisible(!isFullScreen());
            m_fsTitlebar->setVisible(isFullScreen());
            m_fsTitlebar->raise();
            m_pToolbox->show();
        } else {
            m_pToolbox->hide();
        }
    } else {
        m_pMiniPlayBtn->show();
        m_pMiniCloseBtn->show();
        m_pMiniQuitMiniBtn->show();
    }

_finish:
    m_autoHideTimer.start(AUTOHIDE_TIMEOUT);
}

void MainWindow::checkOnlineState(const bool bIsOnline)
{
    if (!bIsOnline) {
        this->sendMessage(QIcon(":/icons/deepin/builtin/icons/ddc_warning_30px.svg"), QObject::tr("Network disconnected"));
    }
}

void MainWindow::checkOnlineSubtitle(const OnlineSubtitle::FailReason reason)
{
    if (OnlineSubtitle::FailReason::NoSubFound == reason) {
        m_pCommHintWid->updateWithMessage(tr("No matching online subtitles"));
    }
}

void MainWindow::checkWarningMpvLogsChanged(const QString sPrefix, const QString sText)
{
    QString warningMessage(sText);
    qInfo() << "checkWarningMpvLogsChanged" << sText;
    if (warningMessage.contains(QString("Hardware does not support image size 3840x2160"))) {
        requestAction(ActionFactory::TogglePause);

        DDialog *pDialog = new DDialog;
        pDialog->setFixedWidth(440);
        QImage icon = utils::LoadHiDPIImage(":/resources/icons/warning.svg");
        QPixmap pix = QPixmap::fromImage(icon);
        pDialog->setIcon(QIcon(pix));
        pDialog->setMessage(tr("4K video may be stuck"));
        pDialog->addButton(tr("OK"), true, DDialog::ButtonRecommend);
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
        effect->setOffset(0, 4);
        effect->setColor(QColor(0, 145, 255, 76));
        effect->setBlurRadius(4);
        pDialog->getButton(0)->setFixedWidth(340);
        pDialog->getButton(0)->setGraphicsEffect(effect);
#ifndef USE_TEST
        pDialog->exec();
#else
        pDialog->show();
        pDialog->deleteLater();
#endif
        QTimer::singleShot(500, [ = ]() {
            //startPlayStateAnimation(true);
            if (!m_bMiniMode) {
                m_pAnimationlable->playAnimation();
            }
            m_pEngine->pauseResume();
        });
    }

}

void MainWindow::slotdefaultplaymodechanged(const QString &sKey, const QVariant &value)
{
    if (sKey != "base.play.playmode") {
        qInfo() << "Settings key error";
        return;
    }
    QPointer<DSettingsOption> modeOpt = Settings::get().settings()->option("base.play.playmode");
    QString sMode = modeOpt->data("items").toStringList()[value.toInt()];
    if (sMode == tr("Order play")) {
        m_pEngine->playlist().setPlayMode(PlaylistModel::OrderPlay);
        reflectActionToUI(ActionFactory::OrderPlay);
    } else if (sMode == tr("Shuffle play")) {
        m_pEngine->playlist().setPlayMode(PlaylistModel::ShufflePlay);
        reflectActionToUI(ActionFactory::ShufflePlay);
    } else if (sMode == tr("Single play")) {
        m_pEngine->playlist().setPlayMode(PlaylistModel::SinglePlay);
        reflectActionToUI(ActionFactory::SinglePlay);
    } else if (sMode == tr("Single loop")) {
        m_pEngine->playlist().setPlayMode(PlaylistModel::SingleLoop);
        reflectActionToUI(ActionFactory::SingleLoop);
    } else if (sMode == tr("List loop")) {
        m_pEngine->playlist().setPlayMode(PlaylistModel::ListLoop);
        reflectActionToUI(ActionFactory::ListLoop);
    }
}

void MainWindow::onSetDecodeModel(const QString &key, const QVariant &value)
{
    Q_UNUSED(key);
    MpvProxy* pMpvProxy = nullptr;
    pMpvProxy = dynamic_cast<MpvProxy*>(m_pEngine->getMpvProxy());
    if(pMpvProxy)
        pMpvProxy->setDecodeModel(value);
}

void MainWindow::onRefreshDecode()
{
    MpvProxy* pMpvProxy = nullptr;
    pMpvProxy =  dynamic_cast<MpvProxy*>(m_pEngine->getMpvProxy());
    if(pMpvProxy)
        pMpvProxy->refreshDecode();
}

void MainWindow::my_setStayOnTop(const QWidget *pWidget, bool bOn)
{
    Q_ASSERT(pWidget);

    const auto display = QX11Info::display();
    const auto screen = QX11Info::appScreen();

    const auto wmStateAtom = XInternAtom(display, kAtomNameWmState, false);
    const auto stateAboveAtom = XInternAtom(display, kAtomNameWmStateAbove, false);
    const auto stateStaysOnTopAtom = XInternAtom(display,
                                                 kAtomNameWmStateStaysOnTop,
                                                 false);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));

    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = wmStateAtom;
    xev.xclient.display = display;
    xev.xclient.window = pWidget->winId();
    xev.xclient.format = 32;

    xev.xclient.data.l[0] = bOn ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
    xev.xclient.data.l[1] = stateAboveAtom;
    xev.xclient.data.l[2] = stateStaysOnTopAtom;
    xev.xclient.data.l[3] = 1;

    XSendEvent(display,
               QX11Info::appRootWindow(screen),
               false,
               SubstructureRedirectMask | SubstructureNotifyMask,
               &xev);
    XFlush(display);
}

void MainWindow::slotmousePressTimerTimeOut()
{
    m_mousePressTimer.stop();
    if (m_bMiniMode || m_bInBurstShootMode || !m_bMousePressed)
        return;

    if (insideToolsArea(QCursor::pos()))
        return;

    resumeToolsWindow();
    m_bMousePressed = false;
    m_bIsTouch = false;
}

void MainWindow::slotPlayerStateChanged()
{
    bool bAudio = false;
    PlayerEngine *pEngine = dynamic_cast<PlayerEngine *>(sender());
    if (!pEngine) return;
    setInit(pEngine->state() != PlayerEngine::Idle);
    resumeToolsWindow();
    updateWindowTitle();

    // delayed checking if engine is still idle, in case other videos are schedulered (next/prev req)
    // and another resize event will happen after that
    QTimer::singleShot(100, [ = ]() {
        if (pEngine->state() == PlayerEngine::Idle && !m_bMiniMode
                && windowState() == Qt::WindowNoState && !isFullScreen()) {
            this->setMinimumSize(QSize(614, 500));
            this->resize(850, 600);
        }
    });

    if (m_pEngine->playlist().count() > 0) {
        bAudio =  m_pEngine->currFileIsAudio();
    }
    if (m_pEngine->state() == PlayerEngine::CoreState::Playing && bAudio) {
        m_pMovieWidget->startPlaying();
    } else if ((m_pEngine->state() == PlayerEngine::CoreState::Paused) && bAudio) {
        m_pMovieWidget->pausePlaying();
    } else if (pEngine->state() == PlayerEngine::CoreState::Idle) {
        m_pMovieWidget->stopPlaying();
    }
}

void MainWindow::slotFocusWindowChanged()
{
    if (qApp->focusWindow() != windowHandle())
        suspendToolsWindow();
    else
        resumeToolsWindow();
}

void MainWindow::slotFileLoaded()
{
    PlayerEngine *pEngine = dynamic_cast<PlayerEngine *>(sender());
    if (!pEngine) return;
    m_nRetryTimes = 0;
    if (utils::check_wayland_env() && windowState() == Qt::WindowNoState && m_lastRectInNormalMode.isValid()) {
        const MovieInfo &mi = pEngine->playlist().currentInfo().mi;
        if (!m_bMiniMode) {
            if (utils::check_wayland_env()) {
                //wayland下存在最大化>全屏->全屏->最小化，窗口超出界面问题。且现在用不着videosize大小窗口
                m_lastRectInNormalMode.setSize({850, 600});
            } else {
                m_lastRectInNormalMode.setSize({mi.width, mi.height});
            }
        }
    }
    this->resizeByConstraints();

    if (utils::check_wayland_env()) {
        QDesktopWidget desktop;
        if (desktop.screenCount() > 1) {
            if (!isFullScreen() && !isMaximized() && !m_bMiniMode) {
                QRect geom = qApp->desktop()->availableGeometry(this);
                move((geom.width() - this->width()) / 2, (geom.height() - this->height()) / 2);
            }
        }
    }
    m_bIsFree = true;
}

void MainWindow::slotUrlpause(bool bStatus)
{
    if (bStatus) {
        auto msg = QString(tr("Buffering..."));
        m_pCommHintWid->updateWithMessage(msg);
    }
}

void MainWindow::slotMuteChanged(bool bMute)
{
    m_pEngine->setMute(bMute);

    if (bMute) {
        m_pCommHintWid->updateWithMessage(tr("Mute"));
    } else {
        m_pCommHintWid->updateWithMessage(tr("Volume: %1%").arg(m_nDisplayVolume));   // 取消静音时显示音量提示
    }
}

/*void MainWindow::slotAwaacelModeChanged(const QString &sKey, const QVariant &value)
{
    if (sKey != "base.play.hwaccel") {
        qInfo() << "Settings key error";
        return;
    }

    setHwaccelMode(value);
}*/

void MainWindow::slotVolumeChanged(int nVolume)
{
    m_nDisplayVolume = nVolume;
    m_pEngine->changeVolume(nVolume);
    if (m_pPresenter) {
        m_pPresenter->slotvolumeChanged();
    }

    if (nVolume == 0) {
        m_pCommHintWid->updateWithMessage(tr("Mute"));
    } else {
        m_pCommHintWid->updateWithMessage(tr("Volume: %1%").arg(nVolume));
    }
}

void MainWindow::slotWMChanged(QString msg)
{
    if (msg.contains("deepin metacity")) {
        m_bIsWM = false;
    } else {
        m_bIsWM = true;
    }

    m_pAnimationlable->setWM(m_bIsWM);
    m_pCommHintWid->setWM(m_bIsWM);
}

void MainWindow::slotMediaError()
{
    m_pCommHintWid->updateWithMessage(tr("Cannot open file or stream"));
    m_pEngine->playlist().remove(m_pEngine->playlist().current());
}

void MainWindow::mircastSuccess(QString name)
{
    if (m_pEngine->state() == PlayerEngine::Playing)
        m_pEngine->pauseResume();
    updateActionsState();
    m_pMircastShowWidget->setDeviceName(name);
    m_pMircastShowWidget->show();
    m_pToolbox->hideMircastWidget();
}

void MainWindow::exitMircast()
{
    if (m_pEngine->state() == PlayerEngine::Playing)
        m_pEngine->pauseResume();
    m_pEngine->seekAbsolute(m_pToolbox->getSlider()->value());
    updateActionsState();
    m_pToolbox->getMircast()->slotExitMircast();
    m_pMircastShowWidget->hide();
}

void MainWindow::checkErrorMpvLogsChanged(const QString sPrefix, const QString sText)
{
    QString sErrorMessage(sText);
    qInfo() << "checkErrorMpvLogsChanged" << sText;
    if (sErrorMessage.toLower().contains(QString("avformat_open_input() failed"))) {
        //do nothing
    } else if (sErrorMessage.toLower().contains(QString("fail")) && sErrorMessage.toLower().contains(QString("open"))
               && !sErrorMessage.toLower().contains(QString("dlopen"))) {
        m_pCommHintWid->updateWithMessage(tr("Cannot open file or stream"));
        m_pEngine->playlist().remove(m_pEngine->playlist().current());
    } else if (sErrorMessage.toLower().contains(QString("fail")) &&
               (sErrorMessage.toLower().contains(QString("format")))) {
        //Open the URL there is three cases of legal paths, illegal paths, and semi-legal
        //paths, which only processes the prefix legality, the suffix is not legal
        //please refer to other places to modify
        //powered by xxxxp
        if (m_pEngine->playlist().currentInfo().mi.title.isEmpty()) {
            m_pCommHintWid->updateWithMessage(tr("Parse failed"));
            m_pEngine->playlist().remove(m_pEngine->playlist().current());
        } else {
            if (m_nRetryTimes < 10) {
                m_nRetryTimes++;
                requestAction(ActionFactory::ActionKind::StartPlay);
            } else {
                m_nRetryTimes = 0;
                m_pCommHintWid->updateWithMessage(tr("Invalid file"));
                m_pEngine->playlist().remove(m_pEngine->playlist().current());
            }
        }
    } else if (sErrorMessage.toLower().contains(QString("moov atom not found"))) {
        m_pCommHintWid->updateWithMessage(tr("Invalid file"));
    } else if (sErrorMessage.toLower().contains(QString("couldn't open dvd device"))) {
        m_pCommHintWid->updateWithMessage(tr("Please insert a CD/DVD"));
    } else if (sErrorMessage.toLower().contains(QString("incomplete frame")) ||
               sErrorMessage.toLower().contains(QString("MVs not available"))) {
    } else if ((sErrorMessage.toLower().contains(QString("can't"))) &&
               (sErrorMessage.toLower().contains(QString("open")))) {
        m_pCommHintWid->updateWithMessage(tr("No video file found"));
    }
}

void MainWindow::closeEvent(QCloseEvent *pEvent)
{
    qInfo() << __func__;
    if(m_pMircastShowWidget&&m_pMircastShowWidget->isVisible()) {
        slotExitMircast();
    }
    if (m_bInBurstShootMode) {
        pEvent->ignore();
        return;
    }

    if (m_nLastCookie > 0) {
        utils::UnInhibitStandby(m_nLastCookie);
        qInfo() << "uninhibit cookie" << m_nLastCookie;
        m_nLastCookie = 0;
    }

    if (Settings::get().isSet(Settings::ResumeFromLast)) {
        int nCur = 0;
        nCur = m_pEngine->playlist().current();
        if (nCur >= 0) {
            Settings::get().setInternalOption("playlist_pos", nCur);
        }
    }
    //关闭窗口时保存音量值
    Settings::get().setInternalOption("global_volume", m_nDisplayVolume > 100 ? 100 : m_nDisplayVolume);
    m_pEngine->savePlaybackPosition();

    pEvent->accept();
    if (utils::check_wayland_env()) {
#ifndef _LIBDMR_
        if (Settings::get().isSet(Settings::ClearWhenQuit)) {
            m_pEngine->playlist().clearPlaylist();
        } else {
            //persistently save current playlist
            m_pEngine->playlist().savePlaylist();
        }
#endif
        // xcb close slow so add this for wayland  by xxj
        DMainWindow::closeEvent(pEvent);
        m_pEngine->stop();
        disconnect(m_pEngine, nullptr, nullptr, nullptr);
        disconnect(&m_pEngine->playlist(), nullptr, nullptr, nullptr);
        if (m_pEngine) {
            delete m_pEngine;
            m_pEngine = nullptr;
        }
        CompositingManager::get().setTestFlag(true);
        /*lmh0724临时规避退出崩溃问题*/
        QApplication::quit();
        _Exit(0);
    }
}

void MainWindow::wheelEvent(QWheelEvent *pEvent)
{
    if (insideToolsArea(pEvent->pos()) || insideResizeArea(pEvent->globalPos()))
        return;

    if (m_pToolbox->isInMircastWidget(pEvent->pos()))
        return;

    if (m_pPlaylist && m_pPlaylist->state() == PlaylistWidget::Opened) {
        pEvent->ignore();
        return;
    }

    if (pEvent->buttons() == Qt::NoButton && pEvent->modifiers() == Qt::NoModifier && m_pToolbox->getVolSliderIsHided()) {
        m_iAngleDelta = pEvent->angleDelta().y() ;
        requestAction(pEvent->angleDelta().y() > 0 ? ActionFactory::VolumeUp : ActionFactory::VolumeDown);
    }
}

void MainWindow::focusInEvent(QFocusEvent *pEvent)
{
    resumeToolsWindow();
}

void MainWindow::hideEvent(QHideEvent *pEvent)
{
    QMainWindow::hideEvent(pEvent);
}

void MainWindow::showEvent(QShowEvent *pEvent)
{
    m_pAnimationlable->raise();
    m_pTitlebar->raise();
    m_pToolbox->raise();
    if (m_pPlaylist) {
        m_pPlaylist->raise();
    }
    //判断屏幕可用坐标与应用的geometry是否有交集，没有就设置到可用屏幕坐标中
    QRect geoRect = geometry();
    QRect deskRect = QApplication::desktop()->availableGeometry(geoRect.topLeft());

    if(!deskRect.intersects(geoRect)) {
        setGeometry(QRect(deskRect.x(), deskRect.y(), geoRect.width(), geoRect.width()));
    }
    resumeToolsWindow();

    if (!qgetenv("FLATPAK_APPID").isEmpty()) {
        qInfo() << "workaround for flatpak";
        if (m_pPlaylist->isVisible())
            updateProxyGeometry();
    }

    QMainWindow::showEvent(pEvent);
}

void MainWindow::resizeByConstraints(bool bForceCentered)
{
    if (m_pEngine->state() == PlayerEngine::Idle || m_pEngine->playlist().count() == 0) {
        m_pTitlebar->setTitletxt(QString());
        m_fsTitlebar->setTitletxt(QString());
        return;
    }

    if (m_bMiniMode || isFullScreen() || isMaximized()) {
        return;
    }

    qInfo() << __func__;
    updateWindowTitle();
    //lmh0710修复窗口变成影片分辨率问题
    if (utils::check_wayland_env()) {
        return;
    }

    const MovieInfo &mi = m_pEngine->playlist().currentInfo().mi;
    QSize vidoeSize = m_pEngine->videoSize();
    if (CompositingManager::get().platform() == Platform::Mips)
        m_pCommHintWid->syncPosition();
    if (vidoeSize.isEmpty()) {
        vidoeSize = QSize(mi.width, mi.height);
        qInfo() << mi.width << mi.height;
    }

    auto geom = qApp->desktop()->availableGeometry(this);
    if (vidoeSize.width() > geom.width() || vidoeSize.height() > geom.height()) {
        vidoeSize.scale(geom.width(), geom.height(), Qt::KeepAspectRatio);
    }

    qInfo() << "original: " << size() << "requested: " << vidoeSize;
    if (size() == vidoeSize)
        return;

    if (bForceCentered) {
        QRect r;
        r.setSize(vidoeSize);
        r.moveTopLeft({(geom.width() - r.width()) / 2, (geom.height() - r.height()) / 2});
        if (utils::check_wayland_env()) {
            this->setGeometry(r);
            this->move(r.x(), r.y());
            this->resize(r.width(), r.height());
        }
    } else {
        if (utils::check_wayland_env()) {
            QRect r = this->geometry();
            r.setSize(vidoeSize);
            this->setGeometry(r);
            this->move(r.x(), r.y());
            this->resize(r.width(), r.height());
        }
    }
}

// 若长≥高,则长≤528px　　　若长≤高,则高≤528px.
// 简而言之,只看最长的那个最大为528px.
void MainWindow::updateSizeConstraints()
{
    QSize size;

    if (m_bMiniMode) {
        size = QSize(40, 40);
    } else {
        size = QSize(614, 500);
    }
    this->setMinimumSize(size);
}

void MainWindow::updateGeometryNotification(const QSize &sz)
{
    QString sMsg = QString("%1x%2").arg(sz.width()).arg(sz.height());
    if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
        m_pCommHintWid->updateWithMessage(sMsg);
    }

    //Wayland's player is normal to active, so add a judgment here
    if ((windowState() == Qt::WindowNoState || utils::check_wayland_env() && windowState() == Qt::WindowActive)
            &&  !m_isSettingMiniMode && !m_bMiniMode && !m_bMaximized) {
        m_lastRectInNormalMode = geometry();
    }
}

void MainWindow::LimitWindowize()
{
    if (!m_bMiniMode && (geometry().width() == 380 || geometry().height() == 380)) {
        setGeometry(m_lastRectInNormalMode);
    }
}

void MainWindow::resizeEvent(QResizeEvent *pEvent)
{
    qInfo() << __func__ << geometry();
    if (utils::check_wayland_env()) {
        if (m_pToolbox) {
            m_pToolbox->setFixedWidth(this->width() - 10);
        }
    }
    // modify 4.1  Limit video to mini mode size by thx
    LimitWindowize();

    updateSizeConstraints();
    updateProxyGeometry();
    QTimer::singleShot(0, [ = ]() {
        updateWindowTitle();
    });
    updateGeometryNotification(geometry().size());
    //add by heyi
    /*******
     * 之前为修改全屏下呼出右键菜单任务栏不消失问题
     * 此处修改存在逻辑错误，未判断窗口初始状态是否为置顶
     * 此处先注释掉完成当前版本功能，sp3开发人员根据后期开发状态进行修改
     *******/
//    if (!isFullScreen()) {
//        my_setStayOnTop(this, false);
//    }
    m_pMovieWidget->resize(rect().size());
    m_pMovieWidget->move(0, 0);

    m_pMircastShowWidget->resize(rect().size());
    m_pMircastShowWidget->move(0, 0);

    m_pAnimationlable->move(QPoint((width() - m_pAnimationlable->width()) / 2,
                                   (height() - m_pAnimationlable->height()) / 2));
}

void MainWindow::updateWindowTitle()
{
    if (m_pEngine->state() != PlayerEngine::Idle) {
        const MovieInfo &mi = m_pEngine->playlist().currentInfo().mi;
        QString sTitle = m_pTitlebar->fontMetrics().elidedText(mi.title,
                                                               Qt::ElideMiddle, m_pTitlebar->contentsRect().width() - 400);
        m_pTitlebar->setTitletxt(sTitle);
        m_pTitlebar->setTitleBarBackground(true);
        m_fsTitlebar->setTitletxt(sTitle);
    } else {
        m_pTitlebar->setTitletxt(QString());
        m_pTitlebar->setTitleBarBackground(false);
        m_fsTitlebar->setTitletxt(QString());
    }
    m_pTitlebar->setProperty("idle", m_pEngine->state() == PlayerEngine::Idle);
}

void MainWindow::moveEvent(QMoveEvent *pEvent)
{
    qInfo() << __func__ << "进入moveEvent";
    QWidget::moveEvent(pEvent);

    if (CompositingManager::get().platform() != Platform::X86) {
        QPoint relativePoint = mapToGlobal(QPoint(0, 0));
        m_pToolbox->updateSliderPoint(relativePoint);
        m_pCommHintWid->syncPosition();
    }

    updateGeometryNotification(geometry().size());
}

void MainWindow::keyPressEvent(QKeyEvent *pEvent)
{
    if (m_pPlaylist && (m_pPlaylist->state() == PlaylistWidget::Opened) && pEvent->modifiers() == Qt::NoModifier) {
        if (pEvent) {
            m_pPlaylist->updateSelectItem(pEvent->key());
        }
        pEvent->setAccepted(true);
    }
    QWidget::keyPressEvent(pEvent);
}

void MainWindow::keyReleaseEvent(QKeyEvent *pEvent)
{
    QWidget::keyReleaseEvent(pEvent);
}

static bool s_bAfterDblClick = false;

void MainWindow::capturedMousePressEvent(QMouseEvent *pEvent)
{
    m_bMouseMoved = false;
    m_bMousePressed = false;
    if (CompositingManager::get().platform() != Platform::X86 && !utils::check_wayland_env()) {
        m_pCommHintWid->hide();
        m_pPopupWid->hide();
    }
    if (qApp->focusWindow() == nullptr) return;

    if (pEvent->buttons() == Qt::LeftButton) {
        m_bMousePressed = true;
    }

    m_posMouseOrigin = mapToGlobal(pEvent->pos());
}

void MainWindow::capturedMouseReleaseEvent(QMouseEvent *pEvent)
{
    if (m_bIsTouch) {
        m_bLastIsTouch = true;
        m_bIsTouch = false;

        if (m_bTouchChangeVolume) {
            m_bTouchChangeVolume = false;
            m_pToolbox->setVisible(true);
        }

        if (m_bProgressChanged) {
            m_pToolbox->updateSlider();   //手势释放时改变影片进度
            m_bProgressChanged = false;
        }
    } else {
        m_bLastIsTouch = false;
    }

    if (m_bDelayedResizeByConstraint) {
        m_bDelayedResizeByConstraint = false;

        QTimer::singleShot(0, [ = ]() {
            this->setMinimumSize({0, 0});
            this->resizeByConstraints(true);
        });
    }

    if (!m_bMousePressed) {
        s_bAfterDblClick = false;
        m_bMouseMoved = false;
    }

    if (qApp->focusWindow() == nullptr || !m_bMousePressed)
        return;

    m_bMousePressed = false;

    //NOTE: If the mouseMoveEvent of the titlebar is triggered
    // reset the status here, otherwise it cannot respond to the mini mode shortcut
    if (m_pTitlebar->geometry().contains(pEvent->pos()))
        m_bMouseMoved = false;
}

void MainWindow::capturedKeyEvent(QKeyEvent *pEvent)
{
    if (pEvent->key() == Qt::Key_Tab) {
        if (!isFullScreen()) {
            m_pTitlebar->show();
        } else {
            m_fsTitlebar->show();
        }
        m_pToolbox->show();
        m_autoHideTimer.start(AUTOHIDE_TIMEOUT);  //如果点击tab键，重置计时器
    }
}

void MainWindow::mousePressEvent(QMouseEvent *pEvent)
{
    m_bMouseMoved = false;
    m_bMousePressed = false;
    if (CompositingManager::get().platform() != Platform::X86) {
        m_pCommHintWid->hide();
        m_pPopupWid->hide();
    }
    if (qApp->focusWindow() == nullptr)
        return;
    if (pEvent->buttons() == Qt::LeftButton) {
        m_bMousePressed = true;
        if (!m_mousePressTimer.isActive() && m_bIsTouch) {
            m_mousePressTimer.stop();

            m_nLastPressX = mapToGlobal(QCursor::pos()).x();
            m_nLastPressY = mapToGlobal(QCursor::pos()).y();
            qInfo() << __func__ << "已经进入触屏按下事件" << m_nLastPressX << m_nLastPressY;
            m_mousePressTimer.start();
        }
    }

    m_posMouseOrigin = mapToGlobal(pEvent->pos());
    m_pressPoint = pEvent->pos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    /// NOTE: 为了其它控件的鼠标操作与MainWindow一致，统一使用capturedMouseReleaseEvent()捕获鼠标释放
    /// 事件，若无特殊要求，请尽量在capturedMouseReleaseEvent()进行处理。

    // 以下代码貌似没什么用，可以考虑去掉
    static bool bFlags = true;
    if (bFlags) {
        repaint();
        bFlags = false;
    }

    qInfo() << __func__ << "进入mouseReleaseEvent";

    if (!insideResizeArea(ev->globalPos()) && !m_bMouseMoved && (m_pPlaylist->state() != PlaylistWidget::Opened)) {
        if (!insideToolsArea(ev->pos())) {
            m_delayedMouseReleaseTimer.start(120);
        } else {
            if (m_pEngine->state() == PlayerEngine::CoreState::Idle && !insideToolsArea(ev->pos())) {
                m_delayedMouseReleaseTimer.start(120);
            }
        }
    }

    m_bMouseMoved = false;

    QWidget::mouseReleaseEvent(ev);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *pEvent)
{
    qInfo() << __func__ << "进入mouseDoubleClickEvent";
    if (!m_bMiniMode && this->m_pEngine->getplaylist()->getthreadstate()) {
        qInfo() << "playlist loadthread is running";
        return;
    }
    //投屏时双击操作不做处理
    if(m_pMircastShowWidget && m_pMircastShowWidget->isVisible()) return;
    if (!m_bMiniMode && !m_bInBurstShootMode) {
        m_delayedMouseReleaseTimer.stop();
        if (m_pEngine->state() == PlayerEngine::Idle) {
            requestAction(ActionFactory::StartPlay);
        } else {
            requestAction(ActionFactory::ToggleFullscreen, false, {}, true);
        }
        pEvent->accept();
        s_bAfterDblClick = true;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *pEvent)
{
    if (m_bStartMini)
        return;
    qInfo() << __func__ << "进入mouseMoveEvent";
    QPoint ptCurr = mapToGlobal(pEvent->pos());
    QPoint ptDelta = ptCurr - this->m_posMouseOrigin;

    if (qAbs(ptDelta.x()) < 5 && qAbs(ptDelta.y()) < 5) { //避免误触
        return;
    }

    if (m_bIsTouch && isFullScreen()) { //全屏时才触发滑动改变音量和进度的操作
        if (qAbs(ptDelta.x()) > qAbs(ptDelta.y())
                && m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            m_bTouchChangeVolume = false;
            m_pToolbox->updateProgress(ptDelta.x());     //改变进度条显示
            this->m_posMouseOrigin = ptCurr;
            m_bProgressChanged = true;
            return;
        } else if (qAbs(ptDelta.x()) < qAbs(ptDelta.y())) {
            if (ptDelta.y() > 0) {
                m_bTouchChangeVolume = true;
                requestAction(ActionFactory::ActionKind::VolumeDown);
            } else {
                m_bTouchChangeVolume = true;
                requestAction(ActionFactory::ActionKind::VolumeUp);
            }

            this->m_posMouseOrigin = ptCurr;
            return;
        }
    }
    QWidget::mouseMoveEvent(pEvent);

    this->m_posMouseOrigin = ptCurr;
    m_bMouseMoved = true;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *pEvent)
{
    qInfo() << __func__ << "进入contextMenuEvent";
    if (m_bMiniMode || m_bInBurstShootMode)
        return;

    if (insideToolsArea(pEvent->pos()))
        return;

    if (utils::check_wayland_env()) {
        if (windowHandle()->property("_d_dwayland_staysontop").toBool() != m_bWindowAbove) {
            m_bWindowAbove = !m_bWindowAbove;
            reflectActionToUI(ActionFactory::WindowAbove);
        }
    } else {
        //通过窗口id查询窗口状态是否置顶，同步右键菜单中的选项状态
        QProcess above;
        QStringList options;
        options << "-c" << QString("xprop -id %1 | grep '_NET_WM_STATE(ATOM)'").arg(winId());
        above.start("bash", options);
        if (above.waitForStarted() && above.waitForFinished()) {
            QString drv = QString::fromUtf8(above.readAllStandardOutput().trimmed().constData());
            if (drv.contains("_NET_WM_STATE_ABOVE") != m_bWindowAbove) {
    //            requestAction(ActionFactory::WindowAbove);
                m_bWindowAbove = drv.contains("_NET_WM_STATE_ABOVE");
                reflectActionToUI(ActionFactory::WindowAbove);
            }
        }
    }

    if(m_pMircastShowWidget->isVisible() ) {//投屏中屏蔽全屏、迷你模式，置顶菜单
        QList<ActionFactory::ActionKind> lstActId;
        lstActId << ActionFactory::ToggleFullscreen << ActionFactory::ToggleMiniMode << ActionFactory::WindowAbove;
        for(ActionFactory::ActionKind id: lstActId) {
            QList<QAction *> listActs;
            listActs = ActionFactory::get().findActionsByKind(id);
            if(listActs.size()<=0) {
                continue;
            }
            for(QAction *act: listActs) {
                act->setEnabled(false);
            }
        }
        //倍速播放、画面、声音、字幕、截图
        emit frameMenuEnable(false);
        emit playSpeedMenuEnable(false);
        emit subtitleMenuEnable(false);
        emit soundMenuEnable(false);
    }

    resumeToolsWindow();
    QTimer::singleShot(0, [ = ]() {
        qApp->restoreOverrideCursor();
        ActionFactory::get().mainContextMenu()->popup(QCursor::pos());
    });
    pEvent->accept();

//此段为通过xcb接口查询窗口状态，nItem为状态列表中的个数，properties为返回状态列表
//代码暂时无法实现需求，勿删
//    const auto display = QX11Info::display();
//    const auto screen = QX11Info::appScreen();
//    Atom atom = XInternAtom(display, "_NET_WM_STATE", true);
//    Atom type;
//    int format;
//    unsigned long nItem, bytesAfter;
//    unsigned char *properties = NULL;
//    XGetWindowProperty(display, QX11Info::appRootWindow(screen), atom, 0, (~0L), False, AnyPropertyType, &type, &format, &nItem, &bytesAfter, &properties);
//    qInfo() << atom << nItem;
//    int iItem;
//    for (iItem = 0; iItem < nItem; ++iItem)
//        qInfo() << ((long *)(properties))[iItem];
}

bool MainWindow::insideToolsArea(const QPoint &p)
{
    //XTL:这里测一下看看非x86平台否合并代码
    if (CompositingManager::get().platform() == Platform::X86) {
        return m_pTitlebar->geometry().contains(p) || m_pToolbox->geometry().contains(p) || m_pToolbox->volumeSlider()->geometry().contains(p) ||
                m_pMiniPlayBtn->geometry().contains(p)|| m_pMiniCloseBtn->geometry().contains(p) || m_pMiniQuitMiniBtn->geometry().contains(p);
    } else {
        return m_pTitlebar->geometry().contains(p) || m_pToolbox->rect().contains(p) || m_pToolbox->geometry().contains(p) || m_pToolbox->volumeSlider()->geometry().contains(p) ||
                m_pMiniPlayBtn->geometry().contains(p)|| m_pMiniCloseBtn->geometry().contains(p) || m_pMiniQuitMiniBtn->geometry().contains(p);
    }
}

QMargins MainWindow::dragMargins() const
{
    return QMargins {MOUSE_MARGINS, MOUSE_MARGINS, MOUSE_MARGINS, MOUSE_MARGINS};
}

bool MainWindow::insideResizeArea(const QPoint &globalPos)
{
    const QRect window_visible_rect = frameGeometry() - dragMargins();
    return !window_visible_rect.contains(globalPos);
}

void MainWindow::delayedMouseReleaseHandler()
{
    if ((!s_bAfterDblClick && !m_bLastIsTouch) || m_bMiniMode)
        requestAction(ActionFactory::TogglePause, false, {}, true);

    s_bAfterDblClick = false;
}

void MainWindow::prepareSplashImages()
{
    m_imgBgDark = utils::LoadHiDPIImage(":/resources/icons/dark/init-splash.svg");
    m_imgBgLight = utils::LoadHiDPIImage(":/resources/icons/light/init-splash.svg");
}

void MainWindow::subtitleMatchVideo(const QString &sFileName)
{
    QString sVideoName = sFileName;
    // Search for video files with the same name as the subtitles and play the video file.
    QFileInfo subfileInfo(sFileName);
    QDir dir(subfileInfo.canonicalPath());
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo info = list.at(i);
        qInfo() << info.absoluteFilePath() << endl;
//        if (info.completeBaseName() == subfileInfo.completeBaseName()) {
        if (subfileInfo.fileName().contains(info.completeBaseName())) {
            sVideoName = info.absoluteFilePath();
        } else {
            sVideoName = nullptr;
        }
    }

    QFileInfo vfileInfo(sVideoName);
    if (vfileInfo.exists()) {
        Settings::get().setGeneralOption("last_open_path", vfileInfo.path());

        play({sVideoName});

        // Select the current subtitle display
        const PlayingMovieInfo &pmf = m_pEngine->playingMovieInfo();
        for (const SubtitleInfo &sub : pmf.subs) {
            if (sub["external"].toBool()) {
                QString path = sub["external-filename"].toString();
                if (path == subfileInfo.canonicalFilePath()) {
                    m_pEngine->selectSubtitle(pmf.subs.indexOf(sub));
                    break;
                }
            }
        }
    } else {
        m_pCommHintWid->updateWithMessage(tr("Please load the video first"));
    }
}

void MainWindow::defaultplaymodeinit()
{
    QPointer<DSettingsOption> modeOpt = Settings::get().settings()->option("base.play.playmode");
    int nModeId = modeOpt->value().toInt();
    QString sMode = modeOpt->data("items").toStringList()[nModeId];
    if (sMode == tr("Order play")) {
        requestAction(ActionFactory::OrderPlay);
        reflectActionToUI(ActionFactory::OrderPlay);
    } else if (sMode == tr("Shuffle play")) {
        requestAction(ActionFactory::ShufflePlay);
        reflectActionToUI(ActionFactory::ShufflePlay);
    } else if (sMode == tr("Single play")) {
        requestAction(ActionFactory::SinglePlay);
        reflectActionToUI(ActionFactory::SinglePlay);
    } else if (sMode == tr("Single loop")) {
        requestAction(ActionFactory::SingleLoop);
        reflectActionToUI(ActionFactory::SingleLoop);
    } else if (sMode == tr("List loop")) {
        requestAction(ActionFactory::ListLoop);
        reflectActionToUI(ActionFactory::ListLoop);
    }
}

void MainWindow::decodeInit()
{
    MpvProxy* pMpvProxy = nullptr;
    pMpvProxy = dynamic_cast<MpvProxy*>(m_pEngine->getMpvProxy());

    if(!pMpvProxy)
        return;

    //崩溃检测
    bool bcatch = Settings::get().settings()->getOption(QString("set.start.crash")).toBool();
    if (bcatch) {
        pMpvProxy->setDecodeModel(DecodeMode::AUTO);
        Settings::get().settings()->setOption(QString("base.decode.select"),DecodeMode::AUTO);
    } else {
        int value = Settings::get().settings()->getOption(QString("base.decode.select")).toInt();
        pMpvProxy->setDecodeModel(value);
    }
}

void MainWindow::popupAdapter(QIcon icon, QString sText, QString path)
{
    m_pPopupWid->setIcon(icon);
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    QFontMetrics fm(font);
    auto w = fm.boundingRect(sText).width();
    if (!path.isEmpty()) {
        m_pPopupWid->setImgPath(path);
        m_pPopupWid->resize(w + 112, 40);
    } else {
        m_pPopupWid->resize(w + 70, 40);
    }
    m_pPopupWid->setMessage(sText);
    m_pPopupWid->move((width() - m_pPopupWid->width()) / 2, height() - 127);
    m_pPopupWid->show();
    m_pPopupWid->raise();
}

QString MainWindow::lastOpenedPath()
{
    QString lastPath = Settings::get().generalOption("last_open_path").toString();
    QDir lastDir(lastPath);
    if (lastPath.isEmpty() || !lastDir.exists()) {
        lastPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        QDir newLastDir(lastPath);
        if (!newLastDir.exists()) {
            lastPath = QDir::currentPath();
        }
    }

    return lastPath;
}

void MainWindow::paintEvent(QPaintEvent *pEvent)
{
    QPainter painter(this);
    QRectF bgRect;
    bgRect.setSize(size());
    const QPalette pal = QGuiApplication::palette();//this->palette();
    QColor bgColor = pal.color(QPalette::Window);

    if (CompositingManager::get().platform() == Platform::X86) {
        QPainterPath path;
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            if (m_pEngine->state() != PlayerEngine::Idle && !m_pToolbox->isVisible()) {
                path.addRect(bgRect);
                painter.fillPath(path, Qt::black);
            } else {
                path.addRect(bgRect);
                painter.fillPath(path, Qt::white);
            }
        }
    }

    if (m_pEngine->state() == PlayerEngine::Idle) {
        QImage bg = QIcon::fromTheme("deepin-movie").pixmap(130, 130).toImage();
        //和产品、ui商议深色主题下去除深色背景效果
//        if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
//            QImage img = utils::LoadHiDPIImage(":/resources/icons/dark/init-splash-bac.svg");
//            QPointF pos = bgRect.center() - QPoint(img.width() / 2, img.height() / 2) / devicePixelRatioF();
//            painter.drawImage(pos, img);
//        }
        QPointF pos = bgRect.center() - QPoint(bg.width() / 2, bg.height() / 2) / devicePixelRatioF();
        painter.drawImage(pos, bg);
    }

    QMainWindow::paintEvent(pEvent);
}

void MainWindow::toggleUIMode()
{
    //迷你模式关闭动画及控件
    m_pAnimationlable->hide();
    m_pToolbox->closeAnyPopup();
    //判断窗口是否靠边停靠（靠边停靠不支持MINI模式）thx
    QRect deskrect = QApplication::desktop()->availableGeometry();
    QPoint windowPos = pos();
    if (this->geometry() != deskrect) {
        if (windowPos.x() == 0 && (windowPos.y() == 0 ||
                                   (abs(windowPos.y() + this->geometry().height() - deskrect.height()) < 50))) {
            if (abs(this->geometry().width() - deskrect.width() / 2) < 50) {
                m_pCommHintWid->updateWithMessage(tr("Please exit smart dock"));
                return ;
            }

        }
        if ((abs(windowPos.x() + this->geometry().width() - deskrect.width()) < 50)  &&
                (windowPos.y()  == 0 || abs(windowPos.y() + this->geometry().height() - deskrect.height()) < 50)) {
            if (abs(this->geometry().width() - deskrect.width() / 2) < 50) {
                m_pCommHintWid->updateWithMessage(tr("Please exit smart dock"));
                return ;
            }
        }
    }

    m_bMiniMode = !m_bMiniMode;
    m_isSettingMiniMode = true;
    m_pEngine->toggleRoundedClip(!m_bMiniMode);

    if (utils::check_wayland_env()) {
        Qt::WindowFlags flags = windowFlags();
        if (m_bMiniMode) {
            flags |= Qt::X11BypassWindowManagerHint;
            m_preMiniWindowState = windowState();
            setWindowState(Qt::WindowNoState);
            setWindowFlags(flags);
            show();
        } else {
            flags &= ~Qt::X11BypassWindowManagerHint;
            setWindowFlags(flags);
            show();
            if (m_preMiniWindowState == Qt::WindowMaximized) {
                move(0, 0);
                showMaximized();
            } else if (m_preMiniWindowState & Qt::WindowFullScreen) {
                move(0, 0);
                showFullScreen();
            } else {
                showNormal();
            }
        }
    }
    m_isSettingMiniMode = false;

    qInfo() << __func__ << m_bMiniMode;

    if (m_bMiniMode) {
        m_pTitlebar->titlebar()->setDisableFlags(Qt::WindowMaximizeButtonHint);
    } else {
        m_pTitlebar->titlebar()->setDisableFlags(nullptr);
    }

    m_pTitlebar->setVisible(!m_bMiniMode);
    m_fsTitlebar->setVisible(!m_bMiniMode);

    m_pMiniPlayBtn->setVisible(m_bMiniMode);
    m_pMiniCloseBtn->setVisible(m_bMiniMode);
    m_pMiniQuitMiniBtn->setVisible(m_bMiniMode);

    m_pMiniPlayBtn->setEnabled(m_bMiniMode);
    m_pMiniCloseBtn->setEnabled(m_bMiniMode);
    m_pMiniQuitMiniBtn->setEnabled(m_bMiniMode);

    m_pMiniPlayBtn->raise();
    m_pMiniCloseBtn->raise();
    m_pMiniQuitMiniBtn->raise();

    resumeToolsWindow();

    if (m_bMiniMode) {
        m_pCommHintWid->setAnchorPoint(QPoint(15, 11));    //迷你模式下提示位置稍有不同
        updateSizeConstraints();
        //设置等比缩放
        setEnableSystemResize(false);
        m_nStateBeforeMiniMode = SBEM_None;

        if (isFullScreen()) {
            m_nStateBeforeMiniMode |= SBEM_Fullscreen;
            setWindowState(windowState() & ~Qt::WindowFullScreen);
            this->setWindowState(Qt::WindowNoState);
            setFocus();
            if (utils::check_wayland_env()) {
                m_pToolbox->updateFullState();
            }
        } else if (isMaximized()) {
            m_nStateBeforeMiniMode |= SBEM_Maximized;
            showNormal();
        } else {
            m_lastRectInNormalMode = geometry();
        }

        if (!m_bWindowAbove) {
            m_nStateBeforeMiniMode |= SBEM_Above;
            requestAction(ActionFactory::WindowAbove);
        }

        QSize sz = QSize(380, 380);
        if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
            qreal ratio = 1920 * 1.0 / 1080;
            auto vid_size = m_pEngine->videoSize();

            if (vid_size.height() > 0 && vid_size.width() >= vid_size.height()) {
                ratio = vid_size.width() / static_cast<qreal>(vid_size.height());
                sz = QSize(380, static_cast<int>(380 / ratio) + 1);
            } else if (vid_size.height() > 0 && vid_size.width() < vid_size.height()) {
                ratio = vid_size.width() / static_cast<qreal>(vid_size.height());
                sz = QSize(380, static_cast<int>(380 * ratio) + 1);
            } else {
                sz = QSize(380, static_cast<int>(380 / ratio) + 1);
            }
        }

        QRect geom = {0, 0, 0, 0};
        if (m_lastRectInNormalMode.isValid()) {
            geom = m_lastRectInNormalMode;
        }

        geom.setSize(sz);
        setGeometry(geom);
        if (geom.x() < 0) {
            geom.moveTo(0, geom.y());
        }
        if (geom.y() < 0) {
            geom.moveTo(geom.x(), 0);
        }

        QRect deskGeom = qApp->desktop()->availableGeometry(this);
        move((deskGeom.width() - this->width()) / 2, (deskGeom.height() - this->height()) / 2); //迷你模式下窗口居中 by zhuyuliang
        resize(geom.width(), geom.height());

        m_pMiniPlayBtn->move(sz.width() - 12 - m_pMiniPlayBtn->width(),
                             sz.height() - 10 - m_pMiniPlayBtn->height());
        m_pMiniCloseBtn->move(sz.width() - 15 - m_pMiniCloseBtn->width(), 10);
        m_pMiniQuitMiniBtn->move(14, sz.height() - 10 - m_pMiniQuitMiniBtn->height());
    } else {
        m_pCommHintWid->setAnchorPoint(QPoint(30, 58));
        setEnableSystemResize(true);
        if (m_nStateBeforeMiniMode & SBEM_Maximized) {
            //迷你模式切换最大化时，先恢复原来窗口大小
            if (m_lastRectInNormalMode.isValid()) {
                setGeometry(m_lastRectInNormalMode);
            } else {
                resizeByConstraints();
            }
            showMaximized();
        } else if (m_nStateBeforeMiniMode & SBEM_Fullscreen) {
            setWindowState(windowState() | Qt::WindowFullScreen);
            setFocus();
            if (utils::check_wayland_env()) {
                m_pToolbox->updateFullState();
            }
        } else {
            if (m_pToolbox->listBtn()->isChecked()) {
                m_pToolbox->listBtn()->setChecked(false);
            }

            if (m_lastRectInNormalMode.isValid()) {
                QRect deskRect = QApplication::desktop()->availableGeometry(m_lastRectInNormalMode.topLeft());
                if(m_lastRectInNormalMode.intersects(deskRect)) {
                    setGeometry(m_lastRectInNormalMode);
                } else {
                    setGeometry(QRect(deskRect.x(), deskRect.y(), m_lastRectInNormalMode.width(), m_lastRectInNormalMode.height()));
                }
            } else {
                resizeByConstraints();
            }
        }

        if (m_nStateBeforeMiniMode & SBEM_Above) {
            requestAction(ActionFactory::WindowAbove);
        }

        if (m_nStateBeforeMiniMode & SBEM_PlaylistOpened &&
                m_pPlaylist->state() == PlaylistWidget::Closed) {
            if (m_nStateBeforeMiniMode & SBEM_Fullscreen) {
                QTimer::singleShot(100, [ = ]() {
                    requestAction(ActionFactory::TogglePlaylist);
                });
            }
        }
        m_nStateBeforeMiniMode = SBEM_None;
    }

    m_bStartMini = false;
}

void MainWindow::miniButtonClicked(const QString &id)
{
    qInfo() << id;
    if (id == "play") {
        if (m_pEngine->state() == PlayerEngine::CoreState::Idle) {
            requestAction(ActionFactory::ActionKind::StartPlay);
        } else {
            requestAction(ActionFactory::ActionKind::TogglePause);
        }

    } else if (id == "close") {
        close();

    } else if (id == "quit_mini") {
        requestAction(ActionFactory::ActionKind::ToggleMiniMode);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *ev)
{
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *pEvent)
{
    //add by heyi 拖动进来时先初始化窗口
    //firstPlayInit();
    qInfo() << pEvent->mimeData()->formats();
    if (!pEvent->mimeData()->hasUrls()) {
        return;
    }

    QList<QString> lstFile;
    QList<QUrl> urls = pEvent->mimeData()->urls();

    for (QUrl strUrl : urls) {
        lstFile << strUrl.path();
    }

    if (urls.count() == 1) {
        // check if the dropped file is a subtitle.
        QFileInfo fileInfo(urls.first().toLocalFile());
        if (m_pEngine->isSubtitle(fileInfo.absoluteFilePath())) {
            // Search for video files with the same name as the subtitles and play the video file.
            if(m_pEngine->state() != PlayerEngine::CoreState::Idle
                    && m_pEngine->playlist().currentInfo().mi.isRawFormat()) {
                return;
            }
            else if (m_pEngine->state() == PlayerEngine::Idle)
                subtitleMatchVideo(urls.first().toLocalFile());
            else {
                bool succ = m_pEngine->loadSubtitle(fileInfo);
                m_pCommHintWid->updateWithMessage(succ ? tr("Load successfully") : tr("Load failed"));
            }

            return;
        }
    }

    play(lstFile);

    pEvent->acceptProposedAction();
}

void MainWindow::setInit(bool bInit)
{
    if (m_bInited != bInit) {
        m_bInited = bInit;
        emit initChanged();
    }
}

QString MainWindow::probeCdromDevice()
{
    QFile mountFile("/proc/mounts");
    if (mountFile.open(QIODevice::ReadOnly) == false) {
        return QString();
    }
    do {
        QString sLine = mountFile.readLine();
        if (sLine.indexOf("/dev/sr") != -1 || sLine.indexOf("/dev/cdrom") != -1) {  //说明存在光盘的挂载。
            return sLine.split(" ").at(0);        //A B C 这样的格式，取部分
        }
    } while (!mountFile.atEnd());
    mountFile.close();
    return QString();
}

void MainWindow::diskRemoved(QString strDiskName)
{
    QString sCurrFile;
    if (m_pEngine->getplaylist()->count() <= 0) {
        return;
    }
    sCurrFile = m_pEngine->getplaylist()->currentInfo().url.toString();

    if (sCurrFile.contains(strDiskName)/* && m_pEngine->state() == PlayerEngine::Playing*/)
        m_pCommHintWid->updateWithMessage(tr("The CD/DVD has been ejected"));
}

void MainWindow::sleepStateChanged(bool bSleep)
{
    qInfo() << __func__ << bSleep;

    //if (m_bStateInLock) {                //休眠唤醒后会先执行锁屏操作,如果已经进行锁屏操作则忽略休眠唤醒信号
     //   m_bStartSleep = bSleep;
     //   m_pEngine->seekAbsolute(static_cast<int>(m_pEngine->elapsed()));
    //    return;
    //}
    //休眠退出投屏
    if(bSleep && m_pMircastShowWidget && m_pMircastShowWidget->isVisible()) {
        slotExitMircast();
    }
    if (bSleep && m_pEngine->state() == PlayerEngine::CoreState::Playing) {
        m_bStartSleep = true;
        requestAction(ActionFactory::ActionKind::TogglePause);
    } else if (!bSleep && m_pEngine->state() == PlayerEngine::CoreState::Paused) {
        m_bStartSleep = false;
        m_pEngine->seekAbsolute(static_cast<int>(m_pEngine->elapsed()));      //保证休眠后不管是否播放都不会卡帧
    }
}

void MainWindow::lockStateChanged(bool bLock)
{
    qInfo() << __func__ << bLock;
    //锁屏退出投屏
    if(bLock && m_pMircastShowWidget && m_pMircastShowWidget->isVisible()) {
        slotExitMircast();
    }
    if (bLock && m_pEngine->state() == PlayerEngine::CoreState::Playing && !m_bStateInLock) {
        m_bStateInLock = true;
        requestAction(ActionFactory::ActionKind::TogglePause);
    } else if (!bLock && m_pEngine->state() == PlayerEngine::CoreState::Paused && m_bStateInLock) {
        m_bStateInLock = false;
        QTimer::singleShot(500, [=](){
            //龙芯5000使用命令sudo rtcwake -l -m mem -s 20, 待机唤醒后无dBus信号“PrepareForSleep”发出,加入seek保证解锁后播放不会卡帧
            m_pEngine->seekAbsolute(static_cast<int>(m_pEngine->elapsed()));
            requestAction(ActionFactory::ActionKind::TogglePause);
        });
    }
}

void MainWindow::initMember()
{
    m_pPopupWid = nullptr;
    m_pFullScreenTimeLayout = nullptr;
    m_pTitlebar = nullptr;
    m_fsTitlebar = nullptr;
    m_pToolbox = nullptr;
    m_pPlaylist = nullptr;
    m_pEngine = nullptr;
    m_pAnimationlable = nullptr;
    m_pEventMonitor = nullptr;
    m_pEventListener = nullptr;
    m_pDVDHintWid = nullptr;
    m_pCommHintWid = nullptr;
    m_pShortcutViewProcess = nullptr;
    m_pDBus = nullptr;
    m_pPresenter = nullptr;
    m_pMovieWidget = nullptr;
    m_pMircastShowWidget = nullptr;
    m_bInBurstShootMode = false;
    m_bPausedBeforeBurst = false;

#ifdef __mips__
    m_pMiniPlayBtn = nullptr;
    m_pMiniCloseBtn = nullptr;
    m_pMiniQuitMiniBtn = nullptr;
#else
    m_pMiniPlayBtn = nullptr;
    m_pMiniCloseBtn = nullptr;
    m_pMiniQuitMiniBtn = nullptr;
#endif

    m_bMiniMode = false;
    m_bInited = false;
    m_bMovieSwitchedInFsOrMaxed = false;
    m_bDelayedResizeByConstraint = false;
    m_bWindowAbove = false;
    m_bMouseMoved = false;
    m_bMousePressed = false;
    m_bQuitfullscreenflag = false;
    m_bStartMini = false;
    m_bProgressChanged = false;
    m_bLastIsTouch = false;
    m_bTouchChangeVolume = false;
    m_bIsFree = true;
    m_bIsTouch = false;
    m_bStartAnimation = false;
    m_bStateInLock = false;
    m_bStartSleep = false;
    m_bMaximized = false;
    m_bHaveFile = false;

    m_nDisplayVolume = 100;
    m_nLastPressX = 0;
    m_nLastPressY = 0;
    m_nStateBeforeMiniMode = 0;
    m_nLastCookie = 0;
    m_nPowerCookie = 0;
    m_dPlaySpeed = 1.0;
    m_iAngleDelta = 0;
    m_nFullscreenTime = 0;

    m_lastWindowState = Qt::WindowNoState;

    m_dvdUrl.clear();
    m_listOpenFiles.clear();
    m_sCurrentHwdec.clear();
    m_listBurstShoots.clear();
}

void MainWindow::adjustPlaybackSpeed(ActionFactory::ActionKind actionKind)
{
    if (m_pEngine->state() != PlayerEngine::CoreState::Idle) {
        if (actionKind == ActionFactory::ActionKind::AccelPlayback) {
            m_dPlaySpeed = qMin(2.0, m_dPlaySpeed + 0.1);
        } else if (actionKind == ActionFactory::ActionKind::DecelPlayback) {
            m_dPlaySpeed = qMax(0.1, m_dPlaySpeed - 0.1);
        }

        m_pEngine->setPlaySpeed(m_dPlaySpeed);
        if (qFuzzyCompare(0.5, m_dPlaySpeed)) {
            setPlaySpeedMenuChecked(ActionFactory::ActionKind::ZeroPointFiveTimes);
        } else if (qFuzzyCompare(1.0, m_dPlaySpeed)) {
            setPlaySpeedMenuChecked(ActionFactory::ActionKind::OneTimes);
        } else if (qFuzzyCompare(1.2, m_dPlaySpeed)) {
            setPlaySpeedMenuChecked(ActionFactory::ActionKind::OnePointTwoTimes);
        } else if (qFuzzyCompare(1.5, m_dPlaySpeed)) {
            setPlaySpeedMenuChecked(ActionFactory::ActionKind::OnePointFiveTimes);
        } else if (qFuzzyCompare(2.0, m_dPlaySpeed)) {
            setPlaySpeedMenuChecked(ActionFactory::ActionKind::Double);
        } else {
            setPlaySpeedMenuUnchecked();
        }
        m_pCommHintWid->updateWithMessage(tr("Speed: %1x").arg(m_dPlaySpeed));
    }
}

void MainWindow::setPlaySpeedMenuChecked(ActionFactory::ActionKind actionKind)
{
    QList<QAction *> listActs = ActionFactory::get().findActionsByKind(actionKind);
    auto p = listActs.begin();
    (*p)->setChecked(true);
}

void MainWindow::setPlaySpeedMenuUnchecked()
{
    QList<QAction *> listActs;
    {
        listActs = ActionFactory::get().findActionsByKind(ActionFactory::ActionKind::ZeroPointFiveTimes);
        auto p = listActs.begin();
        if ((*p)->isChecked()) {
            (*p)->setChecked(false);
        }
    }
    {
        listActs = ActionFactory::get().findActionsByKind(ActionFactory::ActionKind::OneTimes);
        auto p = listActs.begin();
        if ((*p)->isChecked()) {
            (*p)->setChecked(false);
        }
    }
    {
        listActs = ActionFactory::get().findActionsByKind(ActionFactory::ActionKind::OnePointTwoTimes);
        auto p = listActs.begin();
        if ((*p)->isChecked()) {
            (*p)->setChecked(false);
        }
    }
    {
        listActs = ActionFactory::get().findActionsByKind(ActionFactory::ActionKind::OnePointFiveTimes);
        auto p = listActs.begin();
        if ((*p)->isChecked()) {
            (*p)->setChecked(false);
        }
    }
    {
        listActs = ActionFactory::get().findActionsByKind(ActionFactory::ActionKind::Double);
        auto p = listActs.begin();
        if ((*p)->isChecked()) {
            (*p)->setChecked(false);
        }
    }

}

void MainWindow::setMusicShortKeyState(bool bState)
{
    ActionFactory::ActionKind actionKind;
    foreach (auto action, this->actions()) {
        actionKind = (ActionFactory::ActionKind)action->property("kind").toInt();
        switch (actionKind) {
        case ActionFactory::Screenshot:
        case ActionFactory::BurstScreenshot:
        case ActionFactory::GoToScreenshotSolder:
        case ActionFactory::DefaultFrame:
        case ActionFactory::Ratio4x3Frame:
        case ActionFactory::Ratio16x9Frame:
        case ActionFactory::Ratio16x10Frame:
        case ActionFactory::Ratio185x1Frame:
        case ActionFactory::Ratio235x1Frame:
        case ActionFactory::ClockwiseFrame:
        case ActionFactory::CounterclockwiseFrame:
        case ActionFactory::NextFrame:
        case ActionFactory::PreviousFrame:
            action->setEnabled(bState);
        }
    }
}

void MainWindow::onSysLockState(QString, QVariantMap key2value, QStringList)
{
    if (m_bStartSleep) {
        m_bStateInLock = true;       //如果进入了休眠状态后进入锁屏,则默认执行了暂停操作
    }

    if (key2value.value("Locked").value<bool>() && m_pEngine->state() == PlayerEngine::CoreState::Playing) {
        m_bStateInLock = true;
        requestAction(ActionFactory::TogglePause);
    } else if (!key2value.value("Locked").value<bool>() && m_bStateInLock) {
        m_bStateInLock = false;
        requestAction(ActionFactory::TogglePause);
    }
}

void MainWindow::slotProperChanged(QString, QVariantMap key2value, QStringList)
{
    qInfo() << __func__ << key2value;
    if (key2value.value("Active").value<bool>() && m_pEngine->state() == PlayerEngine::CoreState::Playing) {
        m_pEngine->seekAbsolute(m_pEngine->elapsed());
    }
}

void MainWindow::slotUnsupported()
{
    if (m_pEngine->state() != PlayerEngine::CoreState::Idle)
        m_pCommHintWid->updateWithMessage(tr("The action is not supported in this video"));
}

void MainWindow::slotInvalidFile(QString strFileName)
{
    static int showTime = -1000;

    showTime += 1000;

    QTimer::singleShot(showTime, [=]{
       showTime = showTime - 1000;
       m_pCommHintWid->updateWithMessage(QString(tr("Invalid file: %1").arg(strFileName)));
    });
}

void MainWindow::slotUpdateMircastState(int state, QString msg)
{
    switch (state) {
    case MIRCAST_SUCCESSED: //投屏成功
    {
        mircastSuccess(msg);
        emit frameMenuEnable(false);
        emit playSpeedMenuEnable(false);
        emit subtitleMenuEnable(false);
        emit soundMenuEnable(false);
    }
        break;
    case MIRCAST_EXIT://投屏退出
    {
        slotExitMircast();
    }
        break;
    case MIRCAST_CONNECTION_FAILED://投屏连接失败
    {
        const QIcon icon = QIcon(":/resources/icons/short_fail.svg");
        QString sText = QString(tr("Connection failed"));
        popupAdapter(icon, sText);
        slotExitMircast();
    }
        break;
    case MIRCAST_DISCONNECTIONED://投屏丢失连接
    {
        m_pCommHintWid->updateWithMessage(tr("Miracast disconnected"));
        slotExitMircast();
    }
        break;
    default:
        break;
    }
}

void MainWindow::slotExitMircast()
{
    exitMircast();
    emit frameMenuEnable(true);
    emit playSpeedMenuEnable(true);
    emit subtitleMenuEnable(true);
    emit soundMenuEnable(true);
}

void MainWindow::updateGeometry(CornerEdge edge, QPoint pos)
{
    bool bKeepRatio = engine()->state() != PlayerEngine::CoreState::Idle;
    QRect oldGeom = frameGeometry();
    QRect geom = frameGeometry();
    qreal ratio = static_cast<qreal>(geom.width()) / geom.height();

    // disable edges
    switch (edge) {
    case CornerEdge::BottomEdge:
    case CornerEdge::TopEdge:
    case CornerEdge::LeftEdge:
    case CornerEdge::RightEdge:
    case CornerEdge::NoneEdge:
        return;
    default:
        break;
    }

    if (bKeepRatio) {
        QSize size = engine()->videoSize();
        if (size.isEmpty()) {
            const auto &MovieInfo = engine()->playlist().currentInfo().mi;
            size = QSize(MovieInfo.width, MovieInfo.height);
        }

        ratio = size.width() / static_cast<qreal>(size.height());
        switch (edge) {
        case CornerEdge::TopLeftCorner:
            geom.setLeft(pos.x());
            geom.setTop(static_cast<int>(geom.bottom() - geom.width() / ratio));
            break;
        case CornerEdge::BottomLeftCorner:
        case CornerEdge::LeftEdge:
            geom.setLeft(pos.x());
            geom.setHeight(static_cast<int>(geom.width() / ratio));
            break;
        case CornerEdge::BottomRightCorner:
        case CornerEdge::RightEdge:
            geom.setRight(pos.x());
            geom.setHeight(static_cast<int>(geom.width() / ratio));
            break;
        case CornerEdge::TopRightCorner:
        case CornerEdge::TopEdge:
            geom.setTop(pos.y());
            geom.setWidth(static_cast<int>(geom.height() * ratio));
            break;
        case CornerEdge::BottomEdge:
            geom.setBottom(pos.y());
            geom.setWidth(static_cast<int>(geom.height() * ratio));
            break;
        default:
            break;
        }
    } else {
        switch (edge) {
        case CornerEdge::BottomLeftCorner:
            geom.setBottomLeft(pos);
            break;
        case CornerEdge::TopLeftCorner:
            geom.setTopLeft(pos);
            break;
        case CornerEdge::LeftEdge:
            geom.setLeft(pos.x());
            break;
        case CornerEdge::BottomRightCorner:
            geom.setBottomRight(pos);
            break;
        case CornerEdge::RightEdge:
            geom.setRight(pos.x());
            break;
        case CornerEdge::TopRightCorner:
            geom.setTopRight(pos);
            break;
        case CornerEdge::TopEdge:
            geom.setTop(pos.y());
            break;
        case CornerEdge::BottomEdge:
            geom.setBottom(pos.y());
            break;
        default:
            break;
        }
    }

    QSize min = minimumSize();
    if (oldGeom.width() <= min.width() && geom.left() > oldGeom.left()) {
        geom.setLeft(oldGeom.left());
    }
    if (oldGeom.height() <= min.height() && geom.top() > oldGeom.top()) {
        geom.setTop(oldGeom.top());
    }

    geom.setWidth(qMax(geom.width(), min.width()));
    geom.setHeight(qMax(geom.height(), min.height()));
    updateContentGeometry(geom);
    updateGeometryNotification(geom.size());
}

void MainWindow::setPresenter(Presenter *pPresenter)
{
    m_pPresenter = pPresenter;
    m_pPresenter->slotvolumeChanged();
}

int MainWindow::getDisplayVolume()
{
    return m_nDisplayVolume;
}

bool MainWindow::getMiniMode()
{
    return m_bMiniMode;
}

MainWindow::~MainWindow()
{
    qInfo() << __func__;
    //Do not enter CloseEvent when exiting from the title bar menu, so add the save function here
    //powered by xxxxp
    if (Settings::get().isSet(Settings::ResumeFromLast)) {
        int nCur = 0;
        nCur = m_pEngine->playlist().current();
        if (nCur >= 0) {
            Settings::get().setInternalOption("playlist_pos", nCur);
        }
    }
    m_pEngine->savePlaybackPosition();
    if (m_pEventListener) {
        this->windowHandle()->removeEventFilter(m_pEventListener);
        delete m_pEventListener;
        m_pEventListener = nullptr;
    }

    if (!utils::check_wayland_env()) {
        disconnect(m_pEngine, 0, 0, 0);
        disconnect(&m_pEngine->playlist(), 0, 0, 0);
    }

    if (m_nLastCookie > 0) {
        utils::UnInhibitStandby(m_nLastCookie);
        qInfo() << "uninhibit cookie" << m_nLastCookie;
        m_nLastCookie = 0;
    }
    if (m_nPowerCookie > 0) {
        utils::UnInhibitPower(m_nPowerCookie);
        m_nPowerCookie = 0;
    }
    delete m_pEngine;
    m_pEngine = nullptr;

    m_diskCheckThread.stop();

    ThreadPool::instance()->quitAll();

#ifdef USE_DXCB
    if (_evm) {
        disconnect(_evm, 0, 0, 0);
        delete _evm;
    }
#endif

    if (m_pShortcutViewProcess) {
        m_pShortcutViewProcess->deleteLater();
        m_pShortcutViewProcess = nullptr;
    }
}
#include "mainwindow.moc"
