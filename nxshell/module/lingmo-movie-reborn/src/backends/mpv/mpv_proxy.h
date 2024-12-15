// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_MPV_PROXY_H
#define _DMR_MPV_PROXY_H

#define MWV206_0  //After Jing Jiawei's graphics card is upgraded, deal with the macro according to the situation,
//This macro is also available for compositing_manager.cpp.

#include <player_backend.h>
#include <player_engine.h>
#include <xcb/xproto.h>
#undef Bool
#include "../../vendor/qthelper.hpp"

typedef mpv_event *(*mpv_waitEvent)(mpv_handle *ctx, double timeout);
typedef int (*mpv_set_optionString)(mpv_handle *ctx, const char *name, const char *data);
typedef int (*mpv_setProperty)(mpv_handle *ctx, const char *name, mpv_format format,
                               void *data);
typedef int (*mpv_setProperty_async)(mpv_handle *ctx, uint64_t reply_userdata,
                                     const char *name, mpv_format format, void *data);
typedef int (*mpv_commandNode)(mpv_handle *ctx, mpv_node *args, mpv_node *result);
typedef int (*mpv_commandNode_async)(mpv_handle *ctx, uint64_t reply_userdata,
                                     mpv_node *args);
typedef int (*mpv_getProperty)(mpv_handle *ctx, const char *name, mpv_format format,
                               void *data);
typedef int (*mpv_observeProperty)(mpv_handle *mpv, uint64_t reply_userdata,
                                   const char *name, mpv_format format);
typedef const char *(*mpv_eventName)(mpv_event_id event);
typedef mpv_handle *(*mpvCreate)(void);
typedef int (*mpv_requestLog_messages)(mpv_handle *ctx, const char *min_level);
typedef int (*mpv_observeProperty)(mpv_handle *mpv, uint64_t reply_userdata,
                                   const char *name, mpv_format format);
typedef void (*mpv_setWakeup_callback)(mpv_handle *ctx, void (*cb)(void *d), void *d);
typedef int (*mpvinitialize)(mpv_handle *ctx);
typedef void (*mpv_freeNode_contents)(mpv_node *node);
typedef void (*mpv_terminateDestroy)(mpv_handle *ctx);

static QString libPath(const QString &sLib)
{
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (sLib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(sLib)) {
        return sLib;
    } else {
        list.sort();
    }

    if(list.size() > 0)
        return list.last();
    else
        return QString();
}

class MpvHandle
{
    struct container {
        explicit container(mpv_handle *pHandle) : m_pHandle(pHandle) {}
        ~container()
        {
            mpv_terminateDestroy func = (mpv_terminateDestroy)QLibrary::resolve(libPath("libmpv.so."), "mpv_terminate_destroy");
            func(m_pHandle);
        }
        mpv_handle *m_pHandle;
    };
    QSharedPointer<container> sptr;
public:
    // Construct a new Handle from a raw mpv_handle with refcount 1. If the
    // last Handle goes out of scope, the mpv_handle will be destroyed with
    // mpv_terminate_destroy().
    // Never destroy the mpv_handle manually when using this wrapper. You
    // will create dangling pointers. Just let the wrapper take care of
    // destroying the mpv_handle.
    // Never create multiple wrappers from the same raw mpv_handle; copy the
    // wrapper instead (that's what it's for).
    static MpvHandle fromRawHandle(mpv_handle *pHandle)
    {
        MpvHandle mpvHandle;
        mpvHandle.sptr = QSharedPointer<container>(new container(pHandle));
        return mpvHandle;
    }

    // Return the raw handle; for use with the libmpv C API.
    operator mpv_handle *() const
    {
        return sptr ? (*sptr).m_pHandle : 0;
    }
};

namespace dmr {
using namespace mpv::qt;
class MpvGLWidget;

//解码模式
enum DecodeMode {
    AUTO = 0,
    HARDWARE,
    SOFTWARE
};

/**
 * @file 封装mpv播放引擎
 */

class MpvProxy: public Backend
{
    Q_OBJECT

    struct my_node_autofree {
        mpv_node *pNode;
        explicit my_node_autofree(mpv_node *pValue) : pNode(pValue) {}
        ~my_node_autofree()
        {
            mpv_freeNode_contents(pNode);
        }
    };

signals:
    void has_mpv_events();

    /**
    * @brief 崩溃检测
    */
    void crashCheck();

public:
    explicit MpvProxy(QWidget *parent = 0);
    virtual ~MpvProxy();

