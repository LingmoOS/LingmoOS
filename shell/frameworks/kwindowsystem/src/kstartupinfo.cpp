/*
    SPDX-FileCopyrightText: 2001-2003 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: MIT
*/

// qDebug() can't be turned off in kdeinit
#if 0
#define KSTARTUPINFO_ALL_DEBUG
#ifdef __GNUC__
#warning Extra KStartupInfo debug messages enabled.
#endif
#endif

#ifdef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_FROM_ASCII
#endif

#include "kstartupinfo.h"
#include "kwindowsystem_debug.h"

#include <QDateTime>

// need to resolve INT32(qglobal.h)<>INT32(Xlibint.h) conflict
#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <QTimer>
#include <netwm.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
#include <X11/Xlib.h>
#include <fixx11h.h>
#include <kwindowsystem.h>
#include <kx11extras.h>
#include <kxmessages.h>
#include <private/qtx11extras_p.h>
#include <signal.h>

static const char NET_STARTUP_MSG[] = "_NET_STARTUP_INFO";

// DESKTOP_STARTUP_ID is used also in kinit/wrapper.c ,
// kdesu in both kdelibs and kdebase and who knows where else
static const char NET_STARTUP_ENV[] = "DESKTOP_STARTUP_ID";

static QByteArray s_startup_id;

static long get_num(const QString &item_P);
static QString get_str(const QString &item_P);
static QByteArray get_cstr(const QString &item_P);
static QStringList get_fields(const QString &txt_P);
static QString escape_str(const QString &str_P);

class Q_DECL_HIDDEN KStartupInfo::Data : public KStartupInfoData
{
public:
    Data()
        : age(0)
    {
    } // just because it's in a QMap
    Data(const QString &txt_P)
        : KStartupInfoData(txt_P)
        , age(0)
    {
    }
    unsigned int age;
};

struct Q_DECL_HIDDEN KStartupInfoId::Private {
    Private()
        : id("")
    {
    }

    QString to_text() const;

    QByteArray id; // id
};

struct Q_DECL_HIDDEN KStartupInfoData::Private {
    Private()
        : desktop(0)
        , wmclass("")
        , hostname("")
        , silent(KStartupInfoData::Unknown)
        , screen(-1)
        , xinerama(-1)
    {
    }

    QString to_text() const;
    void remove_pid(pid_t pid);

    QString bin;
    QString name;
    QString description;
    QString icon;
    int desktop;
    QList<pid_t> pids;
    QByteArray wmclass;
    QByteArray hostname;
    KStartupInfoData::TriState silent;
    int screen;
    int xinerama;
    QString application_id;
};

class Q_DECL_HIDDEN KStartupInfo::Private
{
public:
    // private slots
    void startups_cleanup();
    void startups_cleanup_no_age();
    void got_message(const QString &msg);
    void window_added(WId w);
    void slot_window_added(WId w);

    void init(int flags);
    void got_startup_info(const QString &msg_P, bool update_only_P);
    void got_remove_startup_info(const QString &msg_P);
    void new_startup_info_internal(const KStartupInfoId &id_P, Data &data_P, bool update_only_P);
    void removeAllStartupInfoInternal(const KStartupInfoId &id_P);
    /**
     * Emits the gotRemoveStartup signal and erases the @p it from the startups map.
     * @returns Iterator to next item in the startups map.
     **/
    QMap<KStartupInfoId, Data>::iterator removeStartupInfoInternal(QMap<KStartupInfoId, Data>::iterator it);
    void remove_startup_pids(const KStartupInfoId &id, const KStartupInfoData &data);
    void remove_startup_pids(const KStartupInfoData &data);
    startup_t check_startup_internal(WId w, KStartupInfoId *id, KStartupInfoData *data);
    bool find_id(const QByteArray &id_P, KStartupInfoId *id_O, KStartupInfoData *data_O);
    bool find_pid(pid_t pid_P, const QByteArray &hostname, KStartupInfoId *id_O, KStartupInfoData *data_O);
    bool find_wclass(const QByteArray &res_name_P, const QByteArray &res_class_P, KStartupInfoId *id_O, KStartupInfoData *data_O);
    void startups_cleanup_internal(bool age_P);
    void clean_all_noncompliant();
    static QString check_required_startup_fields(const QString &msg, const KStartupInfoData &data, int screen);
    static void setWindowStartupId(WId w_P, const QByteArray &id_P);

    KStartupInfo *q;
    unsigned int timeout;
    QMap<KStartupInfoId, KStartupInfo::Data> startups;
    // contains silenced ASN's only if !AnnounceSilencedChanges
    QMap<KStartupInfoId, KStartupInfo::Data> silent_startups;
    // contains ASN's that had change: but no new: yet
    QMap<KStartupInfoId, KStartupInfo::Data> uninited_startups;
    KXMessages msgs;
    QTimer *cleanup;
    int flags;

    Private(int flags_P, KStartupInfo *qq)
        : q(qq)
        , timeout(60)
        , msgs(NET_STARTUP_MSG)
        , cleanup(nullptr)
        , flags(flags_P)
    {
    }

    void createConnections()
    {
        // d == nullptr means "disabled"
        if (!QX11Info::isPlatformX11() || !QX11Info::display()) {
            return;
        }

        if (!(flags & DisableKWinModule)) {
            QObject::connect(KX11Extras::self(), SIGNAL(windowAdded(WId)), q, SLOT(slot_window_added(WId)));
        }
        QObject::connect(&msgs, SIGNAL(gotMessage(QString)), q, SLOT(got_message(QString)));
        cleanup = new QTimer(q);
        QObject::connect(cleanup, SIGNAL(timeout()), q, SLOT(startups_cleanup()));
    }
};

