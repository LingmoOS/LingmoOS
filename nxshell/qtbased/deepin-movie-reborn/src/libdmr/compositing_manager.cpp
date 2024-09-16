// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "compositing_manager.h"
#include "utils.h"
#ifndef _LIBDMR_
#include "options.h"
#endif

#include <iostream>
#include <unistd.h>
#include <QtCore>
#include <QtGui>
#include <QX11Info>
#include <QDBusInterface>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#undef Bool
#include "../vendor/qthelper.hpp"

#define BUFFERSIZE 255

typedef const char *glXGetScreenDriver_t (Display *dpy, int scrNum);

static glXGetScreenDriver_t *GetScreenDriver;

//TODO: override by user setting

namespace dmr {
using namespace std;

static CompositingManager *_compManager = nullptr;
bool CompositingManager::m_bCanHwdec = true;

#define C2Q(cs) (QString::fromUtf8((cs).c_str()))

class PlatformChecker
{
public:
    PlatformChecker() {}
    Platform check()
    {
        QProcess uname;
        uname.start("uname -m");
        if (uname.waitForStarted()) {
            if (uname.waitForFinished()) {
                auto data = uname.readAllStandardOutput();
                string machine(data.trimmed().constData());
                qInfo() << QString("machine: %1").arg(machine.c_str());

                QRegExp re("x86.*|i?86|ia64", Qt::CaseInsensitive);
                if (re.indexIn(C2Q(machine)) != -1) {
                    qInfo() << "match x86";
                    _pf = Platform::X86;

                } else if (machine.find("alpha") != string::npos
                           || machine.find("sw_64") != string::npos) {
                    // shenwei
                    qInfo() << "match shenwei";
                    _pf = Platform::Alpha;

                } else if (machine.find("mips") != string::npos
                           || machine.find("loongarch64") != string::npos) { // loongson
                    qInfo() << "match loongson";
                    _pf = Platform::Mips;
                } else if (machine.find("aarch64") != string::npos) { // ARM64
                    qInfo() << "match arm";
                    _pf = Platform::Arm64;
                }
            }
        }

        return _pf;
    }

private:
    Platform _pf {Platform::Unknown};
};


CompositingManager &CompositingManager::get()
{
    if (!_compManager) {
        _compManager = new CompositingManager();
    }

    return *_compManager;
}

//void compositingChanged(bool);

CompositingManager::CompositingManager()
{
    initMember();
    if (dmr::utils::check_wayland_env()) {
        _composited = true;
        //读取配置
        m_pMpvConfig = new QMap<QString, QString>;
        utils::getPlayProperty("/etc/mpv/play.conf", m_pMpvConfig);
        if (m_pMpvConfig->contains("vo")) {
            QString value = m_pMpvConfig->find("vo").value();
            if ("libmpv" == value) {
                _composited = true;//libmpv只能走opengl
            }
        }
        qInfo() << __func__ << "Composited is " << _composited;
        return;
    }
    softDecodeCheck();   //检测是否是kunpeng920（是否走软解码）

    _composited = false;
//    QGSettings gsettings("com.deepin.deepin-movie", "/com/deepin/deepin-movie/");
//    QString aa = gsettings.get("composited").toString();
//    if ((gsettings.get("composited").toString() == "DisableComposited"
//            || gsettings.get("composited").toString() == "EnableComposited")) {
//        if (gsettings.keys().contains("composited")) {
//            if (gsettings.get("composited").toString() == "DisableComposited") {
//                _composited = false;
//            } else if (gsettings.get("composited").toString() == "EnableComposited") {
//                _composited = true;
//            }
//        }
//    } else {
        bool isDriverLoaded = isDriverLoadedCorrectly();
        if (_platform == Platform::X86) {
            if (m_bZXIntgraphics) {
                _composited = false;
            } else {
                _composited = true;
            }
        } else {
            if (_platform == Platform::Arm64 && isDriverLoaded)
                m_bHasCard = true;
            _composited = false;
        }
//    }

    //读取配置
    m_pMpvConfig = new QMap<QString, QString>;
    utils::getPlayProperty("/etc/mpv/play.conf", m_pMpvConfig);
    if (m_pMpvConfig->contains("vo")) {
        QString value = m_pMpvConfig->find("vo").value();
        if ("libmpv" == value) {
            _composited = true;//libmpv只能走opengl
        }
    }
    //单元测试
#ifdef USE_TEST
    utils::getPlayProperty("/data/source/deepin-movie-reborn/movie/play.conf", m_pMpvConfig);
    if (m_pMpvConfig->contains("vo")) {
        QString value = m_pMpvConfig->find("vo").value();
        if ("libmpv" == value) {
            _composited = true;//libmpv只能走opengl
        } else {
            _composited = false;//libmpv只能走opengl
        }
    }
#endif
    if(!isMpvExists())
    {
        _composited = true;
    }
    qInfo() << __func__ << "Composited is " << _composited;
}

CompositingManager::~CompositingManager()
{
    delete m_pMpvConfig;
    m_pMpvConfig = nullptr;
}

#if !defined (__x86_64__)
bool CompositingManager::hascard()
{
    return m_bHasCard;
}
#endif

// Attempt to reuse mpv's code for detecting whether we want GLX or EGL (which
// is tricky to do because of hardware decoding concerns). This is not pretty,
// but quite effective and without having to duplicate too much GLX/EGL code.
/*static QString probeHwdecInterop()
{
//    auto mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
//    if (!mpv)
//        return "";
//    mpv::qt::set_property(mpv, "hwdec-preload", "auto");
//    // Actually creating a window is required. There is currently no way to keep
//    // this window hidden or invisible.
//    mpv::qt::set_property(mpv, "force-window", true);
//    // As a mitigation, put the window in the top/right corner, and make it as
//    // small as possible by forcing 1x1 size and removing window borders.
//    mpv::qt::set_property(mpv, "geometry", "1x1+0+0");
//    mpv::qt::set_property(mpv, "border", false);
//    if (mpv_initialize(mpv) < 0)
//        return "";
//    // return "auto"
//    return mpv::qt::get_property(mpv, "gpu-hwdec-interop").toString();
    return QString("");
}*/

static OpenGLInteropKind _interopKind = OpenGLInteropKind::INTEROP_NONE;

bool CompositingManager::runningOnVmwgfx()
{
    static bool s_runningOnVmwgfx = false;
//    static bool s_checked = false;

//    if (!s_checked) {
    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"vmwgfx"};
            s_runningOnVmwgfx = is_card_exists(id, drivers);
            break;
        }