    /**
     * @brief 设置解码模式
     */
    void setDecodeModel(const QVariant &value);
    /**
     * @brief 刷新解码方式
     */
    void refreshDecode();

//    //add by heyi
    /**
     * @brief initMpvFuns   初始化MPV动态调用库函数
     */
    void initMpvFuns();
    //add by heyi
    /**
     * @brief firstInit 第一次播放需要初库始化函数指针
     */
    void firstInit();
    /**
     * @brief 初始化mpv设置
     */
    void initSetting();
    /**
     * @brief updateRoundClip 更新opengl绘制圆角
     * @param roundClip 是否为圆角
     */
    void updateRoundClip(bool roundClip);
    /**
     * @brief 正在播放影片的影片信息
     */
    const PlayingMovieInfo &playingMovieInfo() override;
    // mpv plays all files by default  (I hope)
    bool isPlayable() const override
    {
        return true;
    }
    // polling until current playback ended
    void pollingEndOfPlayback();
    // polling until current playback started
    void pollingStartOfPlayback();
    /**
     * @brief 获取影片时间长
     */
    qint64 duration() const override;
    /**
     * @brief 获取影片当前进度
     */
    qint64 elapsed() const override;
    /**
     * @brief 获取当前影片显示大小
     */
    QSize videoSize() const override;
    /**
     * @brief 设置播放速度
     * @param 范围0.01-100
     */
    void setPlaySpeed(double dTimes) override;
    /**
     * @brief 播放记录，记录播放到当前时刻
     */
    void savePlaybackPosition() override;
    /**
     * @brief 加载字幕
     */
    bool loadSubtitle(const QFileInfo &fileInfo) override;
    /**
     * @brief 显示或隐藏字幕
     */
    void toggleSubtitle() override;
    /**
     * @brief 获取字幕显示状态
     * @return 是否可见
     */
    bool isSubVisible() override;
    /**
     * @brief 选择字幕
     */
    void selectSubtitle(int nId) override;
    /**
     * @brief 返回当前字幕id
     */
    int sid() const override;
    /**
     * @brief 设置字幕延时
     * @param 延时多少秒
     */
    void setSubDelay(double dSecs) override;
    /**
     * @brief 返回当前字幕延时
     */
    double subDelay() const override;
    /**
     * @brief 设置字幕样式
     * @param 字幕字体
     * @param 字幕大小
     */
    void updateSubStyle(const QString &sFont, int nSize) override;
    /**
     * @brief 设置编码
     */
    void setSubCodepage(const QString &sCodePage) override;
    /**
     * @brief 返回当前编码
     */
    QString subCodepage() override;
    /**
     * @brief 设置在线字幕路径
     */
    void addSubSearchPath(const QString &sPath) override;
    /**
     * @brief 设置声道
     */
    void selectTrack(int nId) override;
    /**
     * @brief 返回当前声道
     */
    int aid() const override;
    /**
     * @brief 设置声道模式
     * @param 声道模式（左声道、右声道、立体声）
     */
    void changeSoundMode(SoundMode soundMode) override;
    /**
     * @brief 获取mpv音量
     */
    int volume() const override;
    /**
     * @brief 获取mpv静音状态
     */
    bool muted() const override;
    /**
     * @brief 设置画面比例
     * @param 宽高比
     */
    void setVideoAspect(double dValue) override;
    /**
     * @brief 获取当前画面比例
     * @return 画面比例
     */
    double videoAspect() const override;
    /**
     * @brief 获取画面旋转角度
     */
    int videoRotation() const override;
    /**
     * @brief 设置影片旋转角度
     */
    void setVideoRotation(int nDegree) override;
    /**
     * @brief 画面截图
     */
    QImage takeScreenshot() override;
    /**
     * @brief 画面连拍截图
     */
    void burstScreenshot() override; //initial the start of burst screenshotting
    /**
     * @brief 停止连拍截图
     */
    void stopBurstScreenshot() override;
    /**
     * @brief 获取mpv参数值
     * @param 要获取值的参数
     * @return 返回的参数值
     */
    QVariant getProperty(const QString &) override;
    /**
     * @brief 设置mpv参数值
     * @param 想要设置值的参数
     * @param 设置的值
     */
    void setProperty(const QString &, const QVariant &) override;
    /**
     * @brief 播放下一帧画面
     */
    void nextFrame() override;
    /**
     * @brief 播放上一帧画面
     */
    void previousFrame() override;
    /**
     * @brief 设置硬解码方式
     */
    void changehwaccelMode(hwaccelMode hwaccelMode) override;