KStartupInfo::KStartupInfo(int flags_P, QObject *parent_P)
    : QObject(parent_P)
    , d(new Private(flags_P, this))
{
    d->createConnections();
}

KStartupInfo::~KStartupInfo()
{
    delete d;
}

void KStartupInfo::Private::got_message(const QString &msg_P)
{
    // TODO do something with SCREEN= ?
    // qCDebug(LOG_KWINDOWSYSTEM) << "got:" << msg_P;
    QString msg = msg_P.trimmed();
    if (msg.startsWith(QLatin1String("new:"))) { // must match length below
        got_startup_info(msg.mid(4), false);
    } else if (msg.startsWith(QLatin1String("change:"))) { // must match length below
        got_startup_info(msg.mid(7), true);
    } else if (msg.startsWith(QLatin1String("remove:"))) { // must match length below
        got_remove_startup_info(msg.mid(7));
    }
}

// if the application stops responding for a while, KWindowSystem may get
// the information about the already mapped window before KXMessages
// actually gets the info about the started application (depends
// on their order in the native x11 event filter)
// simply delay info from KWindowSystem a bit
// SELI???
namespace
{
class DelayedWindowEvent : public QEvent
{
public:
    DelayedWindowEvent(WId w_P)
        : QEvent(uniqueType())
        , w(w_P)
    {
    }
    Window w;
    static Type uniqueType()
    {
        return Type(QEvent::User + 15);
    }
};
}

void KStartupInfo::Private::slot_window_added(WId w_P)
{
    qApp->postEvent(q, new DelayedWindowEvent(w_P));
}

void KStartupInfo::customEvent(QEvent *e_P)
{
    if (e_P->type() == DelayedWindowEvent::uniqueType()) {
        d->window_added(static_cast<DelayedWindowEvent *>(e_P)->w);
    } else
        QObject::customEvent(e_P);
}

void KStartupInfo::Private::window_added(WId w_P)
{
    KStartupInfoId id;
    KStartupInfoData data;
    startup_t ret = check_startup_internal(w_P, &id, &data);
    switch (ret) {
    case Match:
        // qCDebug(LOG_KWINDOWSYSTEM) << "new window match";
        break;
    case NoMatch:
        break; // nothing
    case CantDetect:
        if (flags & CleanOnCantDetect) {
            clean_all_noncompliant();
        }
        break;
    }
}

void KStartupInfo::Private::got_startup_info(const QString &msg_P, bool update_P)
{
    KStartupInfoId id(msg_P);
    if (id.isNull()) {
        return;
    }
    KStartupInfo::Data data(msg_P);
    new_startup_info_internal(id, data, update_P);
}

void KStartupInfo::Private::new_startup_info_internal(const KStartupInfoId &id_P, KStartupInfo::Data &data_P, bool update_P)
{
    if (id_P.isNull()) {
        return;
    }
    if (startups.contains(id_P)) {
        // already reported, update
        startups[id_P].update(data_P);
        startups[id_P].age = 0; // CHECKME
        // qCDebug(LOG_KWINDOWSYSTEM) << "updating";
        if (startups[id_P].silent() == KStartupInfo::Data::Yes && !(flags & AnnounceSilenceChanges)) {
            silent_startups[id_P] = startups[id_P];
            startups.remove(id_P);
            Q_EMIT q->gotRemoveStartup(id_P, silent_startups[id_P]);
            return;
        }
        Q_EMIT q->gotStartupChange(id_P, startups[id_P]);
        return;
    }
    if (silent_startups.contains(id_P)) {
        // already reported, update
        silent_startups[id_P].update(data_P);
        silent_startups[id_P].age = 0; // CHECKME
        // qCDebug(LOG_KWINDOWSYSTEM) << "updating silenced";
        if (silent_startups[id_P].silent() != Data::Yes) {
            startups[id_P] = silent_startups[id_P];
            silent_startups.remove(id_P);
            q->Q_EMIT gotNewStartup(id_P, startups[id_P]);
            return;
        }
        Q_EMIT q->gotStartupChange(id_P, silent_startups[id_P]);
        return;
    }
    if (uninited_startups.contains(id_P)) {
        uninited_startups[id_P].update(data_P);
        // qCDebug(LOG_KWINDOWSYSTEM) << "updating uninited";
        if (!update_P) { // uninited finally got new:
            startups[id_P] = uninited_startups[id_P];
            uninited_startups.remove(id_P);
            Q_EMIT q->gotNewStartup(id_P, startups[id_P]);
            return;
        }
        // no change announce, it's still uninited
        return;
    }
    if (update_P) { // change: without any new: first
        // qCDebug(LOG_KWINDOWSYSTEM) << "adding uninited";
        uninited_startups.insert(id_P, data_P);
    } else if (data_P.silent() != Data::Yes || flags & AnnounceSilenceChanges) {
        // qCDebug(LOG_KWINDOWSYSTEM) << "adding";
        startups.insert(id_P, data_P);
        Q_EMIT q->gotNewStartup(id_P, data_P);
    } else { // new silenced, and silent shouldn't be announced
        // qCDebug(LOG_KWINDOWSYSTEM) << "adding silent";
        silent_startups.insert(id_P, data_P);
    }
    cleanup->start(1000); // 1 sec
}