//        }
    }

    return s_runningOnVmwgfx;
}

bool CompositingManager::isPadSystem()
{
    return false;
}

bool CompositingManager::isCanHwdec()
{
    return m_bCanHwdec;
}

QString  CompositingManager::libPath(const QString &sLib)
{
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (sLib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(sLib)) {
        return (path+ QDir::separator() + sLib);
    } else {
        list.sort();
    }

    if(list.size() > 0)
        return (path + QDir::separator() + list.last());
    else
        return QString();
}

void CompositingManager::setCanHwdec(bool bCanHwdec)
{
    m_bCanHwdec = bCanHwdec;
}

bool CompositingManager::isMpvExists()
{
    QString path  = libPath("libmpv.so.");
    if (path.contains("libmpv.so.")) {
        qInfo() << "curreng load mpv is :" << path;
        return true;
    }
    return false;
}

bool CompositingManager::isZXIntgraphics() const
{
    return m_bZXIntgraphics;
}

bool CompositingManager::runningOnNvidia()
{
    static bool s_runningOnNvidia = false;

    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"nvidia"};
            s_runningOnNvidia = is_card_exists(id, drivers);
            break;
        }
    }

    return s_runningOnNvidia;
}

void CompositingManager::softDecodeCheck()
{
    //获取cpu型号
    QFile cpuInfo("/proc/cpuinfo");
    if (cpuInfo.open(QIODevice::ReadOnly)) {
        QString line = cpuInfo.readLine();
        while (!cpuInfo.atEnd()) {
            line = cpuInfo.readLine();
            QStringList listPara = line.split(":");
            qInfo() << listPara;
            if (listPara.size() < 2) {
                continue;
            }
            if (listPara.at(0).contains("model name")) {
                m_cpuModelName = listPara.at(1);
                break;
            }
        }
        cpuInfo.close();
    }

    //获取设备名
    QFile board("/sys/class/dmi/id/board_vendor");
    if (board.open(QIODevice::ReadOnly)) {
        QString line = board.readLine();
        while (!board.atEnd()) {
            m_boardVendor = line;
            break;
        }
        board.close();
    }

    if ((runningOnNvidia() && m_boardVendor.contains("Sugon"))
            || m_cpuModelName.contains("Kunpeng 920")) {
        m_bOnlySoftDecode = true;
    }

    m_setSpecialControls = m_boardVendor.contains("Ruijie");

    //判断N卡驱动版本
    QFile nvidiaVersion("/proc/driver/nvidia/version");
    if (nvidiaVersion.open(QIODevice::ReadOnly)) {
        QString str = nvidiaVersion.readLine();
        int start = str.indexOf("Module");
        start += 6;
        QString version = str.mid(start, 6);
        while (version.left(1) == " ") {
            start++;
            version = str.mid(start, 6);
        }
        qInfo() << "nvidia version :" << version;
        if (version.toFloat() >= 460.39) {
            m_bOnlySoftDecode = true;
        }
        nvidiaVersion.close();
    }
}

