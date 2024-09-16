// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themes.h"
#include "utils.h"
#include "../common/commondefine.h"
#include "impl/appearancemanager.h"
#include "dbus/appearancedbusproxy.h"

#include <gtk/gtk.h>
#include <xcb/xcb_cursor.h>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

#include <QStandardPaths>

ThemesApi::ThemesApi(AppearanceManager *parent)
    : QObject(parent)
    , scanner(new Scanner())
    , gtk2Mutex(QMutex())
    , gtk3Mutex(QMutex())
    , dbusProxy(parent->getDBusProxy())
{
    if (QGSettings::isSchemaInstalled(XSETTINGSSCHEMA)) {
        xSetting = QSharedPointer<QGSettings>(new QGSettings(XSETTINGSSCHEMA));
    }

    if (QGSettings::isSchemaInstalled(METACITYSCHEMA)) {
        metacitySetting = QSharedPointer<QGSettings>(new QGSettings(METACITYSCHEMA));
    }

    if (QGSettings::isSchemaInstalled(WMSCHEMA)) {
        wmSetting = QSharedPointer<QGSettings>(new QGSettings(WMSCHEMA));
    }

    if (QGSettings::isSchemaInstalled(INTERFACESCHEMA)) {
        interfaceSetting = QSharedPointer<QGSettings>(new QGSettings(INTERFACESCHEMA));
    }
}

ThemesApi::~ThemesApi()
{
}

bool ThemesApi::isThemeInList(QString theme, QVector<QString> list)
{
    int index = theme.lastIndexOf("/");
    QString name = theme.mid(0, index);
    for (auto l : list) {
        index = l.lastIndexOf("/");

        if (name == l.mid(0, index))
            return true;
    }
    return false;
}

QVector<QString> ThemesApi::listGlobalTheme()
{
    QVector<QString> local;
    QDir home = QDir::home();
    local.push_back(QString("%1/dde-appearance/deepin-themes/").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)));
    local.push_back(home.absoluteFilePath(".local/share/deepin-themes"));
    local.push_back(home.absoluteFilePath(".deepin-themes"));

    QVector<QString> sys;
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("deepin-themes");
        if (QFile::exists(path))
            sys.push_back(path);
    }

    return doListTheme(local, sys, TYPEGLOBALTHEME);
}

QVector<QString> ThemesApi::listGtkTheme()
{
    QVector<QString> local;
    QString home = utils::GetUserHomeDir();
    local.push_back(home + "/.local/share/themes");
    local.push_back(home + "/.themes");

    QVector<QString> sys;
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("themes");
        if (QFile::exists(path))
            sys.push_back(path);
    }

    return doListTheme(local, sys, TYPEGTK);
}

QVector<QString> ThemesApi::listIconTheme()
{
    QVector<QString> local;
    QString home = utils::GetUserHomeDir();
    local.push_back(home + "/.local/share/icons");
    local.push_back(home + "/.icons");

    QVector<QString> sys;
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("icons");
        if (QFile::exists(path))
            sys.push_back(path);
    }

    return doListTheme(local, sys, TYPEICON);
}

QVector<QString> ThemesApi::listCursorTheme()
{
    if (qEnvironmentVariableIsSet("XCURSOR_PATH")) {
        const QString xcursor_env = qEnvironmentVariable("XCURSOR_PATH");
        QVector<QString> paths = xcursor_env.split(':', Qt::SkipEmptyParts).toVector();
        return doListTheme(QVector<QString>(), paths, TYPECURSOR);
    }

    QVector<QString> local;
    QString home = utils::GetUserHomeDir();
    local.push_back(home + "/.local/share/icons");
    local.push_back(home + "/.icons");

    QVector<QString> sys;
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath("icons");
        if (QFile::exists(path))
            sys.push_back(path);
    }

    return doListTheme(local, sys, TYPECURSOR);
}

QVector<QString> ThemesApi::doListTheme(QVector<QString> local, QVector<QString> sys, QString type)
{
    QVector<QString> lists = scanThemeDirs(local, type);
    QVector<QString> syslists = scanThemeDirs(sys, type);

    return mergeThemeList(lists, syslists);
}

