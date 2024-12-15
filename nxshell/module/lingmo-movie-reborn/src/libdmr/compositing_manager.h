// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_COMPOSITING_MANAGER
#define _DMR_COMPOSITING_MANAGER

#define MWV206_0  //After Jing Jiawei's graphics card is upgraded, deal with the macro according to the situation,
//This macro is also available for mpv_proxy.cpp.

#include <QtCore>
#include <string>
#include <vector>
#include <QGSettings/QGSettings>

namespace dmr {

enum Platform {
    Unknown,
    X86,  // intel & amd
    Mips, // loongson
    Alpha, // sunway
    Arm64
};

enum OpenGLInteropKind {
    INTEROP_NONE,
    INTEROP_AUTO,
    INTEROP_VAAPI_EGL,
    INTEROP_VAAPI_GLX,
    INTEROP_VDPAU_GLX,
};

using PlayerOption = QPair<QString, QString>;
using PlayerOptionList = QList<PlayerOption>;

class CompositingManager: public QObject
{
public:
    static CompositingManager &get();
    virtual ~CompositingManager();

    /**
     * should call this before any other qt functions get exec'ed.
     * this makes sure mpv openglcb-interop to work correctly
     */
    static void detectOpenGLEarly();
    /**
     * get detectOpenGLEarly result
     */
    static OpenGLInteropKind interopKind();
    static bool runningOnVmwgfx();
    static bool isPadSystem();   //是否是平板设备
    static bool isCanHwdec();
    static void setCanHwdec(bool bCanHwdec);
    static QString libPath(const QString &sLib);
    /**
     * @brief 判断是否可以使用mpv
     */
    static bool isMpvExists();

#if !defined (__x86_64__)
    bool hascard();
#endif

    /**
     * override auto-detected compositing state.
     * should call this right before player engine gets instantiated.
     * it has been deprecated but cannot remove due to ABI
     */
    void overrideCompositeMode(bool useCompositing);

    // this actually means opengl rendering is capable
    bool composited() const
    {
#if defined (_LIBDMR_)
        return true;
#endif
        return _composited;
    }
    Platform platform() const
    {
        return _platform;
    }

    bool isTestFlag() const
    {
        return _isCoreFlag;
    }
    void setTestFlag(bool flag)
    {
        _isCoreFlag = flag;
    }
    bool isZXIntgraphics() const;

    PlayerOptionList getProfile(const QString &name);
    PlayerOptionList getBestProfile(); // best for current platform and env
    static void detectPciID();
    static bool runningOnNvidia();
    void softDecodeCheck();
    bool isOnlySoftDecode();
    bool isSpecialControls();
    void getMpvConfig(QMap<QString, QString> *&aimMap);

signals:
    void compositingChanged(bool);
public:
    static bool m_bCanHwdec;//是否支持硬解，true支持，false不支持
private:
    CompositingManager();
    bool isDriverLoadedCorrectly();
    bool isDirectRendered();
    bool isProprietaryDriver();
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

    static bool is_device_viable(int id);
    static bool is_card_exists(int id, const std::vector<std::string> &drivers);

    bool _composited {false};
    Platform _platform {Platform::Unknown};
    bool _isCoreFlag {false};
    bool m_bHasCard;
    bool m_bOnlySoftDecode {false};  //kunpeng920走软解码
    bool m_setSpecialControls {false};
    bool m_bZXIntgraphics;
    //保存配置
    QMap<QString, QString> *m_pMpvConfig;

    QString m_cpuModelName;
    QString m_boardVendor;
};
}

#endif /* ifndef _DMR_COMPOSITING_MANAGER */