bool CompositingManager::isOnlySoftDecode()
{
    return m_bOnlySoftDecode;
}

bool CompositingManager::isSpecialControls()
{
    return m_setSpecialControls;
}

void CompositingManager::detectOpenGLEarly()
{
    static bool detect_run = false;

    if (detect_run) return;

    ///function probeHwdecInterop() always returns QString(""), this code was not used
//    auto probed = probeHwdecInterop();
//    qInfo() << "probeHwdecInterop" << probed
//             << qgetenv("QT_XCB_GL_INTERGRATION");

//    if (probed == "auto") {
//        _interopKind = INTEROP_AUTO;
//    } else if (probed == "vaapi-egl") {
//        _interopKind = INTEROP_VAAPI_EGL;
//    } else if (probed == "vaapi-glx") {
//        _interopKind = INTEROP_VAAPI_GLX;
//    } else if (probed == "vdpau-glx") {
//        _interopKind = INTEROP_VDPAU_GLX;
//    }

#ifndef USE_DXCB
    /*
     * see mpv/render_gl.h for more details, below is copied verbatim:
     *
     * - Intel/Linux: EGL is required, and also the native display resource needs
     *                to be provided (e.g. MPV_RENDER_PARAM_X11_DISPLAY for X11 and
     *                MPV_RENDER_PARAM_WL_DISPLAY for Wayland)
     * - nVidia/Linux: Both GLX and EGL should work (GLX is required if vdpau is
     *                 used, e.g. due to old drivers.)
     *
     * mpv hwdec is broken with vmwgfx and should use glx
     */
    if (CompositingManager::runningOnNvidia()) {
        qputenv("QT_XCB_GL_INTEGRATION", "xcb_glx");
    } else if (!CompositingManager::runningOnVmwgfx()) {
        auto e = QProcessEnvironment::systemEnvironment();
        QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
        QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

        if (XDG_SESSION_TYPE != QLatin1String("wayland") &&
                !WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
            qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");
        }
    }
#else
    if (_interopKind == INTEROP_VAAPI_EGL) {
        _interopKind = INTEROP_VAAPI_GLX;
    }

#endif

    detect_run = true;
}

void CompositingManager::detectPciID()
{
    QProcess pcicheck;
    pcicheck.start("lspci -vn");
    if (pcicheck.waitForStarted() && pcicheck.waitForFinished()) {

        auto data = pcicheck.readAllStandardOutput();

        QString output(data.trimmed().constData());
        qInfo() << "CompositingManager::detectPciID()" << output.split(QChar('\n')).count();

        QStringList outlist = output.split(QChar('\n'));
        foreach (QString line, outlist) {
//            qInfo()<<"CompositingManager::detectPciID():"<<line;
            if (line.contains(QString("00:02.0"))) {
                if (line.contains(QString("8086")) && line.contains(QString("1912"))) {
                    qInfo() << "CompositingManager::detectPciID():need to change to iHD";
                    qputenv("LIBVA_DRIVER_NAME", "iHD");
                    break;
                }
            }
        }
    }
}

void CompositingManager::getMpvConfig(QMap<QString, QString> *&aimMap)
{
    aimMap = nullptr;
    if (nullptr != m_pMpvConfig) {
        aimMap = m_pMpvConfig;
    }
}

OpenGLInteropKind CompositingManager::interopKind()
{
    return _interopKind;
}

bool CompositingManager::isDriverLoadedCorrectly()
{
    static QRegExp aiglx_err("\\(EE\\)\\s+AIGLX error");
    static QRegExp dri_ok("direct rendering: DRI\\d+ enabled");
    static QRegExp swrast("GLX: Initialized DRISWRAST");
    static QRegExp regZX("loading driver: zx");

    QString xorglog = QString("/var/log/Xorg.%1.log").arg(QX11Info::appScreen());
    qInfo() << "check " << xorglog;
    QFile f(xorglog);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "can not open " << xorglog;
        return false;
    }

    QTextStream ts(&f);
    while (!ts.atEnd()) {
        QString ln = ts.readLine();
        if (aiglx_err.indexIn(ln) != -1) {
            qInfo() << "found aiglx error";
            return false;
        }

        if (dri_ok.indexIn(ln) != -1) {
            qInfo() << "dri enabled successfully";
            return true;
        }

        if (swrast.indexIn(ln) != -1) {
            qInfo() << "swrast driver used";
            return false;
        }

        if (regZX.indexIn(ln) != -1) {
            m_bZXIntgraphics = true;
        }
    }
    f.close();
    return true;
}

