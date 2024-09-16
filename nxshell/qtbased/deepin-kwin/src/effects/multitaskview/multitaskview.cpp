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

#include "multitaskview.h"
#include <QtCore>
#include <QMouseEvent>
#include <QtMath>
#include <QAction>
#include <QScreen>
#include <QX11Info>
#include <QTranslator>

#include <effects.h>
#include <kglobalaccel.h>
#include <qdbusconnection.h>
#include <qdbusinterface.h>
#include <qdbusreply.h>
#include <QtConcurrent>
#include <QGSettings/qgsettings.h>
#include <QImageReader>
#include "workspace.h"
//#include "multitouchgesture.h"       //to do

Q_GLOBAL_STATIC_WITH_ARGS(QGSettings, _gsettings_dde_dock, ("com.deepin.dde.dock"))
//Q_GLOBAL_STATIC_WITH_ARGS(QGSettings, _gsettings_dde_dock_primary, ("com.deepin.dde.dock.mainwindow"))
#define GsettingsDockPosition   "position"
//#define GsettingsDockBottom     "bottom"
//#define GsettingsDockPrimary    "only-show-primary"
#define GsettingsDockHeight     "window-size-efficient"
//#define GsettingsDockShow       "hide-mode"

#define BRIGHTNESS  0.4
#define SCALE_F     1.0
#define SCALE_S     2.0
#define WINDOW_W_H  300
#define MOUSE_MOVE_MIN_DISTANCE 2

#define MAX_DESKTOP_COUNT   6

#define FIRST_WIN_SCALE     (float)(720.0 / 1080.0)
#define WORKSPACE_SCALE     (float)(240.0 / 1920.0)
#define WORK_SPACING_SCALE  (float)(40.0 / 1920.0)
#define SPACING_H_SCALE     (float)(20.0 / 1080.0)
#define SPACING_W_SCALE     (float)(20.0 / 1920.0)
#define ADDBTN_W_SCALE      (float)(104.0 / 1920.0)
#define ADDBTN_H_SCALE      (float)(56.0 / 1080.0)
#define ADDBTN_SIZE_SCALE   (float)(64.0 / 1920.0)
#define ADDBTN_RADIUS_SCALE (float)(18.0 / 1920.0)

#define DBUS_APPEARANCE_SERVICE "org.deepin.dde.Appearance1"
#define DBUS_APPEARANCE_OBJ "/org/deepin/dde/Appearance1"
#define DBUS_APPEARANCE_INTF "org.deepin.dde.Appearance1"

#define DBUS_IMAGEEFFECT_SERVICE "org.deepin.dde.ImageBlur1"
#define DBUS_BLUR_OBJ "/org/deepin/dde/ImageBlur1"
#define DBUS_BLUR_INTF "org.deepin.dde.ImageBlur1"

#define MULTITASK_CLOSE_SVG      ":/effects/multitaskview/buttons/multiview_delete.svg"
#define MULTITASK_TOP_SVG        ":/effects/multitaskview/buttons/multiview_top.svg"
#define MULTITASK_TOP_ACTIVE_SVG ":/effects/multitaskview/buttons/multiview_top_active.svg"

#define POPUP_TIME_SCALE 15.0f
#define EFFECT_DURATION_DEFAULT 300
#define SCISSOR_HOFFU 200
#define SCISSOR_HOFFD 400

const char notification_tips[] = "dde-osd dde-osd";
const char screen_recorder[] = "deepin-screen-recorder deepin-screen-recorder";
const char split_outline[] = "kwin_x11 kwin";
const char fallback_background_name[] = "file:///usr/share/wallpapers/deepin/desktop.jpg";
const char previous_default_background_name[] = "file:///usr/share/backgrounds/default_background.jpg";
const char add_workspace_png[] = ":/effects/multitaskview/buttons/add-light.png";//":/resources/themes/add-light.svg";
const char delete_workspace_png[] = ":/effects/multitaskview/buttons/workspace_delete.svg";

static void ensureResources()
{
    // Must initialize resources manually because the effect is a static lib.
    Q_INIT_RESOURCE(multitaskview);
}

namespace KWin
{
QPoint calculateOffSet(const QRect& rect, const QRect &screenArea, const int maxHeight)
{
    QPoint point;
    if (!QX11Info::isPlatformX11()) {
        point.setX(rect.x() - screenArea.x());
        point.setY(screenArea.height() - (rect.y() - screenArea.y()) - rect.height());
        return point;
    }
    point.setX(rect.x());
    point.setY(maxHeight - rect.y() - rect.height());

    return point;
}

CustomThread::CustomThread(BgInfo_st &st)
    : QThread()
    , m_st(st)
{

}

void CustomThread::run()
{
    MultiViewBackgroundManager::instance()->cacheWorkspaceBg(m_st);
}

MultiViewBackgroundManager *MultiViewBackgroundManager::_instance = new MultiViewBackgroundManager();
MultiViewBackgroundManager *MultiViewBackgroundManager::instance()
{
    return _instance;
}

MultiViewBackgroundManager::MultiViewBackgroundManager()
    : QObject()
    , m_bgmutex(QMutex::Recursive)
{
    QStringList lst = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    if (lst.size() > 0) {
        m_deepinwmrcIni = new QSettings(lst[0] + "/deepinwmrc", QSettings::IniFormat);
    }
}

MultiViewBackgroundManager::~MultiViewBackgroundManager()
{
    if (m_deepinwmrcIni) {
        delete m_deepinwmrcIni;
        m_deepinwmrcIni = nullptr;
    }
}

static QString toRealPath(const QString &path)
{
    // QString res = path;
    QString res = QUrl::fromPercentEncoding(path.toUtf8());
    if (res.startsWith("file:///")) {
        res.remove("file://");
    }

    QFileInfo fi(res);
    if (fi.isSymLink()) {
        res = fi.symLinkTarget();
    }
    return res;
}

static QString convertPath(const QString &path)
{
    QString res = QUrl::fromPercentEncoding(path.toUtf8());
    return res;
}

QPixmap MultiViewBackgroundManager::cutBackgroundPix(const QSize &size, const QString &file)
{
    QImageReader imageReader;
    imageReader.setFileName(file);
    imageReader.setAutoTransform(true);
    auto imageSize = imageReader.size();
    auto targetScaleSize = imageSize.scaled(size, Qt::KeepAspectRatioByExpanding);

    imageReader.setScaledSize(targetScaleSize);
    QPixmap pixmap = QPixmap::fromImageReader(&imageReader);

    if(pixmap.width() > size.width() || pixmap.height() > size.height()) {
        pixmap = pixmap.copy(QRect(static_cast<int>((pixmap.width() - size.width()) / 2.0),
                                    static_cast<int>((pixmap.height() - size.height()) / 2.0), size.width(), size.height()));
    }

    return pixmap;
}

QPixmap MultiViewBackgroundManager::getCachePix(const QSize &size, QPair<QSize, QPixmap> &pair)
{
    if (pair.first == size) {
        return pair.second;
    } else if (pair.first.width() > size.width() && pair.first.height() > size.height()) {
        pair.first = size;
        pair.second = pair.second.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        pair.second = pair.second.copy(QRect(static_cast<int>((pair.second.width() - size.width()) / 2.0),
                                       static_cast<int>((pair.second.height() - size.height()) / 2.0), size.width(), size.height()));
        return pair.second;
    }
    return QPixmap();
}

void MultiViewBackgroundManager::getWorkspaceBgPath(BgInfo_st &st, QPixmap &desktopBg, QPixmap &workspaceBg)
{
    QString strBackgroundPath = QString("%1%2").arg(st.desktop).arg(st.screenName);
    QDBusInterface wm(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);
    QDBusReply<QString> getReply = wm.call("GetWorkspaceBackgroundForMonitor", st.desktop, st.screenName);
    QString backgroundUri;
    if(!getReply.value().isEmpty()) {
        backgroundUri = getReply.value();
    } else {
        backgroundUri = QLatin1String(fallback_background_name);
    }

    backgroundUri = toRealPath(backgroundUri);

    if (m_allBackgroundList[st.screenName].size() < st.desktop+1) {
         m_allBackgroundList[st.screenName].resize(st.desktop+1);
    }

    if (m_wpCachedPixmaps.contains(backgroundUri + strBackgroundPath)) {
        auto& p = m_wpCachedPixmaps[backgroundUri + strBackgroundPath];
        workspaceBg = getCachePix(st.workspaceSize, p);
    }
    if (workspaceBg.isNull()) {
        QPixmap pixmap = cutBackgroundPix(st.workspaceSize, backgroundUri);
        m_wpCachedPixmaps[backgroundUri + strBackgroundPath] = qMakePair(st.workspaceSize, pixmap);
        workspaceBg = pixmap;
    }

    m_allBackgroundList[st.screenName][st.desktop] = backgroundUri; // Use unblured background

    QDBusInterface imageBlurInterface(DBUS_IMAGEEFFECT_SERVICE, DBUS_BLUR_OBJ, DBUS_BLUR_INTF, QDBusConnection::systemBus());
    imageBlurInterface.setTimeout(100);
    QDBusReply<QString> blurReply = imageBlurInterface.call("Get", backgroundUri);
    if (!blurReply.value().isEmpty())
        backgroundUri = toRealPath(QString(blurReply.value()));

    m_currentBackgroundList.insert(backgroundUri);
    if (m_bgCachedPixmaps.contains(backgroundUri + strBackgroundPath)) {
        auto& p = m_bgCachedPixmaps[backgroundUri + strBackgroundPath];
        desktopBg = getCachePix(st.desktopSize, p);
    }
    if (desktopBg.isNull()) {
        QPixmap pixmap = cutBackgroundPix(st.desktopSize, backgroundUri);
        m_bgCachedPixmaps[backgroundUri + strBackgroundPath] = qMakePair(st.desktopSize, pixmap);
        desktopBg = pixmap;
    }

}

void MultiViewBackgroundManager::cacheWorkspaceBg(BgInfo_st &st)
{
    QMutexLocker locker(&m_bgmutex);
    if (m_deepinwmrcIni) {
        QString backgroundUri;
        QString strBackgroundPath = QString("%1@%2").arg(st.desktop).arg(st.screenName);
        backgroundUri = m_deepinwmrcIni->value("WorkspaceBackground/" + strBackgroundPath).toString();
        QString strBackgroundPathkey = QString("%1%2").arg(st.desktop).arg(st.screenName);

        if (!backgroundUri.isEmpty()) {
            backgroundUri = toRealPath(backgroundUri);
            QPixmap pixmap = cutBackgroundPix(st.desktopSize, backgroundUri);
            m_bgCachedPixmaps[backgroundUri + strBackgroundPathkey] = qMakePair(st.desktopSize, pixmap);
            pixmap = cutBackgroundPix(st.workspaceSize, backgroundUri);
            m_wpCachedPixmaps[backgroundUri + strBackgroundPathkey] = qMakePair(st.workspaceSize, pixmap);
        }
    }
}

void MultiViewBackgroundManager::clearCurrentBackgroundList()
{
    m_currentBackgroundList.clear();
}

void MultiViewBackgroundManager::getBackgroundList()
{
    m_backgroundAllList.clear();
    QDBusInterface remoteApp(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);
    QDBusReply<QString> reply = remoteApp.call("List", "background");

    QJsonDocument json = QJsonDocument::fromJson(reply.value().toUtf8());
    QJsonArray arr = json.array();
    if (!arr.isEmpty()) {
        auto p = arr.constBegin();
        while (p != arr.constEnd()) {
            auto o = p->toObject();
            if (!o.value("Id").isUndefined() && !o.value("Deletable").toBool()) {
                m_backgroundAllList << o.value("Id").toString();
            }
            ++p;
        }
    }

    for (const QString &value : m_currentBackgroundList) {
        if (m_backgroundAllList.contains(value)) {
            m_backgroundAllList.remove(value);
        }
    }

    QString oldVerisonDefaultBackground(previous_default_background_name);
    QString defaultBackground(fallback_background_name);
    if (m_currentBackgroundList.contains(oldVerisonDefaultBackground)) {
        m_backgroundAllList.remove(defaultBackground);
    }
}

void MultiViewBackgroundManager::updateBackgroundList(const QString &file)
{
    if (!m_backgroundAllList.contains(file)) {
        m_backgroundAllList.insert(file);
    }
    if (m_currentBackgroundList.contains(file)) {
        m_currentBackgroundList.remove(file);
    }
}

QString MultiViewBackgroundManager::getRandBackground()
{
    int index = 3;
    QString file;
    while (index > 0) {
        int backgroundIndex = m_backgroundAllList.count();
        if (backgroundIndex - 1 != 0) {
            qsrand((uint)QTime::currentTime().msec());
            backgroundIndex = qrand() % (backgroundIndex - 1);
        } else {
            backgroundIndex -= 1;
        }

        if (m_backgroundAllList.count() <= backgroundIndex) {
            index --;
            continue;
        }

        auto b_set = m_backgroundAllList.begin();
        file = *(b_set + backgroundIndex);
        if (m_currentBackgroundList.contains(file)) {
            m_backgroundAllList.remove(file);
            index --;
            continue;
        }
        break;
    }

    return file;
}

void MultiViewBackgroundManager::getPreviewBackground(QSize size, QPixmap &workspaceBg, EffectScreen *screen)
{
    m_previewScreen = screen;
    m_previewFile = getRandBackground();
    QString rfile = toRealPath(m_previewFile);
    workspaceBg = cutBackgroundPix(size, rfile);
}

void MultiViewBackgroundManager::setNewBackground(BgInfo_st &st, QPixmap &desktopBg, QPixmap &workspaceBg)
{
    QString strBackgroundPath = QString("%1%2").arg(st.desktop).arg(st.screenName);

    QDBusInterface wm(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);
    QString file;
    if (st.screen == m_previewScreen && !m_previewFile.isEmpty()) {
        m_previewScreen = nullptr;
        if (m_currentBackgroundList.contains(m_previewFile)) {
            m_previewFile = getRandBackground();
        }
        file = m_previewFile;
        m_currentBackgroundList.insert(file);
        if (m_backgroundAllList.count() - 1 > 0)
            m_backgroundAllList.remove(file);
        m_previewFile = "";
    } else {
        file = getRandBackground();
        if (m_backgroundAllList.count() - 1 > 0)
            m_backgroundAllList.remove(file);
        m_currentBackgroundList.insert(file);
    }

    MultiViewBackgroundManager::instance()->m_allBackgroundList[st.screenName].append(file);

    wm.call("SetWorkspaceBackgroundForMonitor", st.desktop, st.screenName, file);

    file = toRealPath(file);

    workspaceBg = cutBackgroundPix(st.workspaceSize, file);
    m_wpCachedPixmaps[file + strBackgroundPath] = qMakePair(st.workspaceSize, workspaceBg);

    QDBusInterface imageBlurInterface(DBUS_IMAGEEFFECT_SERVICE, DBUS_BLUR_OBJ, DBUS_BLUR_INTF, QDBusConnection::systemBus());
    imageBlurInterface.setTimeout(100);
    QDBusReply<QString> blurReply = imageBlurInterface.call("Get", file);
    if (!blurReply.value().isEmpty())
        file = toRealPath(QString(blurReply.value()));
    desktopBg = cutBackgroundPix(st.desktopSize, file);
    m_bgCachedPixmaps[file + strBackgroundPath] = qMakePair(st.desktopSize, desktopBg);
}

void MultiViewBackgroundManager::setMonitorInfo(QList<QMap<QString,QVariant>> monitorInfoList)
{
    m_monitorInfoList = monitorInfoList;

    QList<QString> monitorNameList;
    for (int i = 0; i < m_monitorInfoList.size(); i++) {
        QMap<QString,QVariant> monitorInfo = m_monitorInfoList.at(i);
        monitorNameList.append(monitorInfo.keys());
    }
    m_screenNamelist = monitorNameList.toSet().toList();
}

MultiViewAddButton::MultiViewAddButton()
{
    m_button = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/icon.qml", true);
}

MultiViewAddButton::~MultiViewAddButton()
{

}

void MultiViewAddButton::render()
{
    m_button->setRadius(m_fullArea.width() * ADDBTN_RADIUS_SCALE);
    m_button->render(infiniteRegion(), 1, 0);
}

void MultiViewAddButton::setImage(const QString &btf, const QRect &rect)
{
    m_rect = rect;
    m_button->setImage(QUrl::fromLocalFile(btf));
    m_button->setGeometry(rect);
}

void MultiViewAddButton::setRect(const QRect &rect)
{
    m_rect = rect;
    m_button->setGeometry(rect);
}

MultiViewWorkspace::MultiViewWorkspace(bool flag)
    : m_backGroundFrame(nullptr)
    , m_desktop(0)
{
    m_backGroundFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/icon.qml", true);
    m_backGroundFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_workspaceBgFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/workspacebg.qml", true);
    m_workspaceBgFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_workspaceBgFrame->setRadius(10);
    m_hoverFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/hover.qml", true);
    m_hoverFrame->setRadius(10);
}

MultiViewWorkspace::~MultiViewWorkspace()
{

}

void MultiViewWorkspace::renderDesktopBackGround(float k)
{
    m_backGroundFrame->setPosition(QPoint(m_fullArea.x() + k * m_fullArea.width(), m_fullArea.y()), true);
    m_backGroundFrame->renderPixmap(infiniteRegion(), 0.6);
}

void MultiViewWorkspace::renderWorkspaceBackGround(float t, int desktop)
{
    int ncurrentDesktop = effects->currentDesktop();
    QRect rect = m_workspaceBgFrame->geometry();

    if (ncurrentDesktop == desktop) {
        QColor color = effectsEx->getActiveColor();
        QRect geoframe = rect;
        geoframe.adjust(-3, -3, 3, 3);
        m_hoverFrame->setGeometry(geoframe);
        m_hoverFrame->setColor(color);
        m_hoverFrame->render(infiniteRegion(), 1, 0);
    }

    m_workspaceBgFrame->render(infiniteRegion(), t, 1);
}

void MultiViewWorkspace::setPosition(QPoint pos)
{
    m_workspaceBgFrame->setPosition(pos, true);
}

void MultiViewWorkspace::setImage(const QPixmap &bgPix, const QPixmap &wpPix, const QRect &rect)
{
    m_rect = rect;
    m_currentRect = rect;

    m_backGroundFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_backGroundFrame->setGeometry(m_fullArea);
    m_backGroundFrame->setPosition(m_fullArea.topLeft());
    m_backGroundFrame->setPixmap(bgPix);

    m_workspaceBgFrame->setGeometry(rect);
    m_workspaceBgFrame->setImage(wpPix);
}

void MultiViewWorkspace::setRect(const QRect rect)
{
    m_rect = rect;
    updateGeometry(rect);
}

QRect MultiViewWorkspace::getGeometry()
{
    return m_workspaceBgFrame->geometry();
}

void MultiViewWorkspace::updateGeometry(QRect rect)
{
    m_currentRect = rect;
    m_workspaceBgFrame->setGeometry(rect);
}

MultiViewWinFill::MultiViewWinFill(EffectScreen *screen, QRect rect, float radius, int maxHeight)
    : m_rect(rect)
    , m_screen(screen)
    , m_maxHeight(maxHeight)
{
    m_fillFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/hoverbg.qml", false);
    m_fillFrame->setGeometry(rect);
    m_fillFrame->setColor("#A0A0A0");
    m_fillFrame->setRadius(5);
}

MultiViewWinFill::~MultiViewWinFill()
{

}

void MultiViewWinFill::render()
{
    m_fillFrame->render(infiniteRegion(), 0.5, 0);
}

MultitaskViewEffect::MultitaskViewEffect()
    : m_showActions(new QAction(this))
    , m_showActionw(new QAction(this))
    , m_showActiona(new QAction(this))
    , m_mutex(QMutex::Recursive)
    , m_timer(new QTimer(this))
    , m_timerCheckWindowClose(new QTimer(this))
{
    QAction *s = m_showActions;
    s->setProperty("componentName", QStringLiteral("deepin-kwin"));
    s->setObjectName(QStringLiteral("ShowMultitasking"));
    s->setText("Show Multitasking View");
    KGlobalAccel::self()->setDefaultShortcut(s, QList<QKeySequence>() << Qt::META + Qt::Key_S);
    KGlobalAccel::self()->setShortcut(s, QList<QKeySequence>() << Qt::META + Qt::Key_S);
    shortcut = KGlobalAccel::self()->shortcut(s);

    QAction *w = m_showActionw;
    w->setObjectName(QStringLiteral("ShowMultitaskingW"));
    w->setText("Show Multitasking View");
    KGlobalAccel::self()->setDefaultShortcut(w, QList<QKeySequence>() << Qt::META + Qt::Key_W);
    KGlobalAccel::self()->setShortcut(w, QList<QKeySequence>() << Qt::META + Qt::Key_W);
    shortcutw = KGlobalAccel::self()->shortcut(w);

    QAction *a = m_showActiona;
    a->setObjectName(QStringLiteral("ShowMultitaskingA"));
    a->setText("Show Multitasking View");
    KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << Qt::META + Qt::Key_A);
    KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << Qt::META + Qt::Key_A);
    shortcuta = KGlobalAccel::self()->shortcut(a);

    connect(s, SIGNAL(triggered(bool)), this, SLOT(toggle()));
    connect(w, SIGNAL(triggered(bool)), this, SLOT(toggle()));
    connect(a, SIGNAL(triggered(bool)), this, SLOT(toggle()));

    connect(effects, &EffectsHandler::windowAdded, this, &MultitaskViewEffect::onWindowAdded);
    connect(effects, &EffectsHandler::windowDeleted, this, &MultitaskViewEffect::onWindowDeleted);
    connect(effects, &EffectsHandler::windowClosed, this, &MultitaskViewEffect::onWindowClosed);
    connect(effects, &EffectsHandler::screenAdded, this, [this] { onCloseEffect(true); });
    connect(effects, &EffectsHandler::screenRemoved, this, [this] { onCloseEffect(true); });

    connect(_gsettings_dde_dock, &QGSettings::changed, this, &MultitaskViewEffect::onDockChange);

    connect(m_timerCheckWindowClose, &QTimer::timeout, this,
        [this] () {
            if (this->m_closingWindow) {
                this->setActive(false);
            }
        }
    );

    ensureResources();
    reconfigure(ReconfigureAll);

    m_curDesktopIndex = effects->currentDesktop();
    m_lastDesktopIndex = m_curDesktopIndex;

    m_bgSlidingStatus = false;
    m_bgSlidingTimeLine.setEasingCurve(QEasingCurve::InQuint);
    m_bgSlidingTimeLine.setDuration(std::chrono::milliseconds(EFFECT_DURATION_DEFAULT));

    m_workspaceSlidingStatus = false;
    m_workspaceSlidingTimeline.setEasingCurve(QEasingCurve::OutQuint);
    m_workspaceSlidingTimeline.setDuration(std::chrono::milliseconds(EFFECT_DURATION_DEFAULT));

    m_popStatus = false;
    m_popTimeLine.setEasingCurve(QEasingCurve::OutQuint);
    m_popTimeLine.setDuration(std::chrono::milliseconds(EFFECT_DURATION_DEFAULT));

    m_opacityStatus = false;
    m_opacityTimeLine.setEasingCurve(QEasingCurve::OutQuint);
    m_opacityTimeLine.setDuration(std::chrono::milliseconds(2*EFFECT_DURATION_DEFAULT));

    QString qm = QString(":/effects/multitaskview/translations/multitasking_%1.qm").arg(QLocale::system().name());
    QTranslator *tran = new QTranslator();
    if (tran->load(qm)) {
        qApp->installTranslator(tran);
    }

    connect(m_timer, &QTimer::timeout, this, &MultitaskViewEffect::motionRepeat);

    cacheWorkspaceBackground();

    QDBusConnection::sessionBus().connect(DBUS_DEEPIN_WM_SERVICE, DBUS_DEEPIN_WM_OBJ, DBUS_DEEPIN_WM_INTF,
                                        "ShowWorkspaceChanged", this, SLOT(toggle()));
    QDBusConnection::sessionBus().connect("com.deepin.ScreenRecorder.time", "/com/deepin/ScreenRecorder/time", "com.deepin.ScreenRecorder.time", "start", this, SLOT(screenRecorderStart()));

    m_addingDesktopTimer.setInterval(200);
    m_addingDesktopTimer.setSingleShot(true);
    connect(&m_addingDesktopTimer, &QTimer::timeout, this, &MultitaskViewEffect::addNewDesktop);
}