void KStartupInfo::Private::got_remove_startup_info(const QString &msg_P)
{
    KStartupInfoId id(msg_P);
    KStartupInfoData data(msg_P);
    if (!data.pids().isEmpty()) {
        if (!id.isNull()) {
            remove_startup_pids(id, data);
        } else {
            remove_startup_pids(data);
        }
        return;
    }
    removeAllStartupInfoInternal(id);
}

void KStartupInfo::Private::removeAllStartupInfoInternal(const KStartupInfoId &id_P)
{
    auto it = startups.find(id_P);
    if (it != startups.end()) {
        // qCDebug(LOG_KWINDOWSYSTEM) << "removing";
        Q_EMIT q->gotRemoveStartup(it.key(), it.value());
        startups.erase(it);
        return;
    }
    it = silent_startups.find(id_P);
    if (it != silent_startups.end()) {
        silent_startups.erase(it);
        return;
    }
    it = uninited_startups.find(id_P);
    if (it != uninited_startups.end()) {
        uninited_startups.erase(it);
    }
}

QMap<KStartupInfoId, KStartupInfo::Data>::iterator KStartupInfo::Private::removeStartupInfoInternal(QMap<KStartupInfoId, Data>::iterator it)
{
    Q_EMIT q->gotRemoveStartup(it.key(), it.value());
    return startups.erase(it);
}

void KStartupInfo::Private::remove_startup_pids(const KStartupInfoData &data_P)
{
    // first find the matching info
    for (QMap<KStartupInfoId, KStartupInfo::Data>::Iterator it = startups.begin(); it != startups.end(); ++it) {
        if ((*it).hostname() != data_P.hostname()) {
            continue;
        }
        if (!(*it).is_pid(data_P.pids().first())) {
            continue; // not the matching info
        }
        remove_startup_pids(it.key(), data_P);
        break;
    }
}

void KStartupInfo::Private::remove_startup_pids(const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    if (data_P.pids().isEmpty()) {
        qFatal("data_P.pids().isEmpty()");
    }
    Data *data = nullptr;
    if (startups.contains(id_P)) {
        data = &startups[id_P];
    } else if (silent_startups.contains(id_P)) {
        data = &silent_startups[id_P];
    } else if (uninited_startups.contains(id_P)) {
        data = &uninited_startups[id_P];
    } else {
        return;
    }
    const auto pids = data_P.pids();
    for (auto pid : pids) {
        data->d->remove_pid(pid); // remove all pids from the info
    }
    if (data->pids().isEmpty()) { // all pids removed -> remove info
        removeAllStartupInfoInternal(id_P);
    }
}

bool KStartupInfo::sendStartup(const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    if (id_P.isNull()) {
        return false;
    }
    return sendStartupXcb(QX11Info::connection(), QX11Info::appScreen(), id_P, data_P);
}

bool KStartupInfo::sendStartupXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    if (id_P.isNull()) {
        return false;
    }
    QString msg = QStringLiteral("new: %1 %2").arg(id_P.d->to_text(), data_P.d->to_text());
    msg = Private::check_required_startup_fields(msg, data_P, screen);
#ifdef KSTARTUPINFO_ALL_DEBUG
    qCDebug(LOG_KWINDOWSYSTEM) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX(conn, NET_STARTUP_MSG, msg, screen);
}

QString KStartupInfo::Private::check_required_startup_fields(const QString &msg, const KStartupInfoData &data_P, int screen)
{
    QString ret = msg;
    if (data_P.name().isEmpty()) {
        //        qWarning() << "NAME not specified in initial startup message";
        QString name = data_P.bin();
        if (name.isEmpty()) {
            name = QStringLiteral("UNKNOWN");
        }
        ret += QStringLiteral(" NAME=\"%1\"").arg(escape_str(name));
    }
    if (data_P.screen() == -1) { // add automatically if needed
        ret += QStringLiteral(" SCREEN=%1").arg(screen);
    }
    return ret;
}

bool KStartupInfo::sendChange(const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    if (id_P.isNull()) {
        return false;
    }
    return sendChangeXcb(QX11Info::connection(), QX11Info::appScreen(), id_P, data_P);
}

bool KStartupInfo::sendChangeXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    if (id_P.isNull()) {
        return false;
    }
    QString msg = QStringLiteral("change: %1 %2").arg(id_P.d->to_text(), data_P.d->to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    qCDebug(LOG_KWINDOWSYSTEM) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX(conn, NET_STARTUP_MSG, msg, screen);
}

bool KStartupInfo::sendFinish(const KStartupInfoId &id_P)
{
    if (id_P.isNull()) {
        return false;
    }
    return sendFinishXcb(QX11Info::connection(), QX11Info::appScreen(), id_P);
}