void CompositingManager::overrideCompositeMode(bool useCompositing)
{
    if (_composited != useCompositing) {
        qInfo() << "override composited = " << useCompositing;
        _composited = useCompositing;
    }
}

using namespace std;

bool CompositingManager::is_card_exists(int id, const vector<string> &drivers)
{
    char buf[1024] = {0};
    snprintf(buf, sizeof buf, "/sys/class/drm/card%d/device/driver", id);

    char buf2[1024] = {0};
    if (readlink(buf, buf2, sizeof buf2) < 0) {
        return false;
    }

    string driver = basename(buf2);
    qInfo() << "drm driver " << driver.c_str();
    if (std::any_of(drivers.cbegin(), drivers.cend(), [ = ](string s) {return s == driver;})) {
        return true;
    }

    return false;
}

bool CompositingManager::is_device_viable(int id)
{
    char path[128];
    snprintf(path, sizeof path, "/sys/class/drm/card%d", id);
    if (access(path, F_OK) != 0) {
        return false;
    }

    //OK, on shenwei, this file may have no read permission for group/other.
    char buf[512];
    snprintf(buf, sizeof buf, "%s/device/enable", path);
    if (access(buf, R_OK) == 0) {
        FILE *fp = fopen(buf, "r");
        if (!fp) {
            return false;
        }

        int enabled = 0;
        int error = fscanf(fp, "%d", &enabled);
        if (error < 0) {
            qInfo() << "someting error";
        }
        fclose(fp);

        // nouveau may write 2, others 1
        return enabled > 0;
    }

    return false;
}

bool CompositingManager::isProprietaryDriver()
{
    for (int id = 0; id <= 10; id++) {
        if (!QFile::exists(QString("/sys/class/drm/card%1").arg(id))) break;
        if (is_device_viable(id)) {
            vector<string> drivers = {"nvidia", "fglrx", "vmwgfx", "hibmc-drm", "radeon", "i915", "amdgpu", "phytium_display"};
            return is_card_exists(id, drivers);
        }
    }

    return false;
}

void CompositingManager::initMember()
{
    m_pMpvConfig = nullptr;
    _platform = PlatformChecker().check();

    m_bZXIntgraphics = false;
    m_bHasCard = false;
}

//this is not accurate when proprietary driver used
bool CompositingManager::isDirectRendered()
{
//避免klu 上产生xdriinfo的coredump
//    QProcess xdriinfo;
//    xdriinfo.start("xdriinfo driver 0");
//    if (xdriinfo.waitForStarted() && xdriinfo.waitForFinished()) {
//        QString drv = QString::fromUtf8(xdriinfo.readAllStandardOutput().trimmed().constData());
//        qInfo() << "xdriinfo: " << drv;
//        return !drv.contains("not direct rendering capable");
//    }

    return true;
}

//FIXME: what about merge options from both config
PlayerOptionList CompositingManager::getProfile(const QString &name)
{
    auto localPath = QString("%1/%2/%3/%4.profile")
                     .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                     .arg(qApp->organizationName())
                     .arg(qApp->applicationName())
                     .arg(name);
    auto defaultPath = QString(":/resources/profiles/%1.profile").arg(name);
#ifdef _LIBDMR_
    QString oc;
#else
    auto oc = CommandLineManager::get().overrideConfig();
#endif

    PlayerOptionList ol;

    QList<QString> files = {oc, localPath, defaultPath};
    auto p = files.begin();
    while (p != files.end()) {
        QFileInfo fi(*p);
        if (fi.exists()) {
            qInfo() << "load" << fi.absoluteFilePath();
            QFile f(fi.absoluteFilePath());
            f.open(QIODevice::ReadOnly);
            QTextStream ts(&f);
            while (!ts.atEnd()) {
                auto l = ts.readLine().trimmed();
                if (l.isEmpty()) continue;

                auto kv = l.split("=");
                qInfo() << l << kv;
                if (kv.size() == 1) {
                    ol.push_back(qMakePair(kv[0], QString::fromUtf8("")));
                } else {
                    ol.push_back(qMakePair(kv[0], kv[1]));
                }
            }
            f.close();

            return ol;
        }
        ++p;
    }

    return ol;
}

PlayerOptionList CompositingManager::getBestProfile()
{
    QString profile_name = "default";
    switch (_platform) {
    case Platform::Alpha:
    case Platform::Mips:
    case Platform::Arm64:
        profile_name = _composited ? "composited" : "failsafe";
        break;
    case Platform::X86:
        profile_name = _composited ? "composited" : "default";
        break;
    case Platform::Unknown:
        break;
    }

    return getProfile(profile_name);
}

#undef C2Q
}