MultitaskViewEffect::~MultitaskViewEffect()
{
    if (m_showActions) {
        delete m_showActions;
        m_showActions = nullptr;
    }
    if (m_showActionw) {
        delete m_showActionw;
        m_showActionw = nullptr;
    }
    if (m_showActiona) {
        delete m_showActiona;
        m_showActiona = nullptr;
    }
}

void MultitaskViewEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    m_bgSlidingStatus = false;
    m_bgSlidingTimeLine.reset();

    m_popStatus = false;
    m_popTimeLine.reset();

    m_opacityStatus = false;
    m_opacityTimeLine.reset();

    m_curDesktopIndex = effects->currentDesktop();
    m_lastDesktopIndex = m_curDesktopIndex;

    m_scalingFactor = qMax(1.0, QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.0);

    KConfigGroup config_group(KSharedConfig::openConfig("deepin-kwinrc"), "Compositing");
    if (effects->waylandDisplay()) {
        setMotionEffect(config_group.readEntry("MultitaskViewMotionEffect", true));
    } else {
        EffectType effectType = effectsEx->effectType();
        setMotionEffect(effectType == EffectType::OpenGLComplete && config_group.readEntry("MultitaskViewMotionEffect", true)
                && config_group.readEntry("window_animation", true));
    }
}

void MultitaskViewEffect::prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime)
{
    if (isActive()) {
        if(m_effectFlyingBack.animating())
            m_effectFlyingBack.update(presentTime, motionEffectEnable());

        if(m_workspaceSlidingStatus) {
            if (m_workspaceSlidingState == 0) {
                m_workspaceSlidingState = 1;
                m_workspaceSlidingTimeline.reset();
                if (motionEffectEnable()) {
                    m_workspaceSlidingTimeline.setElapsed(std::chrono::milliseconds(30));
                } else {
                    m_workspaceSlidingTimeline.setElapsed(m_workspaceSlidingTimeline.duration());
                }
            } else {
                m_workspaceSlidingTimeline.advance(presentTime);
            }
        }

        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS
                  |  PAINT_SCREEN_BACKGROUND_FIRST;

        if(m_popStatus) {
            if (motionEffectEnable()) {
                m_popTimeLine.advance(presentTime);
            } else {
                m_popTimeLine.setElapsed(m_popTimeLine.duration());
            }
        }

        if(m_opacityStatus) {
            if (motionEffectEnable()) {
                m_opacityTimeLine.advance(presentTime);
            } else {
                m_opacityTimeLine.setElapsed(m_opacityTimeLine.duration());
            }
        }

        if(m_bgSlidingStatus) {
            if (m_bgSlidingState == 0) {
                m_bgSlidingState = 1;
                m_bgSlidingTimeLine.reset();
                if (motionEffectEnable()) {
                    m_bgSlidingTimeLine.setElapsed(std::chrono::milliseconds(30));
                } else {
                    m_bgSlidingTimeLine.setElapsed(m_bgSlidingTimeLine.duration());
                }
            } else {
                m_bgSlidingTimeLine.advance(presentTime);
            }
        }

        if (m_windowEffectState)
            m_windowEffect.update(presentTime, motionEffectEnable());

        for (auto& mm: m_motionManagers) {
            mm->calculate(POPUP_TIME_SCALE, motionEffectEnable());
        }
    }

    for (auto const &w : effects->stackingOrder()) {
        Window* t = static_cast< EffectWindowImpl* >(w)->window();
        if (w->isDock() || w->isNotification() || (t && t->isOverride()))
            w->setData(WindowForceBlurRole, QVariant(true));
    }

    effects->prePaintScreen(data, presentTime);
}

void MultitaskViewEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    m_isShowWin = false;
    effects->paintScreen(mask, region, data);

    QMutexLocker locker(&m_mutex);
    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
        if (m_bgSlidingStatus) {
            MultiViewWorkspace *lwkobj = getWorkspaceObject(iter.key(), m_lastDesktopIndex - 1);
            MultiViewWorkspace *cwkobj = getWorkspaceObject(iter.key(), m_curDesktopIndex - 1);
            if (lwkobj && cwkobj) {
                auto area = lwkobj->getfullArea();
                if (effects->waylandDisplay()) {
                    EffectScreen *effectscreen = effectsEx->findScreen(effectsEx->getCurrentPaintingScreen());
                    if (effectscreen && iter.key() != effectscreen) {
                        if (isExtensionMode()) {
                            continue;
                        } else {
                            if (area.x() > 0) {
                                continue;
                            }
                        }
                    }
                } else {
                    glEnable(GL_SCISSOR_TEST);
                    glScissor(area.x(), 0, area.width(), m_allFullArea.height());
                }

                if (effects->numberOfDesktops() > 2) {
                    if (m_lastDesktopIndex == 1 && m_curDesktopIndex == effects->numberOfDesktops()) {
                        lwkobj->renderDesktopBackGround(-m_bgSlidingTimeLine.value());
                        cwkobj->renderDesktopBackGround(1 - m_bgSlidingTimeLine.value());
                    } else if (m_lastDesktopIndex == effects->numberOfDesktops() && m_curDesktopIndex == 1) {
                        lwkobj->renderDesktopBackGround(m_bgSlidingTimeLine.value());
                        cwkobj->renderDesktopBackGround(-1 + m_bgSlidingTimeLine.value());
                    } else if (m_lastDesktopIndex < m_curDesktopIndex) {
                        lwkobj->renderDesktopBackGround(-m_bgSlidingTimeLine.value());
                        cwkobj->renderDesktopBackGround(1 - m_bgSlidingTimeLine.value());
                    } else {
                        lwkobj->renderDesktopBackGround(m_bgSlidingTimeLine.value());
                        cwkobj->renderDesktopBackGround(-1 + m_bgSlidingTimeLine.value());
                    }
                } else {
                    if (m_lastDesktopIndex < m_curDesktopIndex) {
                        lwkobj->renderDesktopBackGround(-m_bgSlidingTimeLine.value());
                        cwkobj->renderDesktopBackGround(1 - m_bgSlidingTimeLine.value());
                    } else {
                        lwkobj->renderDesktopBackGround(m_bgSlidingTimeLine.value());
                        cwkobj->renderDesktopBackGround(-1 + m_bgSlidingTimeLine.value());
                    }
                }

                if (nullptr == effects->waylandDisplay()) {
                    glDisable(GL_SCISSOR_TEST);
                }
            }
        } else {
            MultiViewWorkspace *cwkobj = getWorkspaceObject(iter.key(), effects->currentDesktop() - 1);
            if (cwkobj) {
                cwkobj->renderDesktopBackGround(0.0f);
            }
        }

        MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
        if (wmobj && wmobj->getDesktopWinNumofScreen(iter.key()) == 0) {
            if (m_tipFrames.find(iter.key()) != m_tipFrames.end() && m_tipFrames[iter.key()].size() == 2) {
                std::unique_ptr<EffectFrameEx> &bgframe = m_tipFrames[iter.key()][0];
                std::unique_ptr<EffectFrameEx> &tframe = m_tipFrames[iter.key()][1];
                QRect bgrect = tframe->geometry();
                QFontMetrics metrics(tframe->font());
                int width = metrics.width(tframe->text());
                int height = metrics.height();
                bgrect.adjust(-(width - bgrect.width()) / 2 - (20 * m_scalingFactor),
                              -(height - bgrect.height()) / 2 - (10 * m_scalingFactor),
                              (width - bgrect.width()) / 2 + (20 * m_scalingFactor),
                              (height - bgrect.height()) / 2 + (11 * m_scalingFactor));

                bgframe->setGeometry(bgrect);
                bgframe->render(infiniteRegion(), 0.8, 0);
                tframe->render(infiniteRegion(), 1, 0);
            }
        }
    }

    //draw workspace background
    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
        for (int j = 0; j < effects->numberOfDesktops(); j++) {
            if (m_deleteWorkspaceDesktop - 1 == j)
                continue;
            MultiViewWorkspace *wkobj = getWorkspaceObject(iter.key(), j);
            if (wkobj) {
                if (m_popStatus) {  //startup
                    wkobj->renderWorkspaceBackGround(m_popTimeLine.value(), j + 1);
                } else if (m_opacityStatus) {  //new workspace
                    if (j == effects->numberOfDesktops() - 1)
                        wkobj->renderWorkspaceBackGround(m_opacityTimeLine.value(), j + 1);
                    else
                        wkobj->renderWorkspaceBackGround(1, j + 1);
                } else if (m_wasWorkspaceMove) {  //move workspace
                    if (m_aciveMoveDesktop - 1 != j) {
                        wkobj->renderWorkspaceBackGround(1, j + 1);
                    }
                } else if (m_workspaceSlidingStatus) {
                    renderSlidingWorkspace(wkobj, iter.key(), j, data);
                } else {
                    wkobj->renderWorkspaceBackGround(1, j + 1);
                }
            }
        }
    }

    if (!m_wasWindowMove && m_isShowPreview && m_screen != nullptr && !m_workspaceSlidingStatus) {
        QRect rect = m_addWorkspaceButton[m_screen]->getRect();
        showWorkspacePreview(m_screen, rect);
    } else {
        showWorkspacePreview(m_screen, QRect(), true);
    }

    //draw button
    if (effects->numberOfDesktops() < MAX_DESKTOP_COUNT) {
        for (auto iter = m_addWorkspaceButton.begin(); iter != m_addWorkspaceButton.end(); iter++) {
            m_addWorkspaceButton[iter.key()]->render();
        }
    }

    for (int desktop = effects->numberOfDesktops(); desktop >= 0; desktop--) {
        m_isShowWin = true;
        paintingDesktop = desktop;
        if (desktop == 0) {
            if (m_hoverDesktop != -1 && m_screen != nullptr && effects->numberOfDesktops() > 1) {
                renderWorkspaceHover(m_screen);
            } else {
                m_workspaceCloseBtnArea = QRect();
            }
        }
        effects->paintScreen(mask, region, data);
    }

    if (m_windowMove && m_wasWindowMove) {
        renderWindowMove(data);
    }

    if (m_wasWorkspaceMove) {
        renderWorkspaceMove(data);
    }
}

void MultitaskViewEffect::postPaintScreen()
{
    if (m_bgSlidingTimeLine.done()) {
        m_bgSlidingStatus = false;
        m_bgSlidingTimeLine.reset();
        m_bgSlidingState = 0;
        handlerAfterTimeLine();
    }

    if (m_workspaceSlidingTimeline.done()) {
        m_workspaceSlidingStatus = false;
        m_workspaceSlidingState = 0;
        m_workspaceSlidingTimeline.reset();
    }

    if (m_popTimeLine.done()) {
        m_popStatus = false;
        m_popTimeLine.reset();
    }

    if (m_opacityTimeLine.done()) {
        m_opacityStatus = false;
        m_opacityTimeLine.reset();
    }

    if (m_windowEffect.getStatus() == 2) {
        m_windowEffectState = false;
    }

    if (m_activated)
        effects->addRepaintFull();

    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant());
    }
    effects->postPaintScreen();

    if (m_effectFlyingBack.done()) {
        m_effectFlyingBack.end();
        setActive(false);
        QTimer::singleShot(400, [&]() { m_delayDbus = true; });
        if (QX11Info::isPlatformX11() && m_dockRect.contains(m_cursorPos)) {
            relayDockEvent(m_cursorPos, m_buttonType);
            m_cursorPos.setX(-1);
            m_cursorPos.setY(-1);
            m_buttonType = 0;
        } else if (!QX11Info::isPlatformX11() && m_sendButton != Qt::NoButton) {
            effectsEx->sendPointer(m_cursorPos, m_sendButton);
            m_cursorPos.setX(-1);
            m_cursorPos.setY(-1);
            m_sendButton = Qt::NoButton;
        }
    }
}

void MultitaskViewEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime)
{
    data.mask |= PAINT_WINDOW_TRANSFORMED;

    if (m_bgSlidingStatus) {
        WindowMotionManager *mgr0 = nullptr, *mgr1 = nullptr;
        auto *lastWinManager = getWinManagerObject(m_lastDesktopIndex - 1);
        auto *curWinManager = getWinManagerObject(m_curDesktopIndex - 1);
        if (lastWinManager && curWinManager) {
            lastWinManager->getMotion(m_lastDesktopIndex, w->screen(), mgr0);
            curWinManager->getMotion(m_curDesktopIndex, w->screen(), mgr1);
        }
    }

    effects->prePaintWindow(w, data, presentTime);
}

void MultitaskViewEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (!m_isShowWin) {
        return;
    }

    if (!isActive()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    if (m_effectFlyingBack.animating()) {
        if (w->isWaterMark() || w->windowClass() == notification_tips) {
            effects->paintWindow(w, mask, region, data);
            return;
        }
        if (w->isOnDesktop(effects->currentDesktop()) && !w->isMinimized()) {
            m_effectFlyingBack.paintWindow(w, mask, region, data);
        }
        return;
    } else if (m_effectFlyingBack.done()) {
        if (w->isOnDesktop(effects->currentDesktop()) && !w->isMinimized()) {
            effects->paintWindow(w, mask, region, data);
        }
        return;
    }

    if (w->windowClass() == screen_recorder || w == m_screenRecorderMenu) {
        if (!m_screenRecorderMenu || w == m_screenRecorderMenu) {
            effects->setElevatedWindow(w, true);
        }
        effects->paintWindow(w, mask, region, data);
        return;
    }
    if (w->windowClass() == notification_tips && m_isCloseScreenRecorder) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    int desktop = effects->currentDesktop();
    if (0 == paintingDesktop) {
        if (w == m_windowMove && m_wasWindowMove) {
            return;
        }

        if (w->isWaterMark()) {
            effects->paintWindow(w, mask, region, data);
            return;
        }

        if (m_windowEffectState && m_flyingWinList.contains(w)) {
            m_windowEffect.paintWindow(w, mask, region, data);
            return;
        }

        if (m_bgSlidingStatus) {
            if (m_flyingWinList.contains(w))
                return;

            WindowMotionManager *mgr0 = nullptr, *mgr1 = nullptr;
            auto *lastWinManager = getWinManagerObject(m_lastDesktopIndex - 1);
            auto *curWinManager = getWinManagerObject(m_curDesktopIndex - 1);
            QRect backgroundRect = effects->clientArea(FullScreenArea, w->screen(), 1).toRect();
            int bgRectWidth = backgroundRect.width();
            if (lastWinManager) {
                lastWinManager->getMotion(m_lastDesktopIndex, w->screen(), mgr0);
                if (mgr0 && mgr0->isManaging(w)) {
                    auto area = effects->clientArea(ScreenArea, w->screen(), 0).toRect();
                    WindowPaintData d = data;
                    auto geo = mgr0->targetGeometry(w);
                    d.setScale(QVector2D((float)geo.width() / w->width(), (float)geo.height() / w->height()));

                    if (effects->numberOfDesktops() > 2) {
                        if (m_lastDesktopIndex == 1 && m_curDesktopIndex == effects->numberOfDesktops()) {
                            d.translate(geo.x() - bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else if (m_lastDesktopIndex == effects->numberOfDesktops() && m_curDesktopIndex == 1) {
                            d.translate(geo.x() + bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else if (m_lastDesktopIndex < m_curDesktopIndex) {
                            d.translate(geo.x() - bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else {
                            d.translate(geo.x() + bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        }
                    } else {
                        if (m_lastDesktopIndex < m_curDesktopIndex) {
                            d.translate(geo.x() - bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else {
                            d.translate(geo.x() + bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        }
                    }
                    if (mgr0->isWindowFill(w) && lastWinManager->isHaveWinFill(w)) {
                        // mask |= PAINT_WINDOW_LANCZOS;
                    }
                    effects->paintWindow(w, mask, area, d);     //when open, all windows flying into RegionB
                }
            }
            if (curWinManager) {
                curWinManager->getMotion(m_curDesktopIndex, w->screen(), mgr1);
                if (mgr1->isManaging(w) && !(m_windowEffectState && m_flyingWinList.contains(w))) {
                    auto area = effects->clientArea(ScreenArea, w->screen(), 0).toRect();
                    WindowPaintData d = data;
                    auto geo0 = mgr1->transformedGeometry(w);
                    auto geo = mgr1->targetGeometry(w);
                    d.setScale(QVector2D((float)geo.width() / w->width(), (float)geo.height() / w->height()));

                    if (effects->numberOfDesktops() > 2) {
                        if (m_lastDesktopIndex == effects->numberOfDesktops() && m_curDesktopIndex == 1) {
                            d.translate(geo.x() - bgRectWidth + bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else if (m_lastDesktopIndex == 1 && m_curDesktopIndex == effects->numberOfDesktops()) {
                            d.translate(geo.x() + bgRectWidth - bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else if(m_lastDesktopIndex<m_curDesktopIndex) {
                            d.translate(geo.x() + bgRectWidth - bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        } else {
                            d.translate(geo.x() - bgRectWidth + bgRectWidth * m_bgSlidingTimeLine.value() - w->x(), geo.y() - w->y(), 0);
                        }
                    } else {
                        if (m_lastDesktopIndex < m_curDesktopIndex) {
                            d.translate(geo.x() + bgRectWidth * (1 - m_bgSlidingTimeLine.value()) - w->x(), geo.y() - w->y(), 0);
                        } else {
                            d.translate(geo.x() + bgRectWidth * (m_bgSlidingTimeLine.value() -1) - w->x(), geo.y() - w->y(), 0);
                        }
                    }

                    if (mgr1->isWindowFill(w) && curWinManager->isHaveWinFill(w)) {
                        // mask |= PAINT_WINDOW_LANCZOS;/
                    }
                    effects->paintWindow(w, mask, area, d);
                }
            }
        } else {
            WindowMotionManager *wmm = nullptr;
            QMutexLocker locker(&m_mutex);
            MultiViewWinManager *wmobj = getWinManagerObject(desktop - 1);
            if (wmobj && wmobj->getMotion(desktop, w->screen(), wmm)) {
                if (wmm->isManaging(w)) {
                    auto area = effects->clientArea(ScreenArea, w->screen(), 0).toRect();
                    WindowPaintData d = data;
                    auto geo = wmm->transformedGeometry(w);

                    if (wmm->isWindowFill(w) && wmobj->isHaveWinFill(w)) {
                        wmobj->getWinFill(w)->render();
                        // mask |= PAINT_WINDOW_LANCZOS;
                    }

                    {
                        d.setScale(QVector2D((float)geo.width() / w->width(), (float)geo.height() / w->height()));
                        float dx = 0.0f, dy = 0.0f;
                        auto gt = wmm->targetGeometry(w);
                        float dx0 = gt.x() - w->x();
                        float dy0 = gt.y() - w->y();
                        float dx1 = geo.x() - w->x();
                        float dy1 = geo.y() - w->y();
                        if(dx0>0) {
                            if (dx1 > dx0) dx = dx0;
                            else dx = dx1;
                        } else {
                            if (dx1 < dx0) dx = dx0;
                            else dx = dx1;
                        }
                        if(dy0>0) {
                            if (dy1 > dy0) dy = dy0;
                            else dy = dy1;
                        } else {
                            if (dy1 < dy0) dy = dy0;
                            else dy = dy1;
                        }
                        d += QPoint(qRound(dx), qRound(dy));
                    }
                    effects->paintWindow(w, mask, area, d);

                    if (w == m_hoverWin) {
                        float radius = wmm->getWindowRadius(w);
                        if (wmm->isWindowFill(w) && wmobj->isHaveWinFill(w)) {
                            renderHover(w, wmobj->getWinFill(w)->getRect(), 0, radius);
                        } else {
                            renderHover(w, geo.toRect(), 0, radius);
                        }
                    }

                    if (w == m_hoverWinBtn) {
                        if (wmm->isWindowFill(w) && wmobj->isHaveWinFill(w)) {
                            renderHover(w, wmobj->getWinFill(w)->getRect(), 1);
                        } else {
                            renderHover(w, geo.toRect(), 1);
                        }
                    }
                }
            }
        }

        if (w->isDock()) {
            m_dockRect = w->frameGeometry().toRect();
            effects->paintWindow(w, mask, region, data);
            return;
        }
    } else if (!m_workspaceSlidingStatus) {
        if (m_wasWorkspaceMove && paintingDesktop == m_aciveMoveDesktop)
            return;

        WindowMotionManager *wmm;
        MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(paintingDesktop - 1);
        if (wkmobj && wkmobj->getMotion(paintingDesktop, w->screen(), wmm)) {
            if (wmm->isManaging(w) && !w->isMinimized()) {
                if (workspace()->showingDesktop() && w->desktop() == m_curDesktopIndex)
                    return;

                WindowPaintData d = data;
                auto geo = wmm->transformedGeometry(w).toRect();

                d += QPoint(qRound(geo.x() - w->x()), qRound(geo.y() - w->y()));
                d.setScale(QVector2D((float)geo.width() / w->width(), (float)geo.height() / w->height()));
                geo &= m_workspaceBackgrounds[w->screen()][paintingDesktop - 1]->getGeometry();
                effects->paintWindow(w, mask, geo, d);
            }
        }
    }
}

void MultitaskViewEffect::handlerAfterTimeLine()
{
    if (m_isRemoveWorkspace) {
        cleanDesktopData(m_lastDesktopIndex);
    }
}

void MultitaskViewEffect::renderWindowMove(KWin::ScreenPaintData &data)
{
    QPoint diff = cursorPos() - m_windowMoveStartPos;
    QRect geo = m_windowMoveGeometry.translated(diff);
    WindowPaintData d(data.projectionMatrix());

    MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
    QRect rect1 = wmobj ? wmobj->getWindowGeometry(m_windowMove, m_windowMove->screen()) : QRect();
    float K1 = (qreal)rect1.width() / (qreal)m_windowMove->width();
    d += QPoint(-m_windowMove->x(), -m_windowMove->y());
    if (cursorPos().y() < m_windowMoveStartPos.y()) {
        int Y0 = m_scale[m_windowMove->screen()].workspaceMgrRect.bottom();
        float k = (cursorPos().y() - m_windowMoveStartPos.y()) / float(Y0 - m_windowMoveStartPos.y());
        float K0 = (qreal)geo.width() / (qreal)m_windowMove->width();
        k = K1 - k * (K1 - K0);
        if (k < K0) k = K0;
        d *= QVector2D(k, k);

        float sx = (m_windowMoveStartPos.x() - rect1.x()) / float(rect1.width());
        float sy = (m_windowMoveStartPos.y() - rect1.y()) / float(rect1.height());
        float cx = cursorPos().x();
        float cy = cursorPos().y();
        float cw = k * m_windowMove->width();
        float ch = k * m_windowMove->height();
        float x0 = cx - cw * sx;
        float y0 = cy - ch * sy;
        d += QPoint(x0,y0);
    }
    else {
        d *= QVector2D(K1, K1);
        d += QPoint(rect1.left()+diff.x(), rect1.top()+diff.y());
    }
    effects->drawWindow(m_windowMove, PAINT_WINDOW_TRANSFORMED, infiniteRegion(), d);
}

void MultitaskViewEffect::renderWorkspaceMove(KWin::ScreenPaintData &data)
{
    MultiViewWorkspace *target = getWorkspaceObject(m_screen, m_aciveMoveDesktop - 1);
    if (target) {
        QPoint cursorpos = cursorPos();
        QPoint diff = cursorpos - m_workspaceMoveStartPos;
        if (diff.y() > 1) {
            diff.setY(0);
        }
        if (m_dragWorkspacedirection == dragNone) {
            if (abs(diff.y()) > abs(diff.x())) {
                m_dragWorkspacedirection = dragUpDown;
            } else if (abs(diff.y()) < abs(diff.x())) {
                m_dragWorkspacedirection = dragLeftRight;
            }
        }

        if (m_dragWorkspacedirection == dragUpDown) {
            diff.setX(0);
        } else if (m_dragWorkspacedirection == dragLeftRight) {
            diff.setY(0);
        }

        if (diff.y() < -(target->getRect().height() / 2)) {
            m_workspaceStatus = wpDelete;
            if (effects->numberOfDesktops() != 1)
                renderDragWorkspacePrompt(m_screen);
        } else if (calculateSwitchPos(diff)) {
            m_workspaceStatus = wpSwitch;
        } else {
            m_workspaceStatus = wpRestore;
        }

        QRect wkgeo = target->getRect().translated(diff);
        target->setPosition(wkgeo.topLeft());
        float transparent = 1.0;
        if (m_workspaceStatus == wpDelete)
            transparent = 0.5;
        target->renderWorkspaceBackGround(transparent, m_aciveMoveDesktop);

        WindowMotionManager *wmm;
        EffectWindowList list;
        MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(m_aciveMoveDesktop - 1);
        if (wkmobj && wkmobj->getMotion(m_aciveMoveDesktop, m_screen, wmm)) {
            list = wmm->orderManagedWindows();
        }

        for (EffectWindow *w : list) {
            if (w->screen() == m_screen && wmm->isManaging(w) && !w->isMinimized()) {
                if (workspace()->showingDesktop() && w->desktop() == m_curDesktopIndex)
                    return;

                WindowPaintData d(data.projectionMatrix());
                auto geo = wmm->transformedGeometry(w);
                geo = geo.translated(diff);
                d.setOpacity(transparent);

                d *= QVector2D((qreal)geo.width() / (qreal)w->width(), (qreal)geo.height() / (qreal)w->height());
                d += QPoint(geo.left() - w->x(), geo.top() - w->y());

                effects->drawWindow(w, PAINT_WINDOW_TRANSFORMED, wkgeo, d);
            }
        }
    }
}

void MultitaskViewEffect::renderSlidingWorkspace(MultiViewWorkspace *wkobj, EffectScreen *screen, int desktop, KWin::ScreenPaintData &data)
{
    float x0 = m_workspaceSlidingInfo[wkobj].first;
    float x1 = m_workspaceSlidingInfo[wkobj].second;
    float t = m_workspaceSlidingTimeline.value();
    float x  = (x1 - x0) * t + x0;
    QRect rect = wkobj->getRect();
    rect.moveTo(int(x), rect.y());
    wkobj->setRect(rect);
    wkobj->renderWorkspaceBackGround(1, desktop + 1);

    WindowMotionManager *wmm;
    MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(desktop);
    if (wkmobj && wkmobj->getMotion(m_aciveMoveDesktop, screen, wmm)) {
        for (EffectWindow *w : wmm->orderManagedWindows()) {
            if (wmm->isManaging(w) && !w->isMinimized()) {
                WindowPaintData d(data.projectionMatrix());
                auto geo = wmm->transformedGeometry(w);
                geo.moveTo(x + geo.x() - x1, geo.y());

                d *= QVector2D((qreal)geo.width() / (qreal)w->width(), (qreal)geo.height() / (qreal)w->height());
                d += QPoint(geo.left() - w->x(), geo.top() - w->y());

                effects->drawWindow(w, PAINT_WINDOW_TRANSFORMED, rect, d);
            }
        }
    }
}

void convertFromGLImage(QImage &img, int w, int h)
{
    // from QtOpenGL/qgl.cpp
    // Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
    // see http://qt.gitorious.org/qt/qt/blobs/master/src/opengl/qgl.cpp
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        // OpenGL gives RGBA; Qt wants ARGB
        uint *p = (uint*)img.bits();
        uint *end = p + w * h;
        while (p < end) {
            uint a = *p << 24;
            *p = (*p >> 8) | a;
            p++;
        }
    } else {
        // OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
        for (int y = 0; y < h; y++) {
            uint *q = (uint*)img.scanLine(y);
            for (int x = 0; x < w; ++x) {
                const uint pixel = *q;
                *q = ((pixel << 16) & 0xff0000) | ((pixel >> 16) & 0xff)
                     | (pixel & 0xff00ff00);

                q++;
            }
        }

    }
    img = img.mirrored();
}

QColor pixmapMainColor(QPixmap p, double bright) //p为目标图片 bright为亮度系数，为1则表示保持不变
{
    int step=20; //步长：在图片中取点时两点之间的间隔，若为1,则取所有点，适当将此值调大有利于提高运行速度
    int t=0; //点数：记录一共取了多少个点，用于做计算平均数的分母
    QImage image=p.toImage(); //将Pixmap类型转为QImage类型
    int r=0,g=0,b=0; //三元色的值，分别用于记录各个点的rgb值的和
    for (int i=0;i<p.width();i+=step) {
        for (int j=0;j<p.height();j+=step) {
            if(image.valid(i,j)){ //判断该点是否有效
                t++; //点数加一
                QColor c=image.pixel(i,j); //获取该点的颜色
                r+=c.red(); //将获取到的各个颜色值分别累加到rgb三个变量中
                b+=c.blue();
                g+=c.green();
            }
        }
    }
        return QColor(int(bright*r/t)>255?255:int(bright*r/t),
                  int(bright*g/t)>255?255:int(bright*g/t),
                  int(bright*b/t)>255?255:int(bright*b/t)); //最后返回的值是亮度系数×平均数,若超出255则设置为255也就是最大值，防止乘与亮度系数后导致某些值大于255的情况。
}

void MultitaskViewEffect::renderHover(const EffectWindow *w, const QRect &rect, int order, float radius)
{
    if (!order) {
        QColor color = effectsEx->getActiveColor();
        QRect geoframe = rect;
        geoframe.adjust(-5, -5, 5, 5);
        m_hoverWinFrame->setGeometry(geoframe);
        m_hoverWinFrame->setColor(color);
        m_hoverWinFrame->setRadius(radius);
        m_hoverWinFrame->render(infiniteRegion(), 1, 0);
    } else {
        int width = 0;
        int height = 0;
        {
            QFont font;
            font.setPointSize(14);
            m_textWinFrame->setFont(font);
            QFontMetrics metrics(m_textWinFrame->font());
            QString string = metrics.elidedText(w->caption(), Qt::ElideRight, rect.width() * 0.9);
            if (string != m_textWinFrame->text())
                m_textWinFrame->setText(string);
            width = metrics.width(string);
            height = metrics.height();
        }

        if (w->keepAbove()) {
            m_topWinFrame->setImage(QUrl::fromLocalFile(MULTITASK_TOP_ACTIVE_SVG));
        } else {
            m_topWinFrame->setImage(QUrl::fromLocalFile(MULTITASK_TOP_SVG));
        }

        // button point 微调结果
        m_closeWinFrame->setPosition(QPoint(rect.x() + rect.width() - 12 * effectsEx->getOsScale(), rect.y() - 10 * effectsEx->getOsScale()));
        m_topWinFrame->setPosition(QPoint(rect.x() - 12 * effectsEx->getOsScale(), rect.y() - 10 * effectsEx->getOsScale()));
        m_textWinFrame->setPosition(QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height() - 40));
        if (m_winBtnArea.size() < 2)
            m_winBtnArea.resize(2);
        m_winBtnArea[0] = QRect(QPoint(rect.x() + rect.width() - 25, rect.y() - 17), QSize(48 * effectsEx->getOsScale(), 48 * effectsEx->getOsScale()));
        m_winBtnArea[1] = QRect(QPoint(rect.x() - 22, rect.y() - 17), QSize(48 * effectsEx->getOsScale(), 48 * effectsEx->getOsScale()));

        QRect geoframe = m_textWinFrame->geometry();
        geoframe.adjust(-(width - geoframe.width()) / 2 - (16 * m_scalingFactor),
                        -(height - geoframe.height()) / 2 - ((48 * m_scalingFactor - height) / 2),
                        (width - geoframe.width()) / 2 + (16 * m_scalingFactor),
                        (height - geoframe.height()) / 2 + (48 * m_scalingFactor - height) / 2);
        m_textWinBgFrame->setGeometry(geoframe);

        m_closeWinFrame->render(infiniteRegion(), 1, 0);
        m_topWinFrame->render(infiniteRegion(), 1, 0);
        m_textWinBgFrame->render(infiniteRegion(), 0.8, 0);
        m_textWinFrame->render(infiniteRegion(), 1, 0);
    }
}

void MultitaskViewEffect::renderWorkspaceHover(EffectScreen *screen)
{
    MultiViewWorkspace *wkobj = getWorkspaceObject(screen, m_hoverDesktop - 1);
    if (wkobj) {
        QRect rect = wkobj->getRect();
        m_workspaceCloseBtnArea = QRect(QPoint(rect.x() + rect.width() - 30, rect.y() - 13), QSize(48, 48));
        m_closeWorkspaceFrame->setPosition(QPoint(rect.x() + rect.width() - 14 * effectsEx->getOsScale(), rect.y() - 9 * effectsEx->getOsScale()));    //point 微调结果
        m_closeWorkspaceFrame->render(infiniteRegion(), 1, 0);
    }
}

void MultitaskViewEffect::renderDragWorkspacePrompt(EffectScreen *screen)
{
    MultiViewWorkspace *wkobj = getWorkspaceObject(screen, m_aciveMoveDesktop - 1);
    if (wkobj) {
        QRect rect = wkobj->getRect();

        QFont font;
        font.setFamily(m_fontFamily);
        font.setPointSize(13);
        QFontMetrics metrics(font);
        int width = m_dragTipsFrameShadow->iconSize().width() + metrics.horizontalAdvance(tr("Drag upwards to remove")) + 13;

        m_dragTipsFrameShadow->setFont(font);
        m_dragTipsFrameShadow->setGeometry(QRect(rect.x() + (rect.width() - width) / 2, rect.y() + (rect.height() / 2), width, 28));
        m_dragTipsFrame->setFont(font);
        m_dragTipsFrame->setGeometry(QRect(rect.x() + (rect.width() - width) / 2, rect.y() + (rect.height() / 2), width, 28));

        m_dragTipsFrameShadow->render(infiniteRegion(), 1, 0);
        m_dragTipsFrame->render(infiniteRegion(), 1, 0);

        drawDottedLine(rect, screen);
    }
}

void MultitaskViewEffect::drawDottedLine(const QRect &geo, EffectScreen *screen)
{
    static std::unique_ptr<GLShader> shader = ShaderManager::instance()->generateShaderFromFile(
            ShaderTrait::UniformColor,
            QStringLiteral(":/effects/multitaskview/shaders/dottedline.vert"),
            QStringLiteral(":/effects/multitaskview/shaders/dottedline.frag"));
    GLVertexBuffer* vbo = GLVertexBuffer::streamingBuffer();
    auto area = effects->clientArea(FullArea, screen, 0);
    vbo->reset();
    vbo->setColor(QColor(255, 255, 255));
    vbo->setUseColor(true);
    QVector<float> verts;
    verts.reserve(44);
    float x1 = (float)(geo.left()) * 2 / area.width() - 1.0;
    float x2 = (float)(geo.right()) * 2 / area.width() - 1.0;
    float y1 = 1.0 - (float)(geo.y() + geo.height()) / 2 * 2 / area.height();

    for (int i = 0; i < 22; i++) {
        verts << x1 + (x2 - x1) / 21.0f * i << y1;
    }
    ShaderBinder bind(shader.get());
    vbo->setData(verts.size() / 2, 2, verts.data(), NULL);
    vbo->render(GL_LINES);
}

void MultitaskViewEffect::onWindowClosed(EffectWindow *w)
{
    if (!m_activated)
        return;

    if (w->windowClass() == screen_recorder && w != m_screenRecorderMenu) {
        effects->startMouseInterception(this, Qt::PointingHandCursor);
        if (QX11Info::isPlatformX11()) {
            m_hasKeyboardGrab = effects->grabKeyboard(this);
        }
    } else if (w->isDock()) {
        m_dock = nullptr;
        m_dockRect.setSize(QSize(0, 0));
    } else if (!QX11Info::isPlatformX11() && w->windowClass() == notification_tips) {
        auto it = std::find(m_notificationList.begin(), m_notificationList.end(), w);
        if (it != m_notificationList.end())
            m_notificationList.erase(it);
    }

    m_closingWindow = false;
}

void MultitaskViewEffect::onWindowDeleted(EffectWindow *w)
{
    if (!m_activated)
        return;

    if (w->isDock()) {
        m_dock = nullptr;
        m_dockRect.setSize(QSize(0, 0));
    } else if (!QX11Info::isPlatformX11() && w->windowClass() == notification_tips) {
        auto it = std::find(m_notificationList.begin(), m_notificationList.end(), w);
        if (it != m_notificationList.end())
            m_notificationList.erase(it);
        if (m_notificationList.empty())
            m_isCloseScreenRecorder = false;
        return;
    } else if (w->windowClass() == screen_recorder && w != m_screenRecorderMenu) {
        m_isScreenRecorder = false;
        if (!QX11Info::isPlatformX11())
            m_isCloseScreenRecorder = true;
        return;
    } else if (w == m_screenRecorderMenu) {
        m_screenRecorderMenu = nullptr;
        return;
    } else {
        removeWinAndRelayout(w);
        m_isShieldEvent = false;
        m_closingWindow = false;
    }
}

void MultitaskViewEffect::onWindowAdded(EffectWindow *w)
{
    if (!m_activated)
        return;

    w->refVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE | EffectWindow::PAINT_DISABLED_BY_DESKTOP);

    if (w->isDock()) {
        m_dockRect = w->frameGeometry().toRect();
        m_dock = w;
        onDockChange("");
    } else if (!QX11Info::isPlatformX11() && w->windowClass() == notification_tips) {
        m_notificationList.push_back(w);
    } else if (!QX11Info::isPlatformX11() && w->caption() == "org.deepin.dde.lock") {
        setActive(false);
    } else if (QX11Info::isPlatformX11()) {
        if (w->windowClass() != screen_recorder
            && w->windowClass() != notification_tips
            && w->windowClass() != split_outline
            && !w->isUtility()) {
            m_effectFlyingBack.begin();
            effects->addRepaintFull();
        } else if (w->windowClass() == screen_recorder) {
            {
                m_wasWorkspaceMove = false;
                if (MultiViewWorkspace *wt = getWorkspaceObject(m_screen, m_aciveMoveDesktop - 1))
                    wt->updateGeometry(wt->getRect());
                m_aciveMoveDesktop = -1;
                m_moveWorkspaceNum = -1;
                m_moveWorkspacedirection = mvNone;
                m_dragWorkspacedirection = dragNone;
                m_workspaceStatus = wpNone;
                m_windowMove = nullptr;
                m_wasWindowMove = false;
            }
            {
                if (m_hasKeyboardGrab)
                    effects->ungrabKeyboard();
                m_hasKeyboardGrab = false;
            }
            if (m_isScreenRecorder) {
                m_screenRecorderMenu = w;
            } else {
                effects->stopMouseInterception(this);
                m_isScreenRecorder = true;
            }
        }
    } else if (!QX11Info::isPlatformX11()) {
        if (m_isScreenRecorder) {
            m_screenRecorderMenu = w;
        } else if (w->windowClass() == screen_recorder) {
            {
                m_wasWorkspaceMove = false;
                if (MultiViewWorkspace *wt = getWorkspaceObject(m_screen, m_aciveMoveDesktop - 1))
                    wt->updateGeometry(wt->getRect());
                m_aciveMoveDesktop = -1;
                m_moveWorkspaceNum = -1;
                m_moveWorkspacedirection = mvNone;
                m_dragWorkspacedirection = dragNone;
                m_workspaceStatus = wpNone;
                m_windowMove = nullptr;
                m_wasWindowMove = false;
            }
            effects->stopMouseInterception(this);
            m_isScreenRecorder = true;
        } else if (w->windowClass() != notification_tips && w->caption() != "deepin-splitoutline") {
            setActive(false);
        }
    }
}

void MultitaskViewEffect::onCloseEffect(bool isSleepBefore)
{
    if (isSleepBefore && isActive()) {
        setActive(false);
    }
}

void MultitaskViewEffect::onDockChange(const QString &key)
{
    QString position = _gsettings_dde_dock->get(GsettingsDockPosition).toString();
    int height = _gsettings_dde_dock->get(GsettingsDockHeight).toInt();
    if (position == "bottom") {
        if (m_dockRect.height() < height) {
            m_dockRect.setY(m_dockRect.height() + m_dockRect.y() - height);
            m_dockRect.setHeight(height);
        }
    } else if (position == "top") {
        if (m_dockRect.height() < height) {
            m_dockRect.setHeight(height);
        }
    } else if (position == "left") {
        if (m_dockRect.width() < height) {
            m_dockRect.setWidth(height);
        }
    } else if (position == "right") {
        if (m_dockRect.width() < height) {
            m_dockRect.setX(m_dockRect.width() + m_dockRect.x() - height);
            m_dockRect.setWidth(height);
        }
    }
}

void MultitaskViewEffect::screenRecorderStart()
{
    if (m_isScreenRecorder /*&& !QX11Info::isPlatformX11()*/) {
        effects->startMouseInterception(this, Qt::PointingHandCursor);
    }
}

void MultitaskViewEffect::fontChanged(const QString &fontType, const QString &fontName)
{
    if (fontType == "standardfont" || fontType == "monospacefont") {
        m_fontFamily = fontName;
    }
}

void MultitaskViewEffect::handlerWheelEvent(Qt::MouseButtons btn)
{
    static bool is_smooth_scrolling = false;
    if (is_smooth_scrolling)
        return;
    if (btn == Qt::ForwardButton) {
        is_smooth_scrolling = true;
        if (effects->currentDesktop() + 1 <= effects->numberOfDesktops()) {
            changeCurrentDesktop(effects->currentDesktop() + 1);
        } else {
            changeCurrentDesktop(1);
        }
    } else if (btn == Qt::BackButton) {
        is_smooth_scrolling = true;
        if (effects->currentDesktop() - 1 >= 1) {
            changeCurrentDesktop(effects->currentDesktop() - 1);
        } else {
            changeCurrentDesktop(effects->numberOfDesktops());
        }
    }
    QTimer::singleShot(400, [&]() { is_smooth_scrolling = false; });
}

void MultitaskViewEffect::windowInputMouseEvent(QEvent* e)
{
    if (!isReceiveEvent()) {
        return;
    }

    switch (e->type()) {
    case QEvent::MouseMove:
        if (m_touch.active && !m_touch.isMotion) {
            return;
        }
        break;
    case QEvent::MouseButtonPress:
        if(m_touch.active && m_touch.isPress) {
            return;
        }
    case QEvent::MouseButtonRelease:
        if(m_touch.active && m_touch.isPress) {
            return;
        }
    case QEvent::Wheel:
        break;
    default:
        return;
    }

    if (e->type() == QEvent::Wheel) {
        auto wheelEvent = static_cast<QWheelEvent*>(e);
        if (wheelEvent->angleDelta().y() < 0) {
            handlerWheelEvent(QX11Info::isPlatformX11() ? Qt::ForwardButton : Qt::BackButton);
        } else if (wheelEvent->angleDelta().y() > 0) {
            handlerWheelEvent(QX11Info::isPlatformX11() ? Qt::BackButton : Qt::ForwardButton);
        }
        return;
    }

    auto mouseEvent = static_cast<QMouseEvent*>(e);
    EffectWindow* target = nullptr;
    bool isAddWorkspace = false;
    bool isHoverWorkspace = false;
    int desktop = -1;
    EffectScreen *screen = nullptr;
    if (!m_wasWorkspaceMove) {
        for (auto effectScreen : effects->screens()) {
            if (m_scale[effectScreen].workspaceMgrRect.contains(mouseEvent->pos())) {
                isHoverWorkspace = true;
                screen = effectScreen;
                m_screen = effectScreen;
                if (effects->numberOfDesktops() < MAX_DESKTOP_COUNT) {
                    if (m_addWorkspaceButton[effectScreen]->getRect().contains(mouseEvent->pos())) {
                        isAddWorkspace = true;
                    }
                }
                break;
            } else if (m_scale[effectScreen].windowMgrRect.contains(mouseEvent->pos())) {
                MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
                if (wmobj) {
                    target = wmobj->getHoverWin(mouseEvent->pos(), effectScreen);
                }
                break;
            }
        }
    }

    auto hoverOnNotification = [this] (const QPoint pos)->bool {
        for (EffectWindow *notification : m_notificationList) {
            if (notification->geometry().contains(pos))
                return true;
        }
        return false;
    };

    QPoint diff = mouseEvent->pos() - m_lastWorkspaceMovePos;
    switch (mouseEvent->type()) {
    case QEvent::MouseMove:
    {
        if (!QX11Info::isPlatformX11() && hoverOnNotification(mouseEvent->pos())) {
            m_hoverWinBtn = nullptr;
            m_hoverDesktop = -1;
            return;
        }

        if (!m_dockRect.contains(mouseEvent->pos())) {
            if (target) {   // window hover
                m_hoverWin = target;
                m_hoverWinBtn = target;
                m_hoverDesktop = -1;
            } else if (!m_wasWorkspaceMove && (abs(diff.x()) > MOUSE_MOVE_MIN_DISTANCE || (abs(diff.y()) > MOUSE_MOVE_MIN_DISTANCE)) && m_aciveMoveDesktop != -1 && m_screen != nullptr) {     //workspace move
                m_workspaceMoveStartPos = mouseEvent->pos();
                m_wasWorkspaceMove = true;
                m_hoverDesktop = -1;
            } else if (isHoverWorkspace) {  // workspace hover
                if (!checkHandlerWorkspace(mouseEvent->pos(), m_screen, m_hoverDesktop))
                    m_hoverDesktop = -1;
                m_hoverWinBtn = nullptr;
            } else {
                m_hoverWinBtn = nullptr;
                m_hoverDesktop = -1;
            }
        } else {
            m_hoverWinBtn = nullptr;
            m_hoverDesktop = -1;
        }

        if (m_windowMove && (abs(diff.x()) > MOUSE_MOVE_MIN_DISTANCE || (abs(diff.y()) > MOUSE_MOVE_MIN_DISTANCE))) {    // window move
            if (!m_wasWindowMove) {
                m_windowMoveStartPos = mouseEvent->pos();
                MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
                m_windowMoveGeometry = wmobj ? wmobj->getWindowGeometry(m_windowMove, m_windowMove->screen()) : QRect();

                int width = m_scale[m_windowMove->screen()].workspaceWidth / 2;
                float scale = (float)width / m_windowMove->width();
                int height = m_windowMove->height() * scale;
                m_windowMoveGeometry.setSize(QSize(width, height));
            }
            m_wasWindowMove = true;
        }

        if (abs(diff.x()) > MOUSE_MOVE_MIN_DISTANCE || abs(diff.y()) > MOUSE_MOVE_MIN_DISTANCE) {
            m_isShowPreview = isAddWorkspace;
        } else {
            m_isShowPreview = false;
        }

        return;
    }
    case QEvent::MouseButtonPress:
        m_isShowPreview = isAddWorkspace;
        m_timer->setSingleShot(true);
        m_timer->start(250);
        if (!m_dockRect.contains(mouseEvent->pos())) {
            if (target) {       // window press
                m_windowMove = target;
                m_windowMoveDiff = mouseEvent->pos() - target->pos().toPoint();
            } else if (isHoverWorkspace && !isAddWorkspace) {   // workspace press
                if (checkHandlerWorkspace(mouseEvent->pos(), screen, m_aciveMoveDesktop)) {
                    m_screen = screen;
                }
            }
            m_lastWorkspaceMovePos = mouseEvent->pos();
        }

        break;
    case QEvent::MouseButtonRelease:
        m_timer->stop();
        if (!m_wasWindowMove && !m_wasWorkspaceMove && m_dockRect.contains(mouseEvent->pos())) {
            m_delayDbus = false;
            m_effectFlyingBack.begin();
            effects->addRepaintFull();
            m_cursorPos = mouseEvent->pos();
            if (QX11Info::isPlatformX11()) {
                m_buttonType = mouseEvent->button();
            } else {
                m_sendButton = mouseEvent->button();
            }
        } else if (!m_wasWindowMove && !QX11Info::isPlatformX11() && hoverOnNotification(mouseEvent->pos())) {
            m_effectFlyingBack.begin();
            effects->addRepaintFull();
            // for pointer could click view button on notification after screen recorder
            m_cursorPos = mouseEvent->pos();
            m_sendButton = mouseEvent->button();
        } else if (target) {
            bool isPressBtn = false;
            int btn = 0;
            for (; btn < m_winBtnArea.size(); ++btn) {
                if (m_winBtnArea[btn].contains(mouseEvent->pos())) {
                    isPressBtn = true;
                    break;
                }
            }
            if (isPressBtn) {
                if (btn == 0) {   // close btn
                    if (closeWindow(target)) {
                        m_hoverWin = nullptr;
                        m_winBtnArea.clear();
                        m_isShieldEvent = true;
                    }
                } else if (btn == 1) {    //top btn
                    setWinKeepAbove(target);
                }
            } else if (!m_wasWindowMove && !m_longPressTouch) {
                effects->defineCursor(Qt::PointingHandCursor);
                effects->setElevatedWindow(target, false);
                effects->activateWindow(target);

                m_effectFlyingBack.setSelecedWindow(target);
                m_effectFlyingBack.begin();
            }
        } else if (m_wasWindowMove && isHoverWorkspace) {
            checkHandlerWorkspace(mouseEvent->pos(), screen, desktop);
            if (desktop != -1 && !m_windowMove->isOnDesktop(desktop)) {
                moveWindowChangeDesktop(m_windowMove, desktop, screen);
            } else {
                MultiViewWorkspace *wkobj = getWorkspaceObject(screen, effects->numberOfDesktops() - 1);
                if (wkobj) {
                    QRect bgrect = QRect(wkobj->getRect().x() + wkobj->getRect().width(), m_scale[screen].workspaceMgrRect.y(),
                                         m_scale[screen].workspaceMgrRect.x() + m_scale[screen].workspaceMgrRect.width() - wkobj->getRect().x() - wkobj->getRect().width(), m_scale[screen].workspaceMgrRect.height());
                    if (bgrect.contains(mouseEvent->pos()) && effects->numberOfDesktops() < 6) {
                        addNewDesktop();
                        moveWindowChangeDesktop(m_windowMove, effects->numberOfDesktops(), screen, true);
                        m_opacityStatus = true;  //start opacity and sliding effects
                    }
                }
            }
        } else if (isHoverWorkspace && !m_wasWorkspaceMove && !m_wasWindowMove) {
            if (isAddWorkspace) {
                addNewDesktop();
            } else if (m_workspaceCloseBtnArea.contains(mouseEvent->pos())) {
                removeDesktop(m_aciveMoveDesktop);
            } else {
                checkHandlerWorkspace(mouseEvent->pos(), screen, desktop);

                if (desktop == effects->currentDesktop() || desktop == -1) {
                    m_effectFlyingBack.begin();
                    effects->addRepaintFull();
                } else {
                    changeCurrentDesktop(desktop);
                }
            }
        } else if (m_workspaceStatus == wpSwitch) {
            switchDesktop();
        } else if (m_workspaceStatus == wpDelete) {
            removeDesktop(m_aciveMoveDesktop);
        } else if (m_workspaceStatus == wpRestore) {
            MultiViewWorkspace *wt = getWorkspaceObject(m_screen, m_aciveMoveDesktop - 1);
            if (wt)
                wt->updateGeometry(wt->getRect());
        } else if (!m_wasWorkspaceMove && !m_wasWindowMove) {
            m_effectFlyingBack.begin();
            effects->addRepaintFull();
        }

        m_workspaceStatus = wpNone;
        m_hoverDesktop = -1;
        m_aciveMoveDesktop = -1;
        m_screen = nullptr;
        m_wasWorkspaceMove = false;
        m_moveWorkspaceNum = -1;
        m_moveWorkspacedirection = mvNone;
        m_dragWorkspacedirection = dragNone;
        m_windowMove = nullptr;
        m_wasWindowMove = false;
        m_longPressTouch = false;

        break;
    default:
        return;
    }
}

void MultitaskViewEffect::grabbedKeyboardEvent(QKeyEvent* e)
{
    if (!isReceiveEvent()) {
        return;
    }

    if (e->type() == QEvent::KeyPress) {
        if (shortcut.contains(e->key() + e->modifiers()) ||
            shortcuta.contains(e->key() + e->modifiers()) ||
            shortcutw.contains(e->key() + e->modifiers())) {
            toggle();
            return;
        }
        switch (e->key()) {
        case Qt::Key_Escape:
            m_effectFlyingBack.begin();
            effects->addRepaintFull();
            break;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
            if (e->modifiers() == Qt::MetaModifier ||
                    e->modifiers() == (Qt::MetaModifier|Qt::KeypadModifier)) {
                int index = e->key() - Qt::Key_0;
                if (effects->numberOfDesktops() >= index) {
                    if (effects->currentDesktop() != index) {
                        changeCurrentDesktop(index);
                    } else {
                        m_effectFlyingBack.begin();
                        effects->addRepaintFull();
                    }
                }
            }
            break;
        case Qt::Key_Exclam:
        case Qt::Key_At: // shift+2
        case Qt::Key_NumberSign: // shift+3
        case Qt::Key_Dollar:
        case Qt::Key_Percent:
        case Qt::Key_AsciiCircum:
            if (e->modifiers() == (Qt::ShiftModifier | Qt::MetaModifier)) {
                int target_desktop = 1;
                switch(e->key()) {
                case Qt::Key_Exclam:      target_desktop = 1; break;
                case Qt::Key_At:          target_desktop = 2; break;
                case Qt::Key_NumberSign:  target_desktop = 3; break;
                case Qt::Key_Dollar:      target_desktop = 4; break;
                case Qt::Key_Percent:     target_desktop = 5; break;
                case Qt::Key_AsciiCircum: target_desktop = 6; break;
                default: break;
                }
                if (m_hoverWin && !(effects->numberOfDesktops() < target_desktop) && effects->currentDesktop() != target_desktop) {
                    moveWindowChangeDesktop(m_hoverWin, target_desktop, m_hoverWin->screen());
                    m_hoverWin = nullptr;
                }
            }
            break;
        case Qt::Key_Home:
            if (e->modifiers() == Qt::NoModifier) {
                m_hoverWin = getHomeOrEndWindow(true);
            }
            break;
        case Qt::Key_End:
            if (e->modifiers() == (Qt::ShiftModifier | Qt::MetaModifier | Qt::KeypadModifier)) {
                if (m_hoverWin && effects->currentDesktop() != 1) {
                    moveWindowChangeDesktop(m_hoverWin, 1, m_hoverWin->screen());
                    m_hoverWin = nullptr;
                }
            } else if (e->modifiers() == Qt::NoModifier) {
                m_hoverWin = getHomeOrEndWindow(false);
            }
            break;
        case Qt::Key_PageDown:
        case Qt::Key_Clear:
            if (e->modifiers() == (Qt::ShiftModifier | Qt::MetaModifier | Qt::KeypadModifier)) {
                int target_desktop = 3;
                switch (e->key())
                {
                case Qt::Key_PageDown:  target_desktop = 3; break;
                case Qt::Key_Clear:     target_desktop = 5; break;
                default: break;
                }
                if (m_hoverWin && !(effects->numberOfDesktops() < target_desktop) && effects->currentDesktop() != target_desktop) {
                    moveWindowChangeDesktop(m_hoverWin, target_desktop, m_hoverWin->screen());
                    m_hoverWin = nullptr;
                }
            }
            break;
        case Qt::Key_Down:
            if (e->modifiers() == (Qt::ShiftModifier | Qt::MetaModifier | Qt::KeypadModifier)) {
                if (m_hoverWin && !(effects->numberOfDesktops() < 2) && effects->currentDesktop() != 2) {
                    moveWindowChangeDesktop(m_hoverWin, 2, m_hoverWin->screen());
                    m_hoverWin = nullptr;
                }
            } else if (e->modifiers() == Qt::NoModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getNextWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_Up:
            if (e->modifiers() == Qt::NoModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getPreWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_Equal:
            if (e->modifiers() == Qt::AltModifier) {
                if(!m_addingDesktopTimer.isActive())
                    m_addingDesktopTimer.start();
            }
            break;
        case Qt::Key_Plus:
            if (e->modifiers() == (Qt::AltModifier|Qt::KeypadModifier)) {
                if(!m_addingDesktopTimer.isActive())
                    m_addingDesktopTimer.start();
            }
            break;
        case Qt::Key_Minus:
            if (e->modifiers() == Qt::AltModifier ||
                e->modifiers() == (Qt::AltModifier|Qt::KeypadModifier)) {
                removeDesktop(effects->currentDesktop());
            }
            break;
        case Qt::Key_Tab:
            if (e->modifiers() == Qt::NoModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getNextWindow(m_hoverWin);
                } else {
                    m_hoverWin = getHomeOrEndWindow(true);
                }
            }
            break;
        case Qt::Key_Backtab:
            if (e->modifiers() == Qt::ShiftModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getPreWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_Right:
            if (e->modifiers() == Qt::MetaModifier) {
                int index = effects->currentDesktop() + 1;
                if (index > effects->numberOfDesktops()) {
                    changeCurrentDesktop(1);
                } else {
                    changeCurrentDesktop(index);
                }
            } else if (e->modifiers() == (Qt::ShiftModifier | Qt::MetaModifier | Qt::KeypadModifier)) {
                if (m_hoverWin && !(effects->numberOfDesktops() < 6) && effects->currentDesktop() != 6) {
                    moveWindowChangeDesktop(m_hoverWin, 6, m_hoverWin->screen());
                    m_hoverWin = nullptr;
                }
            } else if (e->modifiers() == Qt::NoModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getNextWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_Left:
            if (e->modifiers() == Qt::MetaModifier) {
                int index = effects->currentDesktop() - 1;
                if (index <= 0) {
                    changeCurrentDesktop(effects->numberOfDesktops());
                } else {
                    changeCurrentDesktop(index);
                }
            } else if (e->modifiers() == (Qt::ShiftModifier | Qt::MetaModifier | Qt::KeypadModifier)) {
                if (m_hoverWin) {
                    moveWindowChangeDesktop(m_hoverWin, 4, m_hoverWin->screen());
                }
            } else if (e->modifiers() == Qt::NoModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getPreWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_Return:
            if (m_hoverWin) {
                effects->defineCursor(Qt::PointingHandCursor);
                effects->setElevatedWindow(m_hoverWin, false);
                effects->activateWindow(m_hoverWin);

                m_effectFlyingBack.setSelecedWindow(m_hoverWin);
                m_effectFlyingBack.begin();
            }
            break;
        case Qt::Key_Delete:
            if (e->modifiers() == Qt::NoModifier) {
                if (m_hoverWin && closeWindow(m_hoverWin)) {
                    m_hoverWin = nullptr;
                    m_isShieldEvent = true;
                }
            }
            break;
        case Qt::Key_L:
            if (e->modifiers() == Qt::MetaModifier) {
                setActive(false);
                QString sessionPath = workspace()->getProcessSessionPath();
                if (!sessionPath.isEmpty()) {
                    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_LOGIN_SERVICE, sessionPath,
                            DBUS_SESSION_INTF, "Lock");
                    QDBusConnection::systemBus().asyncCall(message);
                }
            }
            break;
        case Qt::Key_QuoteLeft:
            if (e->modifiers() == Qt::AltModifier) {
                if (m_hoverWin) {
                    m_hoverWin = getNextSameTypeWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_AsciiTilde:
            if (e->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) {
                if (m_hoverWin) {
                    m_hoverWin = getPreSameTypeWindow(m_hoverWin);
                }
            }
            break;
        case Qt::Key_Space:
            if (e->modifiers() == (Qt::AltModifier)) {
                break;
            }
        default:
            if (!QX11Info::isPlatformX11() && effectsEx->isShortcuts(e)) {
                QTimer::singleShot(20, [&, this]() {
                    effects->ungrabKeyboard();
                    m_hasKeyboardGrab = false;
                    });
            }
            break;
        }
    }
}

void MultitaskViewEffect::relayDockEvent(QPoint pos, int button)
{
    if (m_dock) {
        auto cl = static_cast<EffectWindowImpl *>(m_dock)->window();
        xcb_button_release_event_t c;
        memset(&c, 0, sizeof(c));

        c.response_type = XCB_BUTTON_PRESS;
        c.event = cl->window();
        c.event_x = pos.x() - m_dockRect.x();
        c.event_y = pos.y() - m_dockRect.y();
        c.detail = (button == 1 ? 1 : 3);  // 1 左键 3 右键
        xcb_send_event(connection(), false, c.event, XCB_EVENT_MASK_BUTTON_PRESS, reinterpret_cast<const char*>(&c));
        xcb_flush(connection());

        memset(&c, 0, sizeof(c));
        c.response_type = XCB_BUTTON_RELEASE;
        c.event = cl->window();
        c.event_x = pos.x() - m_dockRect.x();
        c.event_y = pos.y() - m_dockRect.y();
        c.detail = (button == 1 ? 1 : 3);
        xcb_send_event(connection(), false, c.event, XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char*>(&c));
        xcb_flush(connection());
    } // button release
}

bool MultitaskViewEffect::isActive() const
{
    return m_activated && !effects->isScreenLocked();
}

void MultitaskViewEffect::toggle()
{
    if (!QX11Info::isPlatformX11()) { // MultitaskView cannot be opened without login in Wayland mode
        if(qgetenv("DISPLAY").isEmpty()) {
            return;
        }
    }

    if (m_activated) {
        m_effectFlyingBack.begin();
    } else if (m_delayDbus) {
        if (!QX11Info::isPlatformX11()) {
            QTimer::singleShot(50, [&, this]() {
                this->setActive(!this->m_activated);
            });
        } else {
            setActive(!m_activated);
        }
    }
}

void MultitaskViewEffect::cleanup()
{
    if (m_activated) {
        return;
    }

    m_isScreenRecorder = false;
    m_isCloseScreenRecorder = false;
    m_screenRecorderMenu = nullptr;

    if (m_hasKeyboardGrab)
        effects->ungrabKeyboard();
    m_hasKeyboardGrab = false;
    effects->stopMouseInterception(this);
    effects->setActiveFullScreenEffect(0);

    while (m_motionManagers.size() > 0) {
        m_motionManagers.first()->clearMotion();
        delete m_motionManagers.first();
        m_motionManagers.removeFirst();
    }

    while (m_workspaceWinMgr.size() > 0) {
        m_workspaceWinMgr.first()->clearMotion();
        delete m_workspaceWinMgr.first();
        m_workspaceWinMgr.removeFirst();
    }

    for (auto iter = m_addWorkspaceButton.begin(); iter != m_addWorkspaceButton.end(); iter++) {
        if (iter.value()) {
            delete m_addWorkspaceButton[iter.key()];
            m_addWorkspaceButton[iter.key()] = nullptr;
        }
    }

    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
        QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
        for (int j = 0; j < list.size(); j++) {
            delete list[j];
            list[j] = nullptr;
        }
    }

    m_maxHeight = 0;
    m_windowMove = nullptr;
    m_hoverWin = nullptr;
    m_hoverWinBtn = nullptr;

    m_hoverWinFrame = nullptr;
    m_closeWinFrame = nullptr;
    m_topWinFrame = nullptr;
    m_textWinFrame = nullptr;
    m_textWinBgFrame = nullptr;
    m_previewFrame = nullptr;
    m_closeWorkspaceFrame = nullptr;
    m_dragTipsFrame = nullptr;
    m_dragTipsFrameShadow = nullptr;

    m_motionManagers.clear();
    m_workspaceWinMgr.clear();
    m_addWorkspaceButton.clear();
    m_workspaceBackgrounds.clear();
    m_tipFrames.clear();
    m_flyingWinList.clear();
    m_notificationList.clear();
    MultiViewBackgroundManager::instance()->clearCurrentBackgroundList();

    for (EffectWindow *w : effects->stackingOrder()) {
        w->unrefVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE | EffectWindow::PAINT_DISABLED_BY_DESKTOP);
    }
}

void MultitaskViewEffect::setActive(bool active)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this)
        return;

    if (m_activated == active)
        return;

    m_activated = active;

    QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "MultitaskStateChanged");
    message << bool(m_activated);
    QDBusConnection::sessionBus().send(message);

    if(!active) {
        m_popStatus = false;
        m_popTimeLine.reset();

        m_opacityStatus = false;
        m_opacityTimeLine.reset();

        m_bgSlidingStatus = false;
        m_bgSlidingTimeLine.reset();

        m_effectFlyingBack.reset();
        m_windowEffectState = false;
    } else {
        m_popStatus = true;   //start popup TimeLine
        m_popTimeLine.reset();

        m_opacityStatus = false;
        m_opacityTimeLine.reset();

        m_bgSlidingStatus = false;
        m_bgSlidingTimeLine.reset();

        m_effectFlyingBack.reset();

        m_isShieldEvent = false;
        m_windowEffectState = false;
    }


    cleanup();
    if (active) {
        initWorkspaceBackground();
        createEffectFrame();
        effects->startMouseInterception(this, Qt::PointingHandCursor);
        m_hasKeyboardGrab = effects->grabKeyboard(this);
        effects->setActiveFullScreenEffect(this);

        m_radius = effectsEx->getOsRadius();

        EffectWindowList windowsList = effects->stackingOrder();
        int ncurrentDesktop = effects->currentDesktop();

        m_backgroundRect = effects->clientArea(MaximizeFullArea, effects->findScreen(0), ncurrentDesktop).toRect();
        m_curDesktopIndex = ncurrentDesktop;

        for (int i = 1; i <= effects->numberOfDesktops(); i++) {        //number of desktops
            setWinLayout(i, windowsList);
        }

        for (EffectWindow *w : windowsList) {
            w->refVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE | EffectWindow::PAINT_DISABLED_BY_DESKTOP);
        }

        effects->addRepaintFull();
    } else {
        for (auto &p: m_motionManagers) {
            p->resetWindow();
        }
    }
}

void MultitaskViewEffect::setWinLayout(int desktop, const EffectWindowList &windows)
{
    MultiViewWinManager *wmofDesktop = new MultiViewWinManager();
    MultiViewWinManager *wmofWorkspace = new MultiViewWinManager();

    wmofDesktop->setdesktop(desktop);
    m_motionManagers.append(wmofDesktop);

    MultiViewWinManager *wmobj = getWinManagerObject(desktop - 1);

    for (auto effectScreen : effects->screens()) {           //number of screens
        WindowMotionManager wmm;
        WindowMotionManager workspacewmm;
        EffectWindowList winList;
        for (const auto& w: windows) {
            if (w->isOnDesktop(desktop) && isRelevantWithPresentWindows(w) && w->screen() == effectScreen && checkConfig(w)) {
                wmm.manage(w);
                workspacewmm.manage(w);
                winList.push_back(w);
            } else if (w->isDock()) {
                m_dockRect = w->frameGeometry().toRect();
                m_dock = w;
            }
        }

        calculateWindowTransformations(winList, wmm, desktop, effectScreen);

        wmobj->setWinManager(effectScreen, wmm);

        calculateWorkSpaceWinTransformations(workspacewmm.managedWindows(), workspacewmm, desktop);
        wmofWorkspace->setWinManager(effectScreen, workspacewmm);
    }

    wmofWorkspace->setdesktop(desktop);
    m_workspaceWinMgr.append(wmofWorkspace);
}

void MultitaskViewEffect::updateWorkspaceWinLayout(int numDesktops)
{
    int num = 0;
    if (m_workspaceWinMgr.size() >= numDesktops) {
        num = numDesktops;
    } else {
        return;
    }

    for (int i = 0; i < num; i++) {
        for (auto effectScreen : effects->screens()) {
            WindowMotionManager *wmm;
            if (!m_workspaceWinMgr[i]->getMotion(i + 1, effectScreen, wmm)) {
                continue;
            }
            calculateWorkSpaceWinTransformations(wmm->managedWindows(), *wmm, i + 1);
        }
    }
}

void MultitaskViewEffect::updateWorkspaceWinLayout(int numDesktops, int desktop)
{
    int num = 0;
    if (m_workspaceWinMgr.size() >= numDesktops) {
        num = numDesktops;
    } else {
        return;
    }

    for (int i = 0; i < num; i++) {
        for (auto effectScreen : effects->screens()) {
            WindowMotionManager *wmm;
            if (!m_workspaceWinMgr[i]->getMotion(i + 1, effectScreen, wmm)) {
                continue;
            }
            precalculateWorkSpaceWinTransformations(wmm->managedWindows(), *wmm, desktop <= i + 1 ? i + 2 : i + 1);
        }
    }
}

void MultitaskViewEffect::workspaceWinRelayout(int desktop, EffectScreen *screen)
{
    WindowMotionManager *wmm;
    MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(desktop - 1);
    if (wkmobj && wkmobj->getMotion(desktop, screen, wmm)) {
        calculateWorkSpaceWinTransformations(wmm->managedWindows(), *wmm, desktop);
    }
}

bool MultitaskViewEffect::isRelevantWithPresentWindows(EffectWindow *w) const
{
    if (w->isSpecialWindow() || w->isUtility()) {
        return false;
    }

    if (w->isPopupWindow()) {
        return false;
    }

    if (w->isDock()) {
        return false;
    }

    if (w->isSkipSwitcher()) {
        return false;
    }

    if (w->isDeleted()) {
        return false;
    }

    if (!w->acceptsFocus()) {
        return false;
    }

    if (!w->isCurrentTab()) {
        return false;
    }

    if (!w->isOnCurrentActivity()) {
        return false;
    }
    if (!QX11Info::isPlatformX11()) {
        auto cl = static_cast<EffectWindowImpl *>(w)->window();
        if (w->windowClass() == notification_tips || w->windowClass() == screen_recorder || cl->isStandAlone()) {
            return false;
        }
    }

    return true;
}

bool MultitaskViewEffect::isReceiveEvent()
{
    if (!m_activated) {
        return false;
    }

    if (m_effectFlyingBack.getStatus() != -1) {
        return false;
    }

    if (m_isShieldEvent) {
        return false;
    }

    return true;
}

void MultitaskViewEffect::precalculateWorkSpaceWinTransformations(EffectWindowList windows, WindowMotionManager &wm, int desktop)
{
    if (0 == windows.size() || 0 == m_workspaceBackgrounds.size()) {
        return;
    }

    for (int i = 0; i < windows.size(); i++) {
        EffectWindow *w = windows[i];
        MultiViewWorkspace *wkobj = getWorkspaceObject(w->screen(), desktop - 1);
        if (!wkobj)
            continue;
        QPoint pos((w->x() - wkobj->getfullArea().x()) * WORKSPACE_SCALE + wkobj->getRect().x(), (w->y() - wkobj->getfullArea().y()) * WORKSPACE_SCALE + wkobj->getRect().y());
        QSize size(w->width() * WORKSPACE_SCALE, w->height() * WORKSPACE_SCALE);
        QRect rect(pos, size);
        wm.setTransformedGeometry(w, rect);
    }
}

void MultitaskViewEffect::calculateWorkSpaceWinTransformations(EffectWindowList windows, WindowMotionManager &wm, int desktop)
{
    if (0 == windows.size() || 0 == m_workspaceBackgrounds.size()) {
        return;
    }

    for (int i = 0; i < windows.size(); i++) {
        EffectWindow *w = windows[i];
        MultiViewWorkspace *wkobj = getWorkspaceObject(w->screen(), m_deleteWorkspaceDesktop == 1 ? m_deleteWorkspaceDesktop : desktop - 1);
        if (!wkobj)
            continue;
        QPoint pos((w->x() - wkobj->getfullArea().x()) * WORKSPACE_SCALE + wkobj->getRect().x(), (w->y() - wkobj->getfullArea().y()) * WORKSPACE_SCALE + wkobj->getRect().y());
        QSize size(w->width() * WORKSPACE_SCALE, w->height() * WORKSPACE_SCALE);
        QRect rect(pos, size);
        wm.setTransformedGeometry(w, rect);
    }
}

void MultitaskViewEffect::calculateWindowTransformations(EffectWindowList windows, WindowMotionManager& wmm, int desktop, EffectScreen *screen, bool isReLayout)
{
    if (windows.size() == 0)
        return;
    calculateWindowTransformationsClosest(windows, desktop, screen, wmm, isReLayout);
}

void MultitaskViewEffect::calculateWindowTransformationsClosest(EffectWindowList windowlist, int desktop, EffectScreen *screen,
        WindowMotionManager& motionManager, bool isReLayout)
{
    QHash<EffectWindow*, QRect> targets;

    for (EffectWindow *w : windowlist) {
        QRect rect = w->frameGeometry().toRect();
        targets[w] = rect;
    }

    QRect screenRect = effects->clientArea(MaximizeFullArea, screen, 1).toRect();
    QRect desktopRect = effects->clientArea(MaximizeArea, screen, 1).toRect();
    float scaleHeight = screenRect.height() * FIRST_WIN_SCALE;
    int minSpacingH = m_scale[screen].spacingHeight;
    int totalw = m_scale[screen].spacingWidth;
    QRect clientRect = desktopRect;
    clientRect.setY(clientRect.y() + m_scale[screen].workspaceMgrHeight);

    QList<int> centerList;
    int row = 1;
    int index = 1;
    int xpos = 0;
    float radius = 0.0;
    bool overlap;
    do {
        overlap = false;
        for (int i = windowlist.size() - 1; i >= 0; i--) {
            EffectWindow *w = windowlist[i];
            if (!motionManager.isManaging(w))
                continue;
            QRect *target = &targets[w];
            float width = target->width();
            if (target->height() > scaleHeight) {
                float scale = (float)(scaleHeight / target->height());
                width = target->width() * scale;
            }
            totalw += width;
            totalw += m_scale[screen].spacingWidth;
            if (width + m_scale[screen].spacingWidth * 2 > clientRect.width())
                break;

            if (totalw > clientRect.width()) {
                index ++;
                if (index > row)
                    break;
                xpos = ((clientRect.width() - totalw + width + m_scale[screen].spacingWidth) / 2) + m_scale[screen].spacingWidth + clientRect.x();
                centerList.push_back(xpos);
                totalw = m_scale[screen].spacingWidth;
                totalw += width;
                totalw += m_scale[screen].spacingWidth;
            }
        }
        xpos = ((clientRect.width() - totalw) / 2) + m_scale[screen].spacingWidth + clientRect.x();
        centerList.push_back(xpos);

        if (totalw > clientRect.width()) {
            centerList.clear();
            overlap = true;
            scaleHeight -= 15;
            float critical = (float)(clientRect.height() - (row + 2) * minSpacingH) / (float)(row + 1);
            if (scaleHeight <= critical) {
                row++;
            }
            index = 1;
            totalw = m_scale[screen].spacingWidth;
        }
    } while (overlap);  //calculation layout row

    {
        if (m_radius > 0) {
            radius = row <= 2 ? 18.0 : 10.0;
        }
    }

    float winYPos = (clientRect.height() - (index - 1) * minSpacingH - index * scaleHeight) / 2 + clientRect.y();
    row = 1;
    int x = centerList[row - 1];
    totalw = m_scale[screen].spacingWidth;
    for (int i = windowlist.size() - 1; i >= 0; i--) {
        EffectWindow *w = windowlist[i];
        if (!motionManager.isManaging(w))
            continue;

        QRect *target = &targets[w];
        float width = 0.0, height = 0.0;
        bool isFill = false;
        if (target->height() > scaleHeight) {
            float scale = (float)(scaleHeight / target->height());
            width = target->width() * scale;
            height = scaleHeight;
        } else {
            width = target->width();
            height = target->height();
            isFill = true;
        }
        totalw += width;
        totalw += m_scale[screen].spacingWidth;
        if (totalw > clientRect.width()) {
            row++;
            totalw = m_scale[screen].spacingWidth;
            totalw += width;
            totalw += m_scale[screen].spacingWidth;
            x = centerList[row - 1];
            winYPos += minSpacingH;
            winYPos += scaleHeight;
        }

        if (isReLayout) {
            motionManager.resetWindowFill(w);
            removeBackgroundFill(w, desktop);
        }

        target->setRect(x, winYPos + (scaleHeight - height) / 2, width, height);
        if (isFill) {
            QRect rect(x, winYPos, width, scaleHeight);
            motionManager.setWindowFill(w, true, rect);
            createBackgroundFill(w, rect, radius, desktop);
        }
        motionManager.setWindowRadius(w, radius);
        x += width;
        x += m_scale[screen].spacingWidth;

        motionManager.moveWindow(w, targets.value(w));

        //store window grids infomation
        m_effectFlyingBack.add(w, targets.value(w));
        if (m_flyingWinList.contains(w))
            m_windowEffect.add(w, QRect(), targets.value(w));
    }
}

QRect MultitaskViewEffect::calculateWorkspaceRect(int index, EffectScreen *screen, QRect maxRect, int customDesktopCount)
{
    int spacingScale = maxRect.y() + m_scale[screen].spacingHeight;
    int workspacingScale = m_scale[screen].workspacingWidth;
    int nums = customDesktopCount ? customDesktopCount : effects->numberOfDesktops();
    int thumbWidth = m_scale[screen].workspaceWidth;
    int xfpos = maxRect.width() / 2 - ((thumbWidth * nums) / 2) - (nums / 2 * (workspacingScale/* / 2*/));
    int xpos = xfpos + (thumbWidth + (workspacingScale /*/ 2*/)) * (index - 1);
    QRect rect(xpos + maxRect.x(), spacingScale, thumbWidth, m_scale[screen].workspaceHeight);

    return rect;
}

bool MultitaskViewEffect::calculateSwitchPos(QPoint diffPoint)
{
    int index = 0, num = effects->numberOfDesktops();
    if (diffPoint.x() < 0 && m_moveWorkspacedirection != mvLeft) {
        m_moveWorkspacedirection = mvLeft;
        m_moveWorkspaceNum = -1;
        restoreWorkspacePos(index, num);
    } else if (diffPoint.x() > 0 && m_moveWorkspacedirection != mvRight) {
        m_moveWorkspacedirection = mvRight;
        m_moveWorkspaceNum = -1;
        restoreWorkspacePos(index, num);
    } else if (diffPoint.x() == 0) {
        restoreWorkspacePos(index, num);
        return false;
    }

    int restore = 1;    // 0 restore self; 1 restore other;
    bool bFlag = true;
    if (m_aciveMoveDesktop == 1 && m_moveWorkspacedirection == mvLeft) {
        bFlag = false;
        index = m_aciveMoveDesktop;
    } else if (m_aciveMoveDesktop == effects->numberOfDesktops() && m_moveWorkspacedirection == mvRight) {
        bFlag = false;
        num = m_aciveMoveDesktop - 1;
    } else {
        int frontdiff = abs(diffPoint.x()) - (m_scale[m_screen].workspaceWidth * 2 / 3 + m_scale[m_screen].workspacingWidth);
        int afterdiff = abs(diffPoint.x()) - (m_scale[m_screen].workspaceWidth / 3);

        if (frontdiff > 0) {
            int fnum = frontdiff / (m_scale[m_screen].workspaceWidth + m_scale[m_screen].workspacingWidth);
            if (m_moveWorkspacedirection == mvLeft && fnum > m_aciveMoveDesktop - 2) {
                fnum = m_aciveMoveDesktop - 2;
            } else if (m_moveWorkspacedirection == mvRight && fnum > (effects->numberOfDesktops() - m_aciveMoveDesktop - 1)) {
                fnum = effects->numberOfDesktops() - m_aciveMoveDesktop - 1;
            }

            int anum = afterdiff / (m_scale[m_screen].workspaceWidth + m_scale[m_screen].workspacingWidth);
            if (m_moveWorkspacedirection == mvLeft && anum > m_aciveMoveDesktop - 1) {
                anum = m_aciveMoveDesktop - 1;
            } else if (m_moveWorkspacedirection == mvRight && anum > (effects->numberOfDesktops() - m_aciveMoveDesktop)) {
                anum = effects->numberOfDesktops() - m_aciveMoveDesktop;
            }

            if (m_moveWorkspaceNum < fnum) {
                m_moveWorkspaceNum = fnum;
            }

            for (int i = 0; i <= fnum; i++) {
                if (m_moveWorkspacedirection == mvLeft) {
                    MultiViewWorkspace *target = getWorkspaceObject(m_screen, m_aciveMoveDesktop - 2 - i);
                    MultiViewWorkspace *pretarget = getWorkspaceObject(m_screen, m_aciveMoveDesktop - i - 1);
                    MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(m_aciveMoveDesktop - 2 - i);
                    if (target && pretarget && wkmobj && target->m_posStatus != mvRight) {
                        target->m_posStatus = mvRight;
                        target->updateGeometry(pretarget->getRect());

                        int xdiff = pretarget->getRect().x() - target->getRect().x();
                        wkmobj->updatePos(m_screen, target->getCurrentRect(), QPoint(xdiff, 0));
                    }
                } else if (m_moveWorkspacedirection == mvRight) {
                    MultiViewWorkspace *target = getWorkspaceObject(m_screen, m_aciveMoveDesktop + i);
                    MultiViewWorkspace *pretarget = getWorkspaceObject(m_screen, m_aciveMoveDesktop + i - 1);
                    MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(m_aciveMoveDesktop + i);
                    if (target && pretarget && wkmobj && target->m_posStatus != mvLeft) {
                        target->m_posStatus = mvLeft;
                        target->updateGeometry(pretarget->getRect());

                        int xdiff = pretarget->getRect().x() - target->getRect().x();
                        wkmobj->updatePos(m_screen, target->getCurrentRect(), QPoint(xdiff, 0));
                    }
                }
            }

            if (m_moveWorkspacedirection == mvLeft) {
                num = m_aciveMoveDesktop - 2 - anum;
            } else {
                index = m_aciveMoveDesktop + 1 + anum;
            }

            if (m_moveWorkspaceNum > anum)
                m_moveWorkspaceNum = anum;

        } else if (afterdiff < 0) {
            bFlag = false;
            m_moveWorkspaceNum = -1;
        } else if (afterdiff > 0) {
            restore = 0;
            if (m_moveWorkspaceNum == -1)
                bFlag = false;
        } else {
            bFlag = false;
        }
    }

    if (restore) {
        restoreWorkspacePos(index, num);
    }

    return bFlag;
}

void MultitaskViewEffect::restoreWorkspacePos(int index, int num)
{
    for (; index < num; index++) {
        if (index == m_aciveMoveDesktop - 1)
            continue;
        MultiViewWorkspace *wkobj = getWorkspaceObject(m_screen, index);
        if (wkobj) {
            if (wkobj->m_posStatus == mvNone) {
                continue;
            } else {
                wkobj->m_posStatus = mvNone;
                QRect currentR = wkobj->getCurrentRect();
                wkobj->updateGeometry(wkobj->getRect());

                int xdiff = wkobj->getCurrentRect().x() - currentR.x();
                MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(index);
                if (xdiff != 0 && wkmobj) {
                    wkmobj->updatePos(m_screen, wkobj->getCurrentRect(), QPoint(xdiff, 0));
                }
            }
        }
    }
}

void MultitaskViewEffect::initWorkspaceBackground()
{
    getScreenInfo();

    QHash<QString, ScreenInfo_st>::iterator it = m_screenInfoList.begin();
    for (; it != m_screenInfoList.end(); ++it) {
        QList<MultiViewWorkspace *> list;
        int count = effects->numberOfDesktops();
        for (int i = 1; i <= count; i++) {
            MultiViewWorkspace *b = new MultiViewWorkspace();
            QRect rect = calculateWorkspaceRect(i, it.value().screen, it.value().rect);
            BgInfo_st st;
            st.desktop = i;
            st.screenName = it.value().name;
            st.desktopSize = it.value().screenrect.size();
            st.workspaceSize = rect.size();
            QPixmap bgPix, wpPix;
            MultiViewBackgroundManager::instance()->getWorkspaceBgPath(st, bgPix, wpPix);
            b->setArea(it.value().rect, it.value().screenrect);
            b->setMaxHeight(m_maxHeight);
            b->setImage(bgPix, wpPix, rect);
            b->setScreenDesktop(it.value().screen, i);
            list.push_back(b);
        }
        m_workspaceBackgrounds[it.value().screen] = list;

        {
            std::unique_ptr<EffectFrameEx> bgframe = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/hoverbg.qml", false);
            bgframe->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            bgframe->setColor("#E0E0E0");
            bgframe->setRadius(5);
            std::unique_ptr<EffectFrameEx> frame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/text.qml", false);
            frame->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            QFont font;
            font.setPointSize(12);
            frame->setFont(font);
            frame->setText(tr("No windows"));
            frame->setPosition(QPoint(it.value().screenrect.width() / 2 + it.value().screenrect.x(), it.value().screenrect.height() / 2 + it.value().screenrect.y()));
            m_tipFrames[it.value().screen].push_back(std::move(bgframe));
            m_tipFrames[it.value().screen].push_back(std::move(frame));

            MultiViewAddButton *addButton = new MultiViewAddButton();
            QRect buttonRect(it.value().rect.x() + it.value().rect.width() - it.value().rect.width() * ADDBTN_W_SCALE,
                             it.value().rect.y() + it.value().rect.height() * ADDBTN_H_SCALE,
                             it.value().rect.width() * ADDBTN_SIZE_SCALE,
                             it.value().rect.width() * ADDBTN_SIZE_SCALE);
            QString buttonImage = QLatin1String(add_workspace_png);
            addButton->setArea(it.value().rect, it.value().screenrect);
            addButton->setMaxHeight(m_maxHeight);
            addButton->setImage(buttonImage, buttonRect);
            m_addWorkspaceButton[it.value().screen] = addButton;
        }
    }

    MultiViewBackgroundManager::instance()->getBackgroundList();
}

void MultitaskViewEffect::createEffectFrame()
{
    if (!m_hoverWinFrame) {
        m_hoverWinFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/hover.qml", false);
    }

    if (!m_closeWinFrame) {
        m_closeWinFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/icon.qml", true);
        m_closeWinFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_closeWinFrame->setImage(QUrl::fromLocalFile(MULTITASK_CLOSE_SVG));
        m_closeWinFrame->setGeometry(QRect(0, 0, 24 * effectsEx->getOsScale(), 24 * effectsEx->getOsScale()));
    }

    if (!m_topWinFrame) {
        m_topWinFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/icon.qml", true);
        m_topWinFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_topWinFrame->setGeometry(QRect(0, 0, 24 * effectsEx->getOsScale(), 24 * effectsEx->getOsScale()));
    }

    if (!m_textWinFrame) {
        m_textWinFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/text.qml", false);
    }

    if (!m_textWinBgFrame) {
        m_textWinBgFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/hoverbg.qml", false);
        m_textWinBgFrame->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        m_textWinBgFrame->setRadius(10);
        m_textWinBgFrame->setColor("#E0E0E0");
    }

    if (!m_previewFrame) {
        m_previewFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/workspacebg.qml", true);
        m_previewFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_previewFrame->setRadius(10);
    }

    if (!m_closeWorkspaceFrame) {
        m_closeWorkspaceFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/icon.qml", true);
        m_closeWorkspaceFrame->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_closeWorkspaceFrame->setImage(QUrl::fromLocalFile(MULTITASK_CLOSE_SVG));
        m_closeWorkspaceFrame->setGeometry(QRect(0, 0, 24 * effectsEx->getOsScale(), 24 * effectsEx->getOsScale()));
    }

    if (!m_dragTipsFrame) {
        m_dragTipsFrame = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/dragtips.qml", false);
        m_dragTipsFrame->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        m_dragTipsFrame->setImage(QUrl::fromLocalFile(delete_workspace_png));
        m_dragTipsFrame->setIconSize(QSize(21 * m_scalingFactor, 21 * m_scalingFactor));
        m_dragTipsFrame->setText(tr("Drag upwards to remove"));
        m_dragTipsFrame->setColor(QColor(Qt::white));
    }

    if (!m_dragTipsFrameShadow) {
        m_dragTipsFrameShadow = effectsEx->effectFrameEx("deepin-kwin/effects/multitaskview/qml/dragtips.qml", false);
        m_dragTipsFrameShadow->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        m_dragTipsFrameShadow->setImage(QUrl::fromLocalFile(delete_workspace_png));
        m_dragTipsFrameShadow->setIconSize(QSize(22 * m_scalingFactor, 21 * m_scalingFactor));
        m_dragTipsFrameShadow->setText(tr("Drag upwards to remove"));
        m_dragTipsFrameShadow->setColor(QColor(Qt::black));
    }
}

void MultitaskViewEffect::cacheWorkspaceBackground()
{
    getScreenInfo();
    int count = effects->numberOfDesktops();
    QHash<QString, ScreenInfo_st>::iterator it = m_screenInfoList.begin();
    for (; it != m_screenInfoList.end(); ++it) {
        for (int i = 1; i <= count; i++) {
            QRect rect = calculateWorkspaceRect(i, it.value().screen, it.value().rect);
            BgInfo_st st;
            st.desktop = i;
            st.screenName = it.value().name;
            st.desktopSize = it.value().screenrect.size();
            st.workspaceSize = rect.size();
            CustomThread *thread = new CustomThread(st);
            connect(thread, &QThread::finished, thread, &QObject::deleteLater);
            thread->start();
        }
    }
}

void MultitaskViewEffect::updateWorkspacePos()
{
    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); ++iter) {
        QList<MultiViewWorkspace *> list = iter.value();
        for (int j = 0; j < list.size(); ++j) {
            QRect rect = calculateWorkspaceRect(j + 1, iter.key(), list[j]->getClientArea());
            list[j]->setRect(rect);
        }
    }
}

void MultitaskViewEffect::updateWorkspacePos(int removedesktop)
{
    int index = 0;
    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); ++iter) {
        QList<MultiViewWorkspace *> list = iter.value();
        for (int j = 0; j < list.size(); ++j) {
            if (j == removedesktop - 1)
                continue;
            QRect rect = calculateWorkspaceRect(index + 1, iter.key(), list[j]->getClientArea(), effects->numberOfDesktops() - 1);
            list[j]->setRect(rect);
            ++index;
        }
        index = 0;
    }
}

void MultitaskViewEffect::getScreenInfo()
{
    m_screenInfoList.clear();
    m_allFullArea = effects->clientArea(FullArea, effects->screens().first(), effects->currentDesktop()).toRect();

    for (Output *output : workspace()->outputs()) {
        EffectScreen *effectScreen = effectsEx->findScreen(output);
        QRect rect = effects->clientArea(FullScreenArea, effectScreen, effects->currentDesktop()).toRect();
        QRect maxRect = effects->clientArea(MaximizeArea, effectScreen, effects->currentDesktop()).toRect();
        ScreenInfo_st infost;
        infost.rect = maxRect;
        infost.screenrect = rect;
        infost.screen = effectScreen;
        //use v23 support EffectScreen get name
        infost.name = effectScreen->name();

        m_screenInfoList[infost.name] = infost;

        Scale_st scalest;
        scalest.fullArea = rect;
        scalest.spacingHeight = rect.height() * SPACING_H_SCALE;
        scalest.spacingWidth = rect.width() * SPACING_W_SCALE;
        scalest.workspacingWidth = rect.width() * WORK_SPACING_SCALE;
        scalest.workspaceWidth = rect.width() * WORKSPACE_SCALE;
        scalest.workspaceHeight = rect.height() * WORKSPACE_SCALE;
        scalest.workspaceMgrHeight = scalest.spacingHeight * 2 + rect.height() * WORKSPACE_SCALE;
        scalest.workspaceMgrRect = QRect(maxRect.x(), maxRect.y(), maxRect.width(), scalest.workspaceMgrHeight);
        scalest.windowMgrRect = QRect(maxRect.x(), maxRect.y() + scalest.workspaceMgrHeight, maxRect.width(), maxRect.height() - scalest.workspaceMgrHeight);
        m_scale[effectScreen] = scalest;

        if (m_maxHeight < rect.y() + rect.height())
            m_maxHeight = rect.y() + rect.height();
        if (!isExtensionMode()) {
            break;
        }

    }
}

int MultitaskViewEffect::getNumScreens()
{
    int num = 1;
    if (isExtensionMode()) {
        num = effects->screens().size();
    } else if (effects->screens().size() <= 0) {
        num = 0;
    }
    return num;
}

bool MultitaskViewEffect::isExtensionMode() const
{
    if (effects->screens().size() >= 2) {
        QRect rect = effects->clientArea(FullScreenArea, effects->findScreen(0), effects->currentDesktop()).toRect();
        if (effects->virtualScreenGeometry() != rect) {
            return true;
        }
    }
    return false;
}

void MultitaskViewEffect::createBackgroundFill(EffectWindow *w, QRect rect, float radius, int desktop)
{
    MultiViewWinManager *wmobj = getWinManagerObject(desktop - 1);
    if (!wmobj || wmobj->isHaveWinFill(w))
        return;

    MultiViewWinFill *fill = new MultiViewWinFill(w->screen(), rect, radius, m_maxHeight);
    wmobj->setWinFill(w, fill);
}

void MultitaskViewEffect::removeBackgroundFill(EffectWindow *w, int desktop)
{
    MultiViewWinManager *wmobj = getWinManagerObject(desktop - 1);
    if (!wmobj || !wmobj->isHaveWinFill(w))
        return;

    wmobj->removeWinFill(w);
}

void MultitaskViewEffect::addNewDesktop()
{
    int count = effects->numberOfDesktops();
    if (count >= MAX_DESKTOP_COUNT)
        return;

    m_isShieldEvent = true;
    effects->setNumberOfDesktops(count + 1);
    if (!getWinManagerObject(count - 1))
        return;
    QList<QString> list = m_screenInfoList.keys();
    for (int i = 0; i < list.size(); i++) {
        QString screenName = list[i];
        ScreenInfo_st &st = m_screenInfoList[screenName];
        QRect rect = calculateWorkspaceRect(count + 1, st.screen, st.rect);
        BgInfo_st bgst;
        bgst.desktop = count + 1;
        bgst.screen = st.screen;
        bgst.screenName = screenName;
        bgst.desktopSize = st.screenrect.size();
        bgst.workspaceSize = rect.size();
        QPixmap bgPix, wpPix;
        MultiViewBackgroundManager::instance()->setNewBackground(bgst, bgPix, wpPix);
        MultiViewWorkspace *b = new MultiViewWorkspace();
        b->setMaxHeight(m_maxHeight);
        b->setArea(st.rect, st.screenrect);
        b->setImage(bgPix, wpPix, rect);
        b->setScreenDesktop(st.screen, count + 1);
        m_workspaceBackgrounds[st.screen].push_back(b);
    }

    {
        m_workspaceSlidingStatus = true;
        m_workspaceSlidingTimeline.reset();
        m_workspaceSlidingInfo.clear();
        {
            for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
                QRect maxRect = effects->clientArea(MaximizeArea, iter.key(), 1).toRect();
                QPoint pos(maxRect.x() + maxRect.width() - 20 - m_scale[iter.key()].workspaceWidth / 2, maxRect.y() + m_scale[iter.key()].spacingHeight);
                QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
                for (int j = 0; j < list.size(); j++) {
                    QRect rect = list[j]->getRect();
                    m_workspaceSlidingInfo[list[j]].first = rect.x();
                    // 遍历到最后一个工作区移动的时候,使其新增的工作区的添加动画的起始坐标为要求固定位置
                    if (j == list.size() - 1) {
                        m_workspaceSlidingInfo[list[j]].first = pos.x();
                    }
                }
            }
        }
    }

    updateWorkspacePos();
    EffectWindowList windowsList = effects->stackingOrder();
    setWinLayout(count + 1, windowsList);
    updateWorkspaceWinLayout(count);

    {
        {
            for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
                QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
                for (int j = 0; j < list.size(); j++) {
                    QRect rect = list[j]->getRect();
                    m_workspaceSlidingInfo[list[j]].second = rect.x();
                }
            }
        }
    }

    changeCurrentDesktop(effects->numberOfDesktops());
    m_isShieldEvent = false;
}

void MultitaskViewEffect::removeDesktop(int desktop)
{
    m_isShieldEvent = true;
    int count = effects->numberOfDesktops();
    if (desktop <= 0 || desktop > count || count == 1) {
        MultiViewWorkspace *wt = getWorkspaceObject(m_screen, desktop - 1);
        if (wt) {
            wt->updateGeometry(wt->getRect());
        }
        m_isShieldEvent = false;
        return;
    }
    m_windowEffect.reset();
    m_flyingWinList.clear();
    m_windowInfo.clear();
    m_deleteWorkspaceDesktop = desktop;
     int currentDesktop = effects->currentDesktop();
    if (currentDesktop == desktop) {
        MultiViewWinManager *wmobj = getWinManagerObject(desktop - 1);
        if (wmobj) {
            m_flyingWinList = wmobj->getDesktopWinList();
            wmobj->getDesktopWinInfo(m_windowInfo);
            for (auto it = m_windowInfo.begin(); it != m_windowInfo.end(); ++it) {
                for (const auto& w : it.value().managedWindows()) {
                    // minfo[w] = it.value().transformedGeometry(w);
                    m_windowEffect.add(w, it.value().transformedGeometry(w).toRect(), QRect());
                }
            }
        }
    }

    if (m_motionManagers.size() >= desktop) {
        delete m_motionManagers[desktop - 1];
        m_motionManagers.erase(m_motionManagers.begin() + desktop - 1);
    }
    if (m_workspaceWinMgr.size() >= desktop) {
        delete m_workspaceWinMgr[desktop - 1];
        m_workspaceWinMgr.erase(m_workspaceWinMgr.begin() + desktop - 1);
    }

    QHash<EffectScreen *, EffectWindowList> winManager;
    bool isRelayout = false;
    int newd = 0;
    QSet<EffectScreen *> screens;
    for (const auto &ew : effects->stackingOrder()) {
        if (!isRelevantWithPresentWindows(ew)) {
            continue;
        }
        if (ew->isOnAllDesktops()) {
            if (winManager.contains(ew->screen())) {
                winManager[ew->screen()].push_back(ew);
            } else {
                EffectWindowList list; list.push_back(ew);
                winManager[ew->screen()] = list;
            }
            isRelayout = true;
            screens.insert(ew->screen());
            continue;
        }

        auto dl = ew->desktops();
        int ntmp = desktop == 1 ? 2 : desktop - 1;
        if (dl[0] == ntmp) {
            if (winManager.contains(ew->screen())) {
                winManager[ew->screen()].push_back(ew);
            } else {
                EffectWindowList list; list.push_back(ew);
                winManager[ew->screen()] = list;
            }
        }
        if (dl.size() == 0 || dl[0] < desktop) {
            continue;
        }

        if (dl[0] == desktop) {
            newd = dl[0] == 1 ? 1 : dl[0] - 1;
            MultiViewWinManager *wmobj = getWinManagerObject(newd - 1);
            MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(newd - 1);
            if (wmobj && wkmobj) {
                isRelayout = true;
                wmobj->manageWin(ew->screen(), ew);
                wkmobj->manageWin(ew->screen(), ew);
                screens.insert(ew->screen());
                if (winManager.contains(ew->screen())) {
                    winManager[ew->screen()].push_back(ew);
                } else {
                    EffectWindowList list; list.push_back(ew);
                    winManager[ew->screen()] = list;
                }
            }
        }
    }

    for (const auto &ew : effects->stackingOrder()) {
        if (ew->isOnAllDesktops()) {
            continue;
        }

        auto dl = ew->desktops();
        if (dl.size() == 0 || dl[0] < desktop) {
            continue;
        }

        if (effectsEx->isTransientWin(ew) && !ew->isModal()) {
            continue;
        }
        newd = dl[0] == 1 ? 1 : dl[0] - 1;
        QVector<uint> desks{(uint)newd};
        effects->windowToDesktops(ew, desks);
    }

    desktopAboutToRemoved(desktop, effects->numberOfDesktops());
    // QFuture<void> f = QtConcurrent::run(this, &MultitaskViewEffect::desktopAboutToRemoved, desktop, effects->numberOfDesktops());

    int nrelyout = desktop == 1 ? desktop : (desktop - 1);
    if (isRelayout) {
        for (auto iter = screens.begin(); iter != screens.end(); iter++) {
            WindowMotionManager *wmm;
            MultiViewWinManager *wmobj = getWinManagerObject(nrelyout - 1);
            if (wmobj && wmobj->getMotion(nrelyout, *iter, wmm)) {
                calculateWindowTransformations(winManager.contains(*iter) ? winManager[*iter] : wmm->orderManagedWindows(), *wmm, nrelyout, *iter, true);
            }
            workspaceWinRelayout(nrelyout, *iter);
        }
    }

    if (currentDesktop == desktop) {
        effects->setCurrentDesktop(nrelyout);
        m_bgSlidingStatus = true;
        m_bgSlidingTimeLine.reset();
        m_curDesktopIndex = nrelyout;
        if (desktop == 1)
            m_curDesktopIndex = desktop + 1;
        m_lastDesktopIndex = desktop;
        m_isRemoveWorkspace = true;
        startremoveDesktopEffect(desktop);
        m_windowEffectState = true;
        m_windowEffect.begin();
        effects->addRepaintFull();
    } else {
        m_curDesktopIndex = effects->currentDesktop();
        if (m_curDesktopIndex > desktop)
            effects->setCurrentDesktop(m_curDesktopIndex == 1 ? m_curDesktopIndex : (m_curDesktopIndex - 1));
        removeDesktopEx(desktop);
    }
}

void MultitaskViewEffect::startremoveDesktopEffect(int desktop)
{
    m_workspaceSlidingStatus = true;
    m_workspaceSlidingTimeline.reset();
    m_workspaceSlidingInfo.clear();
    {
        for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
            QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
            for (int j = 0; j < list.size(); j++) {
                if (j == desktop - 1)
                    continue;
                QRect rect = list[j]->getRect();
                m_workspaceSlidingInfo[list[j]].first = rect.x();
            }
        }
    }

    updateWorkspacePos(desktop);
    updateWorkspaceWinLayout(effects->numberOfDesktops() - 1, desktop);

    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
        QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
        for (int j = 0; j < list.size(); j++) {
            if (j == desktop - 1)
                continue;
            QRect rect = list[j]->getRect();
            m_workspaceSlidingInfo[list[j]].second = rect.x();
        }
    }
}

void MultitaskViewEffect::removeDesktopEx(int desktop)
{
    QMutexLocker locker(&m_mutex);
    effects->setNumberOfDesktops(effects->numberOfDesktops() - 1);
    m_isRemoveWorkspace = false;
    m_deleteWorkspaceDesktop = -1;
    for (auto &workspace_list : m_workspaceBackgrounds) {
        delete workspace_list[desktop - 1];
        workspace_list.removeAt(desktop - 1);
    }

    {
        m_workspaceSlidingStatus = true;
        m_workspaceSlidingTimeline.reset();
        m_workspaceSlidingInfo.clear();
        {
            for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
                QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
                for (int j = 0; j < list.size(); j++) {
                    QRect rect = list[j]->getRect();
                    m_workspaceSlidingInfo[list[j]].first = rect.x();
                }
            }
        }
    }

    updateWorkspacePos();
    updateWorkspaceWinLayout(effects->numberOfDesktops());

    {
        for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
            QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[iter.key()];
            for (int j = 0; j < list.size(); j++) {
                QRect rect = list[j]->getRect();
                m_workspaceSlidingInfo[list[j]].second = rect.x();
            }
        }
    }

    m_isShieldEvent = false;
}

void MultitaskViewEffect::cleanDesktopData(int desktop)
{
    QMutexLocker locker(&m_mutex);
    effects->setNumberOfDesktops(effects->numberOfDesktops() - 1);
    m_isRemoveWorkspace = false;
    m_deleteWorkspaceDesktop = -1;
    for (auto iter = m_workspaceBackgrounds.begin(); iter != m_workspaceBackgrounds.end(); iter++) {
        delete m_workspaceBackgrounds[iter.key()][desktop - 1];
        m_workspaceBackgrounds[iter.key()].removeAt(desktop - 1);
    }
    m_isShieldEvent = false;
}

void MultitaskViewEffect::switchDesktop()
{
    for (auto screenInfo : m_screenInfoList) {
        EffectScreen *effectScreen = screenInfo.screen;
        for (int j = 0; j <= m_moveWorkspaceNum; j++) {
            MultiViewWorkspace *target = getWorkspaceObject(effectScreen, m_aciveMoveDesktop - 1);
            MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(m_aciveMoveDesktop - 1);
            if (target && wkmobj && m_moveWorkspacedirection == mvLeft) {
                MultiViewWorkspace *nexttarget = getWorkspaceObject(effectScreen, m_aciveMoveDesktop - 2 -j);
                if (nexttarget) {
                    QRect tmpRect = target->getRect();
                    target->setRect(nexttarget->getRect());
                    nexttarget->setRect(tmpRect);

                    if (effectScreen != m_screen) {
                        MultiViewWinManager *wkmobj1 = getWorkspaceWinManagerObject(m_aciveMoveDesktop - 2 - j);
                        if (wkmobj1) {
                            int xdiff = target->getRect().x() - tmpRect.x();
                            wkmobj->updatePos(effectScreen, target->getRect(), QPoint(xdiff, 0));
                            wkmobj1->updatePos(effectScreen, tmpRect, QPoint(-xdiff, 0));
                        }
                    }
                    if (nexttarget->m_posStatus != mvNone) {
                        nexttarget->m_posStatus = mvNone;
                    }
                }
            } else if (target && wkmobj && m_moveWorkspacedirection == mvRight) {
                QRect tmpRect = target->getRect();

                MultiViewWorkspace *pretarget = getWorkspaceObject(effectScreen, m_aciveMoveDesktop + j);
                if (pretarget) {
                    target->setRect(pretarget->getRect());
                    pretarget->setRect(tmpRect);

                    if (effectScreen != m_screen) {
                        MultiViewWinManager *wkmobj1 = getWorkspaceWinManagerObject(m_aciveMoveDesktop + j);
                        if (wkmobj1) {
                            int xdiff = target->getRect().x() - tmpRect.x();
                            wkmobj->updatePos(effectScreen, target->getRect(), QPoint(xdiff, 0));
                            wkmobj1->updatePos(effectScreen, tmpRect, QPoint(-xdiff, 0));
                        }
                    }
                    if (pretarget->m_posStatus != mvNone) {
                        pretarget->m_posStatus = mvNone;
                    }
                }
            }
        }

        if (m_moveWorkspacedirection == mvLeft) {
            MultiViewWorkspace *wp = m_workspaceBackgrounds[effectScreen].takeAt(m_aciveMoveDesktop - 1);
            m_workspaceBackgrounds[effectScreen].insert(m_aciveMoveDesktop - 2 - m_moveWorkspaceNum, wp);
        } else if (m_moveWorkspacedirection == mvRight) {
            MultiViewWorkspace *wp = m_workspaceBackgrounds[effectScreen].takeAt(m_aciveMoveDesktop - 1);
            m_workspaceBackgrounds[effectScreen].insert(m_aciveMoveDesktop + m_moveWorkspaceNum, wp);
        }
    }

    int ncurrent = effects->currentDesktop();
    int index = m_aciveMoveDesktop - 1;
    MultiViewWinManager *wmobj = getWinManagerObject(index);
    MultiViewWorkspace *target = getWorkspaceObject(m_screen, index);
    MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(index);
    if (wmobj && target && wkmobj && m_moveWorkspacedirection == mvLeft) {
        MultiViewWorkspace *nexttarget = getWorkspaceObject(m_screen, index - 1 - m_moveWorkspaceNum);
        if (nexttarget) {
            m_motionManagers.insert(index - 1 - m_moveWorkspaceNum, wmobj);
            m_motionManagers.erase(m_motionManagers.begin() + index + 1);

            wkmobj->updatePos(m_screen, nexttarget->getRect(), QPoint(nexttarget->getRect().x() - target->getRect().x(), 0));
            m_workspaceWinMgr.insert(index - 1 - m_moveWorkspaceNum, wkmobj);
            m_workspaceWinMgr.erase(m_workspaceWinMgr.begin() + index + 1);
        }
        desktopSwitchPosition(m_aciveMoveDesktop - 1 - m_moveWorkspaceNum, m_aciveMoveDesktop);
        if (m_aciveMoveDesktop == ncurrent) {
            ncurrent = m_aciveMoveDesktop - 1 - m_moveWorkspaceNum;
        } else if (m_aciveMoveDesktop > ncurrent && m_aciveMoveDesktop - 1 - m_moveWorkspaceNum <= ncurrent) {
            ncurrent += 1;
        }
    } else if (wmobj && target && wkmobj && m_moveWorkspacedirection == mvRight) {
        MultiViewWorkspace *pretarget = getWorkspaceObject(m_screen, index + 1 + m_moveWorkspaceNum);
        if (pretarget) {
            m_motionManagers.insert(index + 2 + m_moveWorkspaceNum, wmobj);
            m_motionManagers.erase(m_motionManagers.begin() + index);
            wkmobj->updatePos(m_screen, pretarget->getRect(), QPoint(pretarget->getRect().x() - target->getRect().x(), 0));
            m_workspaceWinMgr.insert(index + 2 + m_moveWorkspaceNum, wkmobj);
            m_workspaceWinMgr.erase(m_workspaceWinMgr.begin() + index);

        }
        desktopSwitchPosition(m_aciveMoveDesktop + 1 + m_moveWorkspaceNum, m_aciveMoveDesktop);
        if (m_aciveMoveDesktop == ncurrent) {
            ncurrent = m_aciveMoveDesktop + 1 + m_moveWorkspaceNum;
        } else if (m_aciveMoveDesktop < ncurrent && m_aciveMoveDesktop + 1 + m_moveWorkspaceNum >= ncurrent) {
            ncurrent -= 1;
        }
    }

    int dir = m_moveWorkspacedirection == 1 ? 1 : -1;
    for (const auto& ew: effects->stackingOrder()) {
        if (ew->isOnAllDesktops())
            continue;
        if (effectsEx->isTransientWin(ew) && !ew->isModal() && !ew->isDialog())
            continue;

        auto dl = ew->desktops();
        if (dl[0] == m_aciveMoveDesktop && !ew->isModal()) {
            QVector<uint> desks {(uint)(m_aciveMoveDesktop - dir - m_moveWorkspaceNum * dir)};
            effects->windowToDesktops(ew, desks);
        } else if (dl[0] >= (m_aciveMoveDesktop - dir - (dir > 0 ? m_moveWorkspaceNum : 0)) && dl[0] <= (m_aciveMoveDesktop - dir + (dir > 0 ? 0 : m_moveWorkspaceNum)) && !ew->isModal()) {
            int newd = dl[0] + dir;
            QVector<uint> desks {(uint)newd};
            effects->windowToDesktops(ew, desks);
        }
    }
    effects->setCurrentDesktop(ncurrent);
}

void MultitaskViewEffect::desktopSwitchPosition(int to, int from)
{
    QDBusInterface wm(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);

    QList<QString> list = m_screenInfoList.keys();
    for (int i = 0; i < list.size(); i++) {
        QString monitorName = list[i];

        auto &list = MultiViewBackgroundManager::instance()->m_allBackgroundList[monitorName];
        QString strFromUri = list[from];

        if (from < to) {
            for (int j = from - 1; j < to; j++) {
                int desktopIndex = j + 1; //desktop index
                if ( desktopIndex == to) {
                    list[desktopIndex] = strFromUri;
                    wm.call("SetWorkspaceBackgroundForMonitor", desktopIndex, monitorName, strFromUri);
                } else {
                    list[desktopIndex] = list[desktopIndex + 1];
                    wm.call("SetWorkspaceBackgroundForMonitor", desktopIndex, monitorName, list[desktopIndex]);
                }
            }
        } else {
            for (int j = from; j > to - 1; j--) {
                if (j == to) {
                    list[to] = strFromUri;
                    wm.call("SetWorkspaceBackgroundForMonitor", to, monitorName, strFromUri);
                } else {
                    list[j] = list[j - 1];
                    wm.call("SetWorkspaceBackgroundForMonitor", j, monitorName, list[j]);
                }
            }
        }
    }
}

void MultitaskViewEffect::desktopAboutToRemoved(int d, int num)
{
    QDBusInterface wm(DBUS_APPEARANCE_SERVICE, DBUS_APPEARANCE_OBJ, DBUS_APPEARANCE_INTF);

    QList<QString> list = m_screenInfoList.keys();
    for (int i = 0; i < list.count(); i++) {
        QString monitorName = list.at(i);

        auto &list = MultiViewBackgroundManager::instance()->m_allBackgroundList[monitorName];

        MultiViewBackgroundManager::instance()->updateBackgroundList(list[d]);

        for (int i = d; i < effects->numberOfDesktops(); i++) {
            list[i] = list[i + 1];
            wm.call("SetWorkspaceBackgroundForMonitor", i, monitorName, list[i]);
        }
        list.removeLast();
    }
}

void MultitaskViewEffect::removeWinAndRelayout(EffectWindow *w)
{
    QMutexLocker locker(&m_mutex);
    if (w->desktop() == -1) {
        for (int wi = 0; wi < m_workspaceWinMgr.size(); wi++) {
            m_workspaceWinMgr[wi]->removeWin(w->screen(), w);
        }
        for (int i = 0; i < m_motionManagers.size(); i++) {
            m_motionManagers[i]->removeWin(w->screen(), w);
            WindowMotionManager *wmm;
            if (m_motionManagers[i]->getMotion(i + 1, w->screen(), wmm)) {
                calculateWindowTransformations(wmm->orderManagedWindows(), *wmm, i + 1, w->screen(), true);
            }
        }

    } else {
        int desktop = w->desktop();
        MultiViewWinManager *wmobj = getWinManagerObject(desktop - 1);
        MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(desktop - 1);
        if (wmobj && wkmobj) {
            wkmobj->removeWin(w->screen(), w);
            wmobj->removeWin(w->screen(), w);
            WindowMotionManager *wmm;
            if (wmobj->getMotion(desktop, w->screen(), wmm)) {
                calculateWindowTransformations(wmm->orderManagedWindows(), *wmm, desktop, w->screen(), true);
            }
        }
    }

    for (int i = 0; i < effects->numberOfDesktops(); i++) {
        MultiViewWinManager *wmobj = getWinManagerObject(i);
        EffectWindowList list;
        if (wmobj) {
            list = wmobj->getDesktopWinList();
        }
        if (list.size() != 0) {
            return;
        }
    }

    if (isRelevantWithPresentWindows(w))
        setActive(false);
}

void MultitaskViewEffect::moveWindowChangeDesktop(EffectWindow *w, int todesktop, EffectScreen *toscreen, bool isSwitch)
{
    if (w->isOnAllDesktops()) {
        return;
    }

    if (effects->numberOfDesktops() < todesktop) {
        return;
    }

    int fromdesktop = w->desktop();
    QMutexLocker locker(&m_mutex);
    EffectScreen *screen = w->screen();
    effects->windowToScreen(w, toscreen);
    QVector<uint> desks{(uint)todesktop};
    effects->windowToDesktops(w, desks);

    EffectWindowList mainWinList;
    mainWinList = w->mainWindows();
    for (EffectWindow * mainWin : mainWinList) {
        effects->windowToScreen(mainWin, toscreen);
        QVector<uint> desks{(uint)todesktop};
        effects->windowToDesktops(mainWin, desks);
    }

    EffectWindowList childWinList;
    childWinList = effectsEx->getChildWinList(w);

    if (isSwitch) {
        effects->activateWindow(w);
    }

    MultiViewWinManager *wkmobj = getWorkspaceWinManagerObject(fromdesktop - 1);
    MultiViewWinManager *wkmobj1 = getWorkspaceWinManagerObject(todesktop - 1);
    if (wkmobj && wkmobj1) {
        for (EffectWindow * mainWin : mainWinList) {
            wkmobj->removeWin(screen, mainWin);
            wkmobj1->manageWin(toscreen, mainWin);
        }

        for (EffectWindow *childWin : childWinList) {
            wkmobj->removeWin(screen, childWin);
            wkmobj1->manageWin(toscreen, childWin);
        }

        wkmobj->removeWin(screen, w);
        wkmobj1->manageWin(toscreen, w);
        workspaceWinRelayout(fromdesktop, screen);
        workspaceWinRelayout(todesktop, toscreen);
    }

    MultiViewWinManager *wmobj = getWinManagerObject(fromdesktop - 1);
    if (wmobj) {
        for (EffectWindow * mainWin : mainWinList) {
            wmobj->removeWin(screen, mainWin);
        }

        for (EffectWindow *childWin : childWinList) {
            wmobj->removeWin(screen, childWin);
        }

        wmobj->removeWin(screen, w);
        WindowMotionManager *wmm;
        if (wmobj->getMotion(fromdesktop, screen, wmm)) {
            calculateWindowTransformations(wmm->orderManagedWindows(), *wmm, fromdesktop, screen, true);
        }
    }

    wmobj = getWinManagerObject(todesktop - 1);
    if (wmobj) {
        for (EffectWindow * mainWin : mainWinList) {
            wmobj->manageWin(toscreen, mainWin);
        }
        for (EffectWindow *childWin : childWinList) {
            wmobj->manageWin(toscreen, childWin);
        }

        wmobj->manageWin(toscreen, w);
        WindowMotionManager *wmmto;
        if (wmobj->getMotion(todesktop, toscreen, wmmto)) {
            calculateWindowTransformations(wmmto->orderManagedWindows(), *wmmto, todesktop, toscreen, true);
        }
    }
    m_curDesktopIndex = effects->currentDesktop();
}

bool MultitaskViewEffect::closeWindow(EffectWindow *w)
{
    if (effectsEx->getChildWinList(w).size() == 0) {
        m_closingWindow = true;
        w->closeWindow();
        m_timerCheckWindowClose->setSingleShot(true);
        m_timerCheckWindowClose->start(100);
        return true;
    }
    return false;
}

void MultitaskViewEffect::setWinKeepAbove(EffectWindow *w)
{
    if (w->keepAbove()) {
        effectsEx->setKeepAbove(w, false);
    } else {
        effectsEx->setKeepAbove(w, true);
    }

    effects->setElevatedWindow(w, false);
    effects->activateWindow(w);
}

void MultitaskViewEffect::changeCurrentDesktop(int desktop)
{
    m_curDesktopIndex = effects->currentDesktop();
    effects->setCurrentDesktop(desktop);
    m_hoverWin = nullptr;
    m_flyingWinList.clear();

    //sliding
    m_bgSlidingStatus = true;
    m_bgSlidingTimeLine.reset();
    m_lastDesktopIndex = m_curDesktopIndex;
    m_curDesktopIndex = desktop;
}

void MultitaskViewEffect::showWorkspacePreview(EffectScreen *screen, QRect addButtonRect, bool isClear)
{
    if (!isClear) {
        if (m_previewFrame->image().isEmpty()) {
            QPixmap wpPix;
            QSize size(m_scale[screen].workspaceWidth, m_scale[screen].workspaceHeight);
            MultiViewBackgroundManager::instance()->getPreviewBackground(size, wpPix, screen);

            QRect maxRect = effects->clientArea(MaximizeArea, screen, 1).toRect();
            QPoint pos1(maxRect.x() + maxRect.width() - 20 - m_scale[screen].workspaceWidth / 2, maxRect.y() + m_scale[screen].spacingHeight);

            m_previewFramePosX = pos1.x();

            m_previewFrame->setImage(wpPix);
            m_previewFrame->setGeometry(QRect(QPoint(pos1.x(), pos1.y()), size));
        }

        QRect rect = m_previewFrame->geometry();
        if (effects->waylandDisplay()) {
            int screenId = 0;
            for (; screenId < effects->screens().size(); screenId++) {
                if (effects->screens()[screenId] == screen)
                    break;
            }
        } else {
            glEnable(GL_SCISSOR_TEST);
            glScissor(rect.x(), m_allFullArea.height() - rect.y() - rect.height(), rect.width() / 2 + 20, rect.height());
        }

        float scale = m_scale[m_screen].fullArea.width() / 1920.0;
        m_previewFrame->render(infiniteRegion(), 1, 0.8);

        if (nullptr == effects->waylandDisplay()) {
            glDisable(GL_SCISSOR_TEST);
        }
    } else {
        if (m_previewFrame && !m_previewFrame->image().isEmpty()) {
            m_previewFrame->setImage(QUrl());
        }
    }
}

bool MultitaskViewEffect::checkConfig(EffectWindow *w)
{
    bool flag = true;
    if (!m_isShowWhole) {
        if (w->isMinimized()) {
            flag = false;
        }
    }
    return flag;
}

QVector<int> MultitaskViewEffect::desktopList(const EffectWindow *w) const
{
    if (w->isOnAllDesktops()) {
        static QVector<int> allDesktops;
        if (allDesktops.count() != effects->numberOfDesktops()) {
            allDesktops.resize(effects->numberOfDesktops());
            for (int i = 0; i < effects->numberOfDesktops(); ++i)
                allDesktops[i] = i+1;
        }
        return allDesktops;
    }

    QVector<int> desks;
    desks.resize(w->desktops().count());
    int i = 0;
    for (const int desk : w->desktops()) {
        desks[i++] = desk;
    }
    return desks;
}

EffectWindow *MultitaskViewEffect::getNextWindow(EffectWindow *w)
{
    EffectWindow *target = nullptr;
    EffectWindowList list;
    MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
    if (wmobj) {
        list = wmobj->getDesktopWinList();
    }

    int index = 0;
    for (int i = list.size() - 1; i >= 0; i--) {
        if (w == list[i]) {
            index = i - 1;
            if (index < 0) {
                target = list[list.size() - 1];
            } else {
                target = list[index];
            }
            break;
        }
    }
    return target;
}

EffectWindow *MultitaskViewEffect::getPreWindow(EffectWindow *w)
{
    EffectWindow *target = nullptr;
    EffectWindowList list;
    MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
    if (wmobj) {
        list = wmobj->getDesktopWinList();
    }

    int index = 0;
    for (int i = list.size() - 1; i >= 0; i--) {
        if (w == list[i]) {
            index = i + 1;
            if (index >= list.size()) {
                target = list[0];
            } else {
                target = list[index];
            }
            break;
        }
    }
    return target;
}

EffectWindow *MultitaskViewEffect::getHomeOrEndWindow(bool flag)
{
    EffectWindow *target = nullptr;
    EffectWindowList list;
    MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
    if (wmobj) {
        list = wmobj->getDesktopWinList();
    }

    if (list.size() <= 0)
        return nullptr;

    if (flag) {
        target = list[list.size() - 1];
    } else {
        target = list[0];
    }
    return target;
}

EffectWindow *MultitaskViewEffect::getNextSameTypeWindow(EffectWindow *w)
{
    EffectWindow *target = nullptr;
    EffectWindowList list;
    MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
    if (wmobj) {
        list = wmobj->getDesktopWinList();
    }

    EffectWindowList sameList;
    int index = 0;
    for (int i = list.size() - 1; i >= 0; i--) {
        if (w->windowClass() == list[i]->windowClass()) {
            sameList.push_back(list[i]);
        }
    }
    if (sameList.size() == 1) {
        target = w;
    } else {
        int index = 0;
        for (int j = 0; j < sameList.size(); j++) {
            if (w == sameList[j]) {
                index = j + 1;
                if (index >= sameList.size()) {
                    target = sameList[0];
                } else {
                    target = sameList[index];
                }
                break;
            }
        }
    }

    return target;
}

EffectWindow *MultitaskViewEffect::getPreSameTypeWindow(EffectWindow *w)
{
    EffectWindow *target = nullptr;
    EffectWindowList list;
    MultiViewWinManager *wmobj = getWinManagerObject(effects->currentDesktop() - 1);
    if (wmobj) {
        list = wmobj->getDesktopWinList();
    }

    EffectWindowList sameList;
    int index = 0;
    for (int i = list.size() - 1; i >= 0; i--) {
        if (w->windowClass() == list[i]->windowClass()) {
            sameList.push_back(list[i]);
        }
    }
    if (sameList.size() == 1) {
        target = w;
    } else {
        int index = 0;
        for (int j = 0; j < sameList.size(); j++) {
            if (w == sameList[j]) {
                index = j - 1;
                if (index < 0) {
                    target = sameList[sameList.size() - 1];
                } else {
                    target = sameList[index];
                }
                break;
            }
        }
    }

    return target;
}

MultiViewWorkspace *MultitaskViewEffect::getWorkspaceObject(EffectScreen *screen, int secindex)
{
    MultiViewWorkspace *target = nullptr;
    if (m_workspaceBackgrounds.contains(screen) && secindex >= 0) {
        if (m_workspaceBackgrounds[screen].size() > secindex) {
            target = m_workspaceBackgrounds[screen][secindex];
        }
    }
    return target;
}

MultiViewWinManager *MultitaskViewEffect::getWinManagerObject(int index)
{
    MultiViewWinManager *target = nullptr;
    if (m_motionManagers.size() > index && index >= 0) {
        target = m_motionManagers[index];
    }
    return target;
}

MultiViewWinManager *MultitaskViewEffect::getWorkspaceWinManagerObject(int index)
{
    MultiViewWinManager *target = nullptr;
    if (m_workspaceWinMgr.size() > index && index >= 0) {
        target = m_workspaceWinMgr[index];
    }
    return target;
}

bool MultitaskViewEffect::checkHandlerWorkspace(QPoint pos, EffectScreen *screen, int &desktop)
{
    bool isChecked = false;
    QList<MultiViewWorkspace *> list = m_workspaceBackgrounds[screen];
    for (int d = 0; d < list.size(); d++) {
        QRect wrect = list[d]->getRect().adjusted(-1, -1, 4, 4);
        QPoint p(list[d]->getRect().topRight().x() - 14 * effectsEx->getOsScale(), list[d]->getRect().y() - 9 * effectsEx->getOsScale());
        QRect brect(p, m_closeWorkspaceFrame->geometry().size());
        if (wrect.contains(pos) || brect.contains(pos)) {
            desktop = d + 1;
            isChecked = true;
            break;
        }
    }

    return isChecked;
}

bool MultitaskViewEffect::touchDown(qint32 id, const QPointF &pos, std::chrono::microseconds time)
{
    Q_UNUSED(time)

    if (!m_activated) {
        return false;
    }
    // only if we don't track a touch id yet
    if (!m_touch.active) {
        m_touch.active = true;
        m_touch.id = id;
        m_touch.pos = pos.toPoint();
        QMouseEvent event(QEvent::MouseButtonPress, pos, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        windowInputMouseEvent(&event);
        m_touch.isPress = true;
    }
    return true;
}

bool MultitaskViewEffect::touchMotion(qint32 id, const QPointF &pos, std::chrono::microseconds time)
{
    Q_UNUSED(id)
    Q_UNUSED(time)

    if (!m_activated) {
        return false;
    }
    if (m_touch.active && m_touch.id == id) {
        // only update for the touch id we track
        QMouseEvent event(QEvent::MouseMove, pos, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        m_touch.isMotion = true;
        m_touch.pos = pos.toPoint();
        windowInputMouseEvent(&event);
        m_touch.isMotion = false;
    }
    return true;
}

bool MultitaskViewEffect::touchUp(qint32 id, std::chrono::microseconds time)
{
    Q_UNUSED(id)
    Q_UNUSED(time)

    if (!m_activated) {
        return false;
    }
    if (m_touch.active && m_touch.id == id) {
        m_touch.active = false;
        m_touch.id = 0;
        m_touch.isPress = false;
        QMouseEvent event(QEvent::MouseButtonRelease, m_touch.pos, m_touch.pos, Qt::LeftButton, Qt::LeftButton ,Qt::NoModifier);
        windowInputMouseEvent(&event);
    }
    return true;
}

void MultitaskViewEffect::motionRepeat()
{
    m_longPressTouch = true;
}

} // namespace KWin