QVector<QString> ThemesApi::scanThemeDirs(QVector<QString> dirs, QString type)
{
    QVector<QString> lists;
    for (auto dir : dirs) {
        QVector<QString> tmp;
        if (type == TYPEGTK) {
            tmp = scanner->listGtkTheme(dir);
        } else if (type == TYPECURSOR) {
            tmp = scanner->listCursorTheme(dir);
        } else if (type == TYPEICON) {
            tmp = scanner->listIconTheme(dir);
        } else if (type == TYPEGLOBALTHEME) {
            tmp = scanner->listGlobalTheme(dir);
        } else {
            break;
        }

        lists.append(tmp);
    }

    std::sort(lists.begin(), lists.end(), [](const QString &a, const QString &b) {
        return QFileInfo(QFileInfo(a).dir().absolutePath()).lastModified() < QFileInfo(QFileInfo(b).dir().absolutePath()).lastModified();
    });

    return lists;
}

QVector<QString> ThemesApi::mergeThemeList(QVector<QString> src, QVector<QString> target)
{
    if (target.size() == 0) {
        return src;
    }

    for (auto t : target) {
        if (isThemeInList(t, src)) {
            continue;
        }
        src.push_back(t);
    }

    return src;
}

bool ThemesApi::setWMTheme(QString name)
{
    if (metacitySetting) {
        metacitySetting->set("theme", name);
    }

    if (!wmSetting) {
        return false;
    }

    wmSetting->set("theme", name);

    return true;
}

bool ThemesApi::setGlobalTheme(QString name)
{
    if (!scanner->isGlobalTheme(getThemePath(name, TYPEGLOBALTHEME, "deepin-theme"))) {
        qWarning() << "isGlobalTheme failed";
        return false;
    }

    return true;
}

bool ThemesApi::setGtkTheme(QString name)
{
    if (!scanner->isGtkTheme(getThemePath(name, TYPEGTK, "themes"))) {
        qWarning() << "isGtkTheme failed";
        return false;
    }

    setGtk2Theme(name);

    setGtk3Theme(name);

    if (!xSetting) {
        return false;
    }
    QString old = xSetting->get(XSKEYTHEME).toString();
    if (old == name) {
        qWarning() << "getXSettingsValue failed";
        return false;
    }

    xSetting->set(XSKEYTHEME, name);

    if (!setWMTheme(name)) {
        xSetting->set(XSKEYTHEME, old);
        qWarning() << "setWMTheme failed";
        return false;
    }

    if (!setQTTheme()) {
        xSetting->set(XSKEYTHEME, old);
        setWMTheme(old);
        qWarning() << "setQTTheme failed";
        return false;
    }

    return true;
}

bool ThemesApi::setIconTheme(QString name)
{
    if (!scanner->isIconTheme(getThemePath(name, TYPEICON, "icons"))) {
        qWarning() << "isIconTheme failed";
        return false;
    }

    setGtk2Icon(name);

    setGtk3Icon(name);

    if (!xSetting) {
        return false;
    }
    QString old = xSetting->get(XSKEYICONTHEME).toString();
    if (old == name) {
        return false;
    }

    xSetting->set(XSKEYICONTHEME, name);

    return true;
}

bool ThemesApi::setCursorTheme(QString name)
{
    if (!scanner->isCursorTheme(getThemePath(name, TYPECURSOR, "icons"))) {
        qWarning() << "isCursorTheme failed";
        return false;
    }

    setGtk2Cursor(name);

    setGtk3Cursor(name);

    setDefaultCursor(name);

    if (!xSetting) {
        return false;
    }

    QString old = xSetting->get(XSKEYCURSORNAME).toString();

    xSetting->set(XSKEYCURSORNAME, name);

    setQtCursor(name);
    setGtkCursor(name);
    setWMCursor(name);

    return true;
}

QString ThemesApi::getThemePath(QString name, QString ty, QString key)
{
    QVector<QString> dirs;
    QString home = utils::GetUserHomeDir();
    dirs.push_back(home + "/.local/share/" + key);
    dirs.push_back(home + "/." + key);
    for (const QString &basedir : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const QString path = QDir(basedir).filePath(key);
        if (QFile::exists(path))
            dirs.push_back(path);
    }

    for (auto dir : dirs) {
        QString tmp = dir + "/" + name;

        if (!utils::isFileExists(tmp)) {
            continue;
        }

        if (ty == TYPEGTK || ty == TYPEICON || ty == TYPEGLOBALTHEME) {
            return utils::enCodeURI(tmp + "/index.theme", SCHEME_FILE);
        } else if (ty == TYPECURSOR) {
            return utils::enCodeURI(tmp + "/cursor.theme", SCHEME_FILE);
        }
    }

    return "";
}