bool KStartupInfo::sendFinishXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id_P)
{
    if (id_P.isNull()) {
        return false;
    }
    QString msg = QStringLiteral("remove: %1").arg(id_P.d->to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    qCDebug(LOG_KWINDOWSYSTEM) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX(conn, NET_STARTUP_MSG, msg, screen);
}

bool KStartupInfo::sendFinish(const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    //    if( id_P.isNull()) // id may be null, the pids and hostname matter then
    //        return false;
    return sendFinishXcb(QX11Info::connection(), QX11Info::appScreen(), id_P, data_P);
}

bool KStartupInfo::sendFinishXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id_P, const KStartupInfoData &data_P)
{
    //    if( id_P.isNull()) // id may be null, the pids and hostname matter then
    //        return false;
    QString msg = QStringLiteral("remove: %1 %2").arg(id_P.d->to_text(), data_P.d->to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    qCDebug(LOG_KWINDOWSYSTEM) << "sending " << msg;
#endif
    return KXMessages::broadcastMessageX(conn, NET_STARTUP_MSG, msg, screen);
}

void KStartupInfo::appStarted()
{
    QByteArray startupId = s_startup_id;

    if (startupId.isEmpty()) {
        startupId = QX11Info::nextStartupId();
    }

    appStarted(startupId);
    setStartupId("0"); // reset the id, no longer valid (must use clearStartupId() to avoid infinite loop)
}

void KStartupInfo::appStarted(const QByteArray &startup_id)
{
    KStartupInfoId id;
    id.initId(startup_id);
    if (id.isNull()) {
        return;
    }
    if (QX11Info::isPlatformX11() && !qEnvironmentVariableIsEmpty("DISPLAY")) { // don't rely on QX11Info::display()
        KStartupInfo::sendFinish(id);
    }
}

void KStartupInfo::setStartupId(const QByteArray &startup_id)
{
    if (startup_id == s_startup_id) {
        return;
    }
    if (startup_id.isEmpty()) {
        s_startup_id = "0";
    } else {
        s_startup_id = startup_id;
        if (QX11Info::isPlatformX11()) {
            KStartupInfoId id;
            id.initId(startup_id);
            long timestamp = id.timestamp();
            if (timestamp != 0) {
                if (QX11Info::appUserTime() == 0 || NET::timestampCompare(timestamp, QX11Info::appUserTime()) > 0) { // time > appUserTime
                    QX11Info::setAppUserTime(timestamp);
                }
                if (QX11Info::appTime() == 0 || NET::timestampCompare(timestamp, QX11Info::appTime()) > 0) { // time > appTime
                    QX11Info::setAppTime(timestamp);
                }
            }
        }
    }
}

void KStartupInfo::setNewStartupId(QWindow *window, const QByteArray &startup_id)
{
    Q_ASSERT(window);
    setStartupId(startup_id);
    bool activate = true;
    if (window != nullptr && QX11Info::isPlatformX11()) {
        if (!startup_id.isEmpty() && startup_id != "0") {
            NETRootInfo i(QX11Info::connection(), NET::Supported);
            if (i.isSupported(NET::WM2StartupId)) {
                KStartupInfo::Private::setWindowStartupId(window->winId(), startup_id);
                activate = false; // WM will take care of it
            }
        }
        if (activate) {
            KX11Extras::setOnDesktop(window->winId(), KX11Extras::currentDesktop());
            // This is not very nice, but there's no way how to get any
            // usable timestamp without ASN, so force activating the window.
            // And even with ASN, it's not possible to get the timestamp here,
            // so if the WM doesn't have support for ASN, it can't be used either.
            KX11Extras::forceActiveWindow(window->winId());
        }
    }
}

KStartupInfo::startup_t KStartupInfo::checkStartup(WId w_P, KStartupInfoId &id_O, KStartupInfoData &data_O)
{
    return d->check_startup_internal(w_P, &id_O, &data_O);
}

KStartupInfo::startup_t KStartupInfo::checkStartup(WId w_P, KStartupInfoId &id_O)
{
    return d->check_startup_internal(w_P, &id_O, nullptr);
}

KStartupInfo::startup_t KStartupInfo::checkStartup(WId w_P, KStartupInfoData &data_O)
{
    return d->check_startup_internal(w_P, nullptr, &data_O);
}

KStartupInfo::startup_t KStartupInfo::checkStartup(WId w_P)
{
    return d->check_startup_internal(w_P, nullptr, nullptr);
}

KStartupInfo::startup_t KStartupInfo::Private::check_startup_internal(WId w_P, KStartupInfoId *id_O, KStartupInfoData *data_O)
{
    if (startups.isEmpty()) {
        return NoMatch; // no startups
    }
    // Strategy:
    //
    // Is this a compliant app ?
    //  - Yes - test for match
    //  - No - Is this a NET_WM compliant app ?
    //           - Yes - test for pid match
    //           - No - test for WM_CLASS match
    qCDebug(LOG_KWINDOWSYSTEM) << "check_startup";
    QByteArray id = windowStartupId(w_P);
    if (!id.isNull()) {
        if (id.isEmpty() || id == "0") { // means ignore this window
            qCDebug(LOG_KWINDOWSYSTEM) << "ignore";
            return NoMatch;
        }
        return find_id(id, id_O, data_O) ? Match : NoMatch;
    }
    if (!QX11Info::isPlatformX11()) {
        qCDebug(LOG_KWINDOWSYSTEM) << "check_startup:cantdetect";
        return CantDetect;
    }
    NETWinInfo info(QX11Info::connection(),
                    w_P,
                    QX11Info::appRootWindow(),
                    NET::WMWindowType | NET::WMPid | NET::WMState,
                    NET::WM2WindowClass | NET::WM2ClientMachine | NET::WM2TransientFor);
    pid_t pid = info.pid();
    if (pid > 0) {
        QByteArray hostname = info.clientMachine();
        if (!hostname.isEmpty() && find_pid(pid, hostname, id_O, data_O)) {
            return Match;
        }
        // try XClass matching , this PID stuff sucks :(
    }
    if (find_wclass(info.windowClassName(), info.windowClassClass(), id_O, data_O)) {
        return Match;
    }
    // ignore NET::Tool and other special window types, if they can't be matched
    NET::WindowType type = info.windowType(NET::NormalMask | NET::DesktopMask | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
                                           | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask);
    if (type != NET::Normal && type != NET::Override && type != NET::Unknown && type != NET::Dialog && type != NET::Utility)
    //        && type != NET::Dock ) why did I put this here?
    {
        return NoMatch;
    }
    // lets see if this is a transient
    xcb_window_t transient_for = info.transientFor();
    if (transient_for != QX11Info::appRootWindow() && transient_for != XCB_WINDOW_NONE) {
        return NoMatch;
    }
    qCDebug(LOG_KWINDOWSYSTEM) << "check_startup:cantdetect";
    return CantDetect;
}

bool KStartupInfo::Private::find_id(const QByteArray &id_P, KStartupInfoId *id_O, KStartupInfoData *data_O)
{
    // qCDebug(LOG_KWINDOWSYSTEM) << "find_id:" << id_P;
    KStartupInfoId id;
    id.initId(id_P);
    if (startups.contains(id)) {
        if (id_O != nullptr) {
            *id_O = id;
        }
        if (data_O != nullptr) {
            *data_O = startups[id];
        }
        // qCDebug(LOG_KWINDOWSYSTEM) << "check_startup_id:match";
        return true;
    }
    return false;
}

bool KStartupInfo::Private::find_pid(pid_t pid_P, const QByteArray &hostname_P, KStartupInfoId *id_O, KStartupInfoData *data_O)
{
    // qCDebug(LOG_KWINDOWSYSTEM) << "find_pid:" << pid_P;
    for (QMap<KStartupInfoId, KStartupInfo::Data>::Iterator it = startups.begin(); it != startups.end(); ++it) {
        if ((*it).is_pid(pid_P) && (*it).hostname() == hostname_P) {
            // Found it !
            if (id_O != nullptr) {
                *id_O = it.key();
            }
            if (data_O != nullptr) {
                *data_O = *it;
            }
            // non-compliant, remove on first match
            removeStartupInfoInternal(it);
            // qCDebug(LOG_KWINDOWSYSTEM) << "check_startup_pid:match";
            return true;
        }
    }
    return false;
}

bool KStartupInfo::Private::find_wclass(const QByteArray &_res_name, const QByteArray &_res_class, KStartupInfoId *id_O, KStartupInfoData *data_O)
{
    QByteArray res_name = _res_name.toLower();
    QByteArray res_class = _res_class.toLower();
    // qCDebug(LOG_KWINDOWSYSTEM) << "find_wclass:" << res_name << ":" << res_class;
    for (QMap<KStartupInfoId, Data>::Iterator it = startups.begin(); it != startups.end(); ++it) {
        const QByteArray wmclass = (*it).findWMClass();
        if (wmclass.toLower() == res_name || wmclass.toLower() == res_class) {
            // Found it !
            if (id_O != nullptr) {
                *id_O = it.key();
            }
            if (data_O != nullptr) {
                *data_O = *it;
            }
            // non-compliant, remove on first match
            removeStartupInfoInternal(it);
            // qCDebug(LOG_KWINDOWSYSTEM) << "check_startup_wclass:match";
            return true;
        }
    }
    return false;
}

QByteArray KStartupInfo::windowStartupId(WId w_P)
{
    if (!QX11Info::isPlatformX11()) {
        return QByteArray();
    }
    NETWinInfo info(QX11Info::connection(), w_P, QX11Info::appRootWindow(), NET::Properties(), NET::WM2StartupId | NET::WM2GroupLeader);
    QByteArray ret = info.startupId();
    if (ret.isEmpty() && info.groupLeader() != XCB_WINDOW_NONE) {
        // retry with window group leader, as the spec says
        NETWinInfo groupLeaderInfo(QX11Info::connection(), info.groupLeader(), QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
        ret = groupLeaderInfo.startupId();
    }
    return ret;
}

void KStartupInfo::Private::setWindowStartupId(WId w_P, const QByteArray &id_P)
{
    if (!QX11Info::isPlatformX11()) {
        return;
    }
    if (id_P.isNull()) {
        return;
    }
    NETWinInfo info(QX11Info::connection(), w_P, QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    info.setStartupId(id_P.constData());
}

void KStartupInfo::setTimeout(unsigned int secs_P)
{
    d->timeout = secs_P;
    // schedule removing entries that are older than the new timeout
    QTimer::singleShot(0, this, SLOT(startups_cleanup_no_age()));
}

void KStartupInfo::Private::startups_cleanup_no_age()
{
    startups_cleanup_internal(false);
}

void KStartupInfo::Private::startups_cleanup()
{
    if (startups.isEmpty() && silent_startups.isEmpty() && uninited_startups.isEmpty()) {
        cleanup->stop();
        return;
    }
    startups_cleanup_internal(true);
}

void KStartupInfo::Private::startups_cleanup_internal(bool age_P)
{
    auto checkCleanup = [this, age_P](QMap<KStartupInfoId, KStartupInfo::Data> &s, bool doEmit) {
        auto it = s.begin();
        while (it != s.end()) {
            if (age_P) {
                (*it).age++;
            }
            unsigned int tout = timeout;
            if ((*it).silent() == KStartupInfo::Data::Yes) {
                // give kdesu time to get a password
                tout *= 20;
            }
            const QByteArray timeoutEnvVariable = qgetenv("KSTARTUPINFO_TIMEOUT");
            if (!timeoutEnvVariable.isNull()) {
                tout = timeoutEnvVariable.toUInt();
            }
            if ((*it).age >= tout) {
                if (doEmit) {
                    Q_EMIT q->gotRemoveStartup(it.key(), it.value());
                }
                it = s.erase(it);
            } else {
                ++it;
            }
        }
    };
    checkCleanup(startups, true);
    checkCleanup(silent_startups, false);
    checkCleanup(uninited_startups, false);
}

void KStartupInfo::Private::clean_all_noncompliant()
{
    for (QMap<KStartupInfoId, KStartupInfo::Data>::Iterator it = startups.begin(); it != startups.end();) {
        if ((*it).WMClass() != "0") {
            ++it;
            continue;
        }
        it = removeStartupInfoInternal(it);
    }
}

QByteArray KStartupInfo::createNewStartupId()
{
    quint32 timestamp = 0;
    if (QX11Info::isPlatformX11()) {
        timestamp = QX11Info::getTimestamp();
    }
    return KStartupInfo::createNewStartupIdForTimestamp(timestamp);
}

QByteArray KStartupInfo::createNewStartupIdForTimestamp(quint32 timestamp)
{
    // Assign a unique id, use hostname+time+pid, that should be 200% unique.
    // Also append the user timestamp (for focus stealing prevention).
    struct timeval tm;
    gettimeofday(&tm, nullptr);
    char hostname[256];
    hostname[0] = '\0';
    if (!gethostname(hostname, 255)) {
        hostname[sizeof(hostname) - 1] = '\0';
    }
    QByteArray id = QStringLiteral("%1;%2;%3;%4_TIME%5").arg(hostname).arg(tm.tv_sec).arg(tm.tv_usec).arg(getpid()).arg(timestamp).toUtf8();
    // qCDebug(LOG_KWINDOWSYSTEM) << "creating: " << id << ":" << (qApp ? qAppName() : QString("unnamed app") /* e.g. kdeinit */);
    return id;
}

const QByteArray &KStartupInfoId::id() const
{
    return d->id;
}

QString KStartupInfoId::Private::to_text() const
{
    return QStringLiteral(" ID=\"%1\" ").arg(escape_str(id));
}

KStartupInfoId::KStartupInfoId(const QString &txt_P)
    : d(new Private)
{
    const QStringList items = get_fields(txt_P);
    for (QStringList::ConstIterator it = items.begin(); it != items.end(); ++it) {
        if ((*it).startsWith(QLatin1String("ID="))) {
            d->id = get_cstr(*it);
        }
    }
}

void KStartupInfoId::initId(const QByteArray &id_P)
{
    if (!id_P.isEmpty()) {
        d->id = id_P;
#ifdef KSTARTUPINFO_ALL_DEBUG
        qCDebug(LOG_KWINDOWSYSTEM) << "using: " << d->id;
#endif
        return;
    }
    const QByteArray startup_env = qgetenv(NET_STARTUP_ENV);
    if (!startup_env.isEmpty()) {
        // already has id
        d->id = startup_env;
#ifdef KSTARTUPINFO_ALL_DEBUG
        qCDebug(LOG_KWINDOWSYSTEM) << "reusing: " << d->id;
#endif
        return;
    }
    d->id = KStartupInfo::createNewStartupId();
}

bool KStartupInfoId::setupStartupEnv() const
{
    if (isNull()) {
        qunsetenv(NET_STARTUP_ENV);
        return false;
    }
    return !qputenv(NET_STARTUP_ENV, id()) == 0;
}

void KStartupInfo::resetStartupEnv()
{
    qunsetenv(NET_STARTUP_ENV);
}

KStartupInfoId::KStartupInfoId()
    : d(new Private)
{
}

KStartupInfoId::~KStartupInfoId()
{
    delete d;
}

KStartupInfoId::KStartupInfoId(const KStartupInfoId &id_P)
    : d(new Private(*id_P.d))
{
}

KStartupInfoId &KStartupInfoId::operator=(const KStartupInfoId &id_P)
{
    if (&id_P == this) {
        return *this;
    }
    *d = *id_P.d;
    return *this;
}

bool KStartupInfoId::operator==(const KStartupInfoId &id_P) const
{
    return id() == id_P.id();
}

bool KStartupInfoId::operator!=(const KStartupInfoId &id_P) const
{
    return !(*this == id_P);
}

// needed for QMap
bool KStartupInfoId::operator<(const KStartupInfoId &id_P) const
{
    return id() < id_P.id();
}

bool KStartupInfoId::isNull() const
{
    return d->id.isEmpty() || d->id == "0";
}

unsigned long KStartupInfoId::timestamp() const
{
    if (isNull()) {
        return 0;
    }
    // As per the spec, the ID must contain the _TIME followed by the timestamp
    int pos = d->id.lastIndexOf("_TIME");
    if (pos >= 0) {
        bool ok;
        unsigned long time = QString(d->id.mid(pos + 5)).toULong(&ok);
        if (!ok && d->id[pos + 5] == '-') { // try if it's as a negative signed number perhaps
            time = QString(d->id.mid(pos + 5)).toLong(&ok);
        }
        if (ok) {
            return time;
        }
    }
    return 0;
}

QString KStartupInfoData::Private::to_text() const
{
    QString ret;
    // prepare some space which should be always enough.
    // No need to squeze at the end, as the result is only used as intermediate string
    ret.reserve(256);
    if (!bin.isEmpty()) {
        ret += QStringLiteral(" BIN=\"%1\"").arg(escape_str(bin));
    }
    if (!name.isEmpty()) {
        ret += QStringLiteral(" NAME=\"%1\"").arg(escape_str(name));
    }
    if (!description.isEmpty()) {
        ret += QStringLiteral(" DESCRIPTION=\"%1\"").arg(escape_str(description));
    }
    if (!icon.isEmpty()) {
        ret += QStringLiteral(" ICON=\"%1\"").arg(icon);
    }
    if (desktop != 0) {
        ret += QStringLiteral(" DESKTOP=%1").arg(desktop == NET::OnAllDesktops ? NET::OnAllDesktops : desktop - 1); // spec counts from 0
    }
    if (!wmclass.isEmpty()) {
        ret += QStringLiteral(" WMCLASS=\"%1\"").arg(QString(wmclass));
    }
    if (!hostname.isEmpty()) {
        ret += QStringLiteral(" HOSTNAME=%1").arg(QString(hostname));
    }
    for (QList<pid_t>::ConstIterator it = pids.begin(); it != pids.end(); ++it) {
        ret += QStringLiteral(" PID=%1").arg(*it);
    }
    if (silent != KStartupInfoData::Unknown) {
        ret += QStringLiteral(" SILENT=%1").arg(silent == KStartupInfoData::Yes ? 1 : 0);
    }
    if (screen != -1) {
        ret += QStringLiteral(" SCREEN=%1").arg(screen);
    }
    if (xinerama != -1) {
        ret += QStringLiteral(" XINERAMA=%1").arg(xinerama);
    }
    if (!application_id.isEmpty()) {
        ret += QStringLiteral(" APPLICATION_ID=\"%1\"").arg(application_id);
    }
    return ret;
}

KStartupInfoData::KStartupInfoData(const QString &txt_P)
    : d(new Private)
{
    const QStringList items = get_fields(txt_P);
    for (QStringList::ConstIterator it = items.begin(); it != items.end(); ++it) {
        if ((*it).startsWith(QLatin1String("BIN="))) {
            d->bin = get_str(*it);
        } else if ((*it).startsWith(QLatin1String("NAME="))) {
            d->name = get_str(*it);
        } else if ((*it).startsWith(QLatin1String("DESCRIPTION="))) {
            d->description = get_str(*it);
        } else if ((*it).startsWith(QLatin1String("ICON="))) {
            d->icon = get_str(*it);
        } else if ((*it).startsWith(QLatin1String("DESKTOP="))) {
            d->desktop = get_num(*it);
            if (d->desktop != NET::OnAllDesktops) {
                ++d->desktop; // spec counts from 0
            }
        } else if ((*it).startsWith(QLatin1String("WMCLASS="))) {
            d->wmclass = get_cstr(*it);
        } else if ((*it).startsWith(QLatin1String("HOSTNAME="))) { // added to version 1 (2014)
            d->hostname = get_cstr(*it);
        } else if ((*it).startsWith(QLatin1String("PID="))) { // added to version 1 (2014)
            addPid(get_num(*it));
        } else if ((*it).startsWith(QLatin1String("SILENT="))) {
            d->silent = get_num(*it) != 0 ? Yes : No;
        } else if ((*it).startsWith(QLatin1String("SCREEN="))) {
            d->screen = get_num(*it);
        } else if ((*it).startsWith(QLatin1String("XINERAMA="))) {
            d->xinerama = get_num(*it);
        } else if ((*it).startsWith(QLatin1String("APPLICATION_ID="))) {
            d->application_id = get_str(*it);
        }
    }
}

KStartupInfoData::KStartupInfoData(const KStartupInfoData &data)
    : d(new Private(*data.d))
{
}

KStartupInfoData &KStartupInfoData::operator=(const KStartupInfoData &data)
{
    if (&data == this) {
        return *this;
    }
    *d = *data.d;
    return *this;
}

void KStartupInfoData::update(const KStartupInfoData &data_P)
{
    if (!data_P.bin().isEmpty()) {
        d->bin = data_P.bin();
    }
    if (!data_P.name().isEmpty() && name().isEmpty()) { // don't overwrite
        d->name = data_P.name();
    }
    if (!data_P.description().isEmpty() && description().isEmpty()) { // don't overwrite
        d->description = data_P.description();
    }
    if (!data_P.icon().isEmpty() && icon().isEmpty()) { // don't overwrite
        d->icon = data_P.icon();
    }
    if (data_P.desktop() != 0 && desktop() == 0) { // don't overwrite
        d->desktop = data_P.desktop();
    }
    if (!data_P.d->wmclass.isEmpty()) {
        d->wmclass = data_P.d->wmclass;
    }
    if (!data_P.d->hostname.isEmpty()) {
        d->hostname = data_P.d->hostname;
    }
    for (QList<pid_t>::ConstIterator it = data_P.d->pids.constBegin(); it != data_P.d->pids.constEnd(); ++it) {
        addPid(*it);
    }
    if (data_P.silent() != Unknown) {
        d->silent = data_P.silent();
    }
    if (data_P.screen() != -1) {
        d->screen = data_P.screen();
    }
    if (data_P.xinerama() != -1 && xinerama() != -1) { // don't overwrite
        d->xinerama = data_P.xinerama();
    }
    if (!data_P.applicationId().isEmpty() && applicationId().isEmpty()) { // don't overwrite
        d->application_id = data_P.applicationId();
    }
}

KStartupInfoData::KStartupInfoData()
    : d(new Private)
{
}

KStartupInfoData::~KStartupInfoData()
{
    delete d;
}

void KStartupInfoData::setBin(const QString &bin_P)
{
    d->bin = bin_P;
}

const QString &KStartupInfoData::bin() const
{
    return d->bin;
}

void KStartupInfoData::setName(const QString &name_P)
{
    d->name = name_P;
}

const QString &KStartupInfoData::name() const
{
    return d->name;
}

const QString &KStartupInfoData::findName() const
{
    if (!name().isEmpty()) {
        return name();
    }
    return bin();
}

void KStartupInfoData::setDescription(const QString &desc_P)
{
    d->description = desc_P;
}

const QString &KStartupInfoData::description() const
{
    return d->description;
}

const QString &KStartupInfoData::findDescription() const
{
    if (!description().isEmpty()) {
        return description();
    }
    return name();
}

void KStartupInfoData::setIcon(const QString &icon_P)
{
    d->icon = icon_P;
}

const QString &KStartupInfoData::findIcon() const
{
    if (!icon().isEmpty()) {
        return icon();
    }
    return bin();
}

const QString &KStartupInfoData::icon() const
{
    return d->icon;
}

void KStartupInfoData::setDesktop(int desktop_P)
{
    d->desktop = desktop_P;
}

int KStartupInfoData::desktop() const
{
    return d->desktop;
}

void KStartupInfoData::setWMClass(const QByteArray &wmclass_P)
{
    d->wmclass = wmclass_P;
}

const QByteArray KStartupInfoData::findWMClass() const
{
    if (!WMClass().isEmpty() && WMClass() != "0") {
        return WMClass();
    }
    return bin().toUtf8();
}

QByteArray KStartupInfoData::WMClass() const
{
    return d->wmclass;
}

void KStartupInfoData::setHostname(const QByteArray &hostname_P)
{
    if (!hostname_P.isNull()) {
        d->hostname = hostname_P;
    } else {
        char tmp[256];
        tmp[0] = '\0';
        if (!gethostname(tmp, 255)) {
            tmp[sizeof(tmp) - 1] = '\0';
        }
        d->hostname = tmp;
    }
}

QByteArray KStartupInfoData::hostname() const
{
    return d->hostname;
}

void KStartupInfoData::addPid(pid_t pid_P)
{
    if (!d->pids.contains(pid_P)) {
        d->pids.append(pid_P);
    }
}

void KStartupInfoData::Private::remove_pid(pid_t pid_P)
{
    pids.removeAll(pid_P);
}

QList<pid_t> KStartupInfoData::pids() const
{
    return d->pids;
}

bool KStartupInfoData::is_pid(pid_t pid_P) const
{
    return d->pids.contains(pid_P);
}

void KStartupInfoData::setSilent(TriState state_P)
{
    d->silent = state_P;
}

KStartupInfoData::TriState KStartupInfoData::silent() const
{
    return d->silent;
}

void KStartupInfoData::setScreen(int _screen)
{
    d->screen = _screen;
}

int KStartupInfoData::screen() const
{
    return d->screen;
}

void KStartupInfoData::setXinerama(int xinerama)
{
    d->xinerama = xinerama;
}

int KStartupInfoData::xinerama() const
{
    return d->xinerama;
}

void KStartupInfoData::setApplicationId(const QString &desktop)
{
    if (desktop.startsWith(QLatin1Char('/'))) {
        d->application_id = desktop;
        return;
    }
    // the spec requires this is always a full path, in order for everyone to be able to find it
    QString desk = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, desktop);
    if (desk.isEmpty()) {
        return;
    }
    d->application_id = desk;
}

QString KStartupInfoData::applicationId() const
{
    return d->application_id;
}

static long get_num(const QString &item_P)
{
    unsigned int pos = item_P.indexOf(QLatin1Char('='));
    return item_P.mid(pos + 1).toLong();
}

static QString get_str(const QString &item_P)
{
    int pos = item_P.indexOf(QLatin1Char('='));
    return item_P.mid(pos + 1);
}

static QByteArray get_cstr(const QString &item_P)
{
    return get_str(item_P).toUtf8();
}

static QStringList get_fields(const QString &txt_P)
{
    QString txt = txt_P.simplified();
    QStringList ret;
    QString item;
    bool in = false;
    bool escape = false;
    for (int pos = 0; pos < txt.length(); ++pos) {
        if (escape) {
            item += txt[pos];
            escape = false;
        } else if (txt[pos] == QLatin1Char('\\')) {
            escape = true;
        } else if (txt[pos] == QLatin1Char('\"')) {
            in = !in;
        } else if (txt[pos] == QLatin1Char(' ') && !in) {
            ret.append(item);
            item = QString();
        } else {
            item += txt[pos];
        }
    }
    ret.append(item);
    return ret;
}

static QString escape_str(const QString &str_P)
{
    QString ret;
    // prepare some space which should be always enough.
    // No need to squeze at the end, as the result is only used as intermediate string
    ret.reserve(str_P.size() * 2);
    for (int pos = 0; pos < str_P.length(); ++pos) {
        if (str_P[pos] == QLatin1Char('\\') || str_P[pos] == QLatin1Char('"')) {
            ret += QLatin1Char('\\');
        }
        ret += str_P[pos];
    }
    return ret;
}

#include "moc_kstartupinfo.cpp"