    void makeCurrent() override;

public slots:
    /**
     * @brief 播放当前影片
     */
    void play() override;
    /**
     * @brief 暂停或恢复暂停
     */
    void pauseResume() override;
    /**
     * @brief 终止播放
     */
    void stop() override;
    /**
     * @brief 向前seek
     * @param 当前往前多少秒
     */
    void seekForward(int nSecs) override;
    /**
     * @brief 向后seek
     * @param 当前往后多少秒
     */
    void seekBackward(int nSecs) override;
    /**
     * @brief seek到某个位置
     * @param 某个进度点(秒)
     */
    void seekAbsolute(int nPos) override;
    /**
     * @brief 加音量
     */
    void volumeUp() override;
    /**
     * @brief 减音量
     */
    void volumeDown() override;
    /**
     * @brief 调整音量大小
     */
    void changeVolume(int nVol) override;
    /**
     * @brief 循环改变静音状态
     */
    void toggleMute() override;
    /**
     * @brief 指定改变静音状态
     */
    void setMute(bool bMute) override;

protected:
    void initMember();      //初始化成员变量
    /**
     * @brief initMpvFuns   初始化gpuinfo硬解探测函数
     */
    void initGpuInfoFuns();
    /**
     * @brief 是否支持硬件解码
     */
    bool isSurportHardWareDecode(const QString sDecodeName, const int &nVideoWidth, const int &nVideoHeight);
    /**
     * @brief 获取探测解码值
     */
    int getDecodeProbeValue(const QString sDecodeName);
    void resizeEvent(QResizeEvent *pEvent) override;
    void showEvent(QShowEvent *pEvent) override;

protected slots:
    void handle_mpv_events();
    void stepBurstScreenshot();
    void slotStateChanged();

private:
    mpv_handle *mpv_init();   //初始化mpv
    void processPropertyChange(mpv_event_property *pEvent);
    void processLogMessage(mpv_event_log_message *pEvent);
    QImage takeOneScreenshot();
    void updatePlayingMovieInfo();
    void setState(PlayState state);
    qint64 nextBurstShootPoint();
    int volumeCorrection(int);

    //add by heyi
    QVariant my_get_property(mpv_handle *pHandle, const QString &sName) const;
    int my_set_property(mpv_handle *pHandle, const QString &sName, const QVariant &value);
    bool my_command_async(mpv_handle *pHandle, const QVariant &args, uint64_t tag);
    int my_set_property_async(mpv_handle *pHandle, const QString &sName,
                              const QVariant &v, uint64_t tag);
    QVariant my_get_property_variant(mpv_handle *pHandle, const QString &sName);
    QVariant my_command(mpv_handle *pHandle, const QVariant &args);

private:
    mpv_waitEvent m_waitEvent;
    mpv_set_optionString m_setOptionString;
    mpv_setProperty m_setProperty;
    mpv_setProperty_async m_setPropertyAsync;
    mpv_commandNode m_commandNode;
    mpv_commandNode_async m_commandNodeAsync;
    mpv_getProperty m_getProperty;
    mpv_observeProperty m_observeProperty;
    mpv_eventName m_eventName;
    mpvCreate m_creat;
    mpv_requestLog_messages m_requestLogMessage;
    mpv_setWakeup_callback m_setWakeupCallback;
    mpvinitialize m_initialize;
    mpv_freeNode_contents m_freeNodecontents;
    void *m_gpuInfo; //解码探测函数指针


    MpvHandle m_handle;                    //mpv句柄
    MpvGLWidget *m_pMpvGLwidget;           //opengl窗口
    QWidget *m_pParentWidget;
    PlayingMovieInfo m_movieInfo;          //播放过的影片的信息

    QString m_sInitVo;                     //初始vo方式
    QVariant m_posBeforeBurst;             //截图前影片播放位置
    QList<qint64> m_listBurstPoints;       //存储连拍截图截图位置

    qint64 m_nBurstStart;                  //记录连拍截图次数

    bool m_bPendingSeek;
    bool m_bInBurstShotting;               //是否停止连拍截图

    bool m_bPolling;
    bool m_bConnectStateChange;
    bool m_bPauseOnStart;                  //mpv是否在暂停中
    bool m_bIsJingJia;                     //是否在景嘉微平台上
    bool m_bInited;                        //mpv是否已经初始化
    bool m_bHwaccelAuto;                   //如果设置为不为自动，则不允许此类改变硬件设置
    bool m_bLastIsSpecficFormat;           //上一曲是否是特殊格式的影片，如果是则应该重新设置vo
    QMap<QString, QVariant> m_mapWaitSet;  //等待mpv初始化后设置的参数
    QVector<QVariant> m_vecWaitCommand;    //等待mpv初始化后设置的参数
    //mpv播放配置
    QMap<QString, QString> *m_pConfig;

    //解码模式
    DecodeMode m_decodeMode {DecodeMode::AUTO};
};

}

#endif /* ifndef _DMR_MPV_PROXY_H */