void ThemesApi::setGtk2Theme(QString name)
{
    setGtk2Prop("gtk-theme-name", "\"" + name + "\"", getGtk2ConfFile());
}

void ThemesApi::setGtk2Icon(QString name)
{
    setGtk2Prop("gtk-icon-theme-name", "\"" + name + "\"", getGtk2ConfFile());
}

void ThemesApi::setGtk2Cursor(QString name)
{
    setGtk2Prop("gtk-cursor-theme-name", "\"" + name + "\"", getGtk2ConfFile());
}

void ThemesApi::setGtk2Prop(QString key, QString value, QString file)
{
    QMutexLocker gtk2MutexLocker(&gtk2Mutex);
    gtk2FileReader(file);
    QString info = getGtk2ConfInfo(key);
    if (info.isEmpty()) {
        addGtk2ConfInfo(key, value);
    } else {
        if (info == value) {
            return;
        }
        addGtk2ConfInfo(key, value);
    }

    gtk2FileWriter(file);
}

void ThemesApi::gtk2FileReader(QString file)
{
    QFile qfile(file);
    if (!qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    while (!qfile.atEnd()) {
        QString line = qfile.readLine();
        if (line.length() == 0) {
            continue;
        }

        QStringList strv = line.split(GTK2CONFDELIM);
        if (strv.size() != 2) {
            continue;
        }

        addGtk2ConfInfo(strv[0], strv[1]);
    }

    qfile.close();
}

QString ThemesApi::getGtk2ConfInfo(QString key)
{
    return gtk2ConfInfos.value(key);
}

void ThemesApi::addGtk2ConfInfo(QString key, QString value)
{
    gtk2ConfInfos[key] = value;
}

void ThemesApi::gtk2FileWriter(QString file)
{
    QStringList content;
    for (auto it = gtk2ConfInfos.cbegin(); it != gtk2ConfInfos.end(); it++) {
        content.append(it.key() + GTK2CONFDELIM + it.value());
    }

    QFile qfile(file);
    if (!qfile.exists()) {
        QDir dir(file.left(file.lastIndexOf("/")));
        dir.mkpath(file.left(file.lastIndexOf("/")));
        qInfo() << "mkpath" << file;
    }

    qfile.open(QIODevice::WriteOnly);
    qfile.write(content.join("\n").toLatin1());
    qfile.close();
}

void ThemesApi::setGtk3Theme(QString name)
{
    setGtk3Prop(GTK3KEYTHEME, name, getGtk3ConfFile());
}

void ThemesApi::setGtk3Icon(QString name)
{
    setGtk3Prop(GTK3KEYICON, name, getGtk3ConfFile());
}

void ThemesApi::setGtk3Cursor(QString name)
{
    setGtk3Prop(GTK3KEYCURSOR, name, getGtk3ConfFile());
}

void ThemesApi::setGtk3Prop(QString key, QString value, QString file)
{
    QMutexLocker gtk3MutexLocker(&gtk3Mutex);
    QFile qfile(file);
    if (!qfile.exists()) {
        QDir dir(file.left(file.lastIndexOf("/")));
        dir.mkpath(file.left(file.lastIndexOf("/")));
    }
    KeyFile keyfile;
    if (!keyfile.loadFile(file)) {
        return;
    }

    if (isGtk3PropEqual(key, value, keyfile)) {
        return;
    }

    doSetGtk3Prop(key, value, file, keyfile);
}

bool ThemesApi::isGtk3PropEqual(QString key, QString value, KeyFile &keyfile)
{
    QString old = keyfile.getStr(GTK3GROUPSETTINGS, key);
    return old == value;
}

void ThemesApi::doSetGtk3Prop(QString key, QString value, QString file, KeyFile &keyfile)
{
    keyfile.setKey(GTK3GROUPSETTINGS, key, value);
    keyfile.saveToFile(file);
}

bool ThemesApi::setQTTheme()
{
    QString config = utils::GetUserConfigDir();
    config += "/Trolltech.conf";
    return setQt4Theme(config);
}

bool ThemesApi::setQt4Theme(QString config)
{
    if (!utils::isFileExists(config)) {
        return false;
    }

    KeyFile keyfile;
    keyfile.loadFile(config);

    QString value = keyfile.getStr("Qt", "style");
    if (value == "GTK+")
        return true;

    if (config.length() == 0)
        return false;

    QFile file(config);
    if (file.exists()) {
        QDir dir(config.left(config.lastIndexOf("/")));
        if (!dir.mkpath(config.left(config.lastIndexOf("/"))))
            return false;
    }

    keyfile.setKey("Qt", "style", "GTK+");

    return keyfile.saveToFile(config);
}

bool ThemesApi::setDefaultCursor(QString name)
{
    QString file = utils::GetUserHomeDir() + "/.icons/default/index.theme";
    if (utils::isFileExists(file)) {
        QDir qdir(file.left(file.lastIndexOf("/")));
        if (!qdir.mkpath(file.left(file.lastIndexOf("/")))) {
            return false;
        }
    }

    KeyFile keyfile;
    keyfile.loadFile(file);

    QString value = keyfile.getStr("Icon Theme", "Inherits");
    if (value == name)
        return true;

    if (file.length() == 0)
        return false;

    QFile qfile(file);
    if (qfile.exists()) {
        QDir dir(file.left(file.lastIndexOf("/")));
        if (!dir.mkpath(file.left(file.lastIndexOf("/"))))
            return false;
    }

    keyfile.setKey("Icon Theme", "Inherits", name);
    return keyfile.saveToFile(file);
}

void ThemesApi::setGtkCursor(QString name)
{
    GtkSettings *s = gtk_settings_get_default();
    g_object_set(G_OBJECT(s), "gtk-cursor-theme-name", name.toLatin1().data(), NULL);
}

static const char *findAlternative(const char *name)
{
    // Qt uses non-standard names for some core cursors.
    // If Xcursor fails to load the cursor, Qt creates it with the correct name
    // using the core protocol instead (which in turn calls Xcursor).
    // We emulate that process here.
    // Note that there's a core cursor called cross, but it's not the one Qt expects.
    // Precomputed MD5 hashes for the hardcoded bitmap cursors in Qt and KDE.
    // Note that the MD5 hash for left_ptr_watch is for the KDE version of that cursor.
    static const char *xcursor_alter[] = {
        "cross", "crosshair",
        "up_arrow", "center_ptr",
        "wait", "watch",
        "ibeam", "xterm",
        "size_all", "fleur",
        "pointing_hand", "hand2",
        // Precomputed MD5 hashes for the hardcoded bitmap cursors in Qt and KDE.
        // Note that the MD5 hash for left_ptr_watch is for the KDE version of that cursor.
        "size_ver", "00008160000006810000408080010102",
        "size_hor", "028006030e0e7ebffc7f7070c0600140",
        "size_bdiag", "c7088f0f3e6c8088236ef8e1e3e70000",
        "size_fdiag", "fcf1c3c7cd4491d801f1e1c78f100000",
        "whats_this", "d9ce0ab605698f320427677b458ad60b",
        "split_h", "14fef782d02440884392942c11205230",
        "split_v", "2870a09082c103050810ffdffffe0204",
        "forbidden", "03b6e0fcb3499374a867c041f52298f0",
        "left_ptr_watch", "3ecb610c1bf2410f44200f48c40d3599",
        "hand2", "e29285e634086352946a0e7090d73106",
        "openhand", "9141b49c8149039304290b508d208c40",
        "closedhand", "05e88622050804100c20044008402080",
        NULL
    };
    for (int i = 0; xcursor_alter[i] != NULL; i += 2) {
        if (strcmp(name, xcursor_alter[i]) == 0)
            return xcursor_alter[i + 1];
    }
    return NULL;
}

static XcursorImages *xcLoadImages(const char *theme, const char *image, int size)
{
    return XcursorLibraryLoadImages(image, theme, size);
}

static unsigned long loadCursorHandle(Display *disp, const char *theme, const char *name, int size)
{
    if (size == -1) {
        size = XcursorGetDefaultSize(disp);
    }

    // Load the cursor images
    XcursorImages *images = NULL;
    images = xcLoadImages(theme, name, size);
    if (!images) {
        images = xcLoadImages(theme, findAlternative(name), size);
        if (!images) {
            return 0;
        }
    }

    unsigned long handle = (unsigned long)XcursorImagesLoadCursor(disp, images);
    XcursorImagesDestroy(images);

    return handle;
}

int set_qt_cursor(const char *name)
{
    if (!name) {
        fprintf(stderr, "Cursor theme is NULL\n");
        return -1;
    }

    /**
     * Fixed Qt cursor not work when cursor theme changed.
     * For details see: lxqt-config/lxqt-config-cursor
     *
     * XFixes multiple qt cursor name, a X Error will be occurred.
     * Now only XFixes qt cursor name 'left_ptr'
     * Why?
     **/
    static const char *list[] = {
        // Qt cursors
        "left_ptr",
        "up_arrow",
        "cross",
        "wait",
        "left_ptr_watch",
        "ibeam",
        "size_ver",
        "size_hor",
        "size_bdiag",
        "size_fdiag",
        "size_all",
        "split_v",
        "split_h",
        "pointing_hand",
        "openhand",
        "closedhand",
        "forbidden",
        "whats_this",
        // X core cursors
        "X_cursor",
        "right_ptr",
        "hand1",
        "hand2",
        "watch",
        "xterm",
        "crosshair",
        "left_ptr_watch",
        "center_ptr", // invalid Cursor parameter, why?
        "sb_h_double_arrow",
        "sb_v_double_arrow",
        "fleur",
        "top_left_corner",
        "top_side",
        "top_right_corner",
        "right_side",
        "bottom_right_corner",
        "bottom_side",
        "bottom_left_corner",
        "left_side",
        "question_arrow",
        "pirate",
        NULL
    };

    Display *disp = XOpenDisplay(0);
    if (!disp) {
        qWarning() << "Open display failed";
        return -1;
    }
    for (int i = 0; list[i] != NULL; ++i) {
        Cursor cursor = (Cursor)loadCursorHandle(disp, name, list[i], -1);
        if (cursor == 0) {
            qWarning() << "Load cursor" << list[i] << "failed";
            continue;
        }

        XFixesChangeCursorByName(disp, cursor, list[i]);
        // FIXME: do we need to free the cursor?
        XFreeCursor(disp, cursor);
    }
    XCloseDisplay(disp);

    return 0;
}

void ThemesApi::setQtCursor(QString name)
{
#ifndef USE_XCB_CURSOR
    set_qt_cursor(name.toLatin1().data());
#else
    xcb_connection_t *conn;
    xcb_screen_t *screen = nullptr;
    int screen_nbr;
    xcb_screen_iterator_t iter;

    conn = xcb_connect(nullptr, &screen_nbr);

    iter = xcb_setup_roots_iterator(xcb_get_setup(conn));

    for (; iter.rem; --screen_nbr, xcb_screen_next(&iter)) {
        if (screen_nbr == 0) {
            screen = iter.data;
            break;
        }
    }
    if (!screen) {
        qWarning() << "get screen fail";
        return;
    }

    xcb_cursor_context_t *ctx;
    if (xcb_cursor_context_new(conn, screen, &ctx) < 0) {
        qWarning() << "xcb_cursor_context_new fail";
        return;
    }

    xcb_cursor_t cid = xcb_cursor_load_cursor(ctx, name.toLatin1());

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    xcb_change_window_attributes(conn, screen->root, XCB_CW_CURSOR, static_cast<void *>(&cid));
    xcb_flush(conn);

    xcb_cursor_context_free(ctx);
    xcb_disconnect(conn);
#endif
}

void ThemesApi::setWMCursor(QString name)
{
    if (interfaceSetting) {
        interfaceSetting->set("cursorTheme", name);
    }

    dbusProxy->setcursorTheme(name);
}

QString ThemesApi::getGtk2ConfFile()
{
    QString path = utils::GetUserHomeDir();
    path = path + "/.gtkrc-2.0";

    return path;
}

QString ThemesApi::getGtk3ConfFile()
{
    QString path = utils::GetUserHomeDir();
    path = path + "/.config/gtk-3.0/settings.ini";

    return path;
}
