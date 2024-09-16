// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <DWidget>
#include <DFloatingWidget>
#include <DLabel>
#include <DFontSizeManager>
#include <DPushButton>
#include <DGuiApplicationHelper>
#include <DApplicationHelper>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QtMultimedia/QSound>
#include <QDateTime>
#include <QSvgRenderer>
#include <QGraphicsView>

#include "LPF_V4L2.h"
#include "majorimageprocessingthread.h"
#include "audioprocessingthread.h"
#include "previewopenglwidget.h"
#include "filterpreviewbutton.h"
#include "gridline.h"
#include "titlebar.h"

#include "gstvideowriter.h"

DWIDGET_USE_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsTextItem;
class QGridLayout;
class QVBoxLayout;
class QHBoxLayout;
class QSpacerItem;
class QSound;
class QGraphicsSvgItem;

#define FLASH_TIME 500//拍照闪光时间，500毫秒
/**
* @brief PRIVIEW_ENUM_STATE　枚举拍照，无设备，录像
*/
enum PRIVIEW_ENUM_STATE {PICTRUE, NODEVICE, VIDEO};

// 重写QGraphicsView类，在子类中，直接将鼠标事件路由到QWidget，这样QGraphicsScene不会接收和处理鼠标事件
class QGraphicsViewEx : public QGraphicsView
{
    Q_OBJECT

public:
    explicit QGraphicsViewEx(QWidget *parent = nullptr);

protected:
    virtual void mouseMoveEvent(QMouseEvent* e) override;
};

/**
* @brief videowidget　完成拍照，录像相关工作
* 连拍，延时拍照录像，闪光灯，文件保存等
*/
class videowidget : public DWidget
{
    Q_OBJECT
public:
    explicit videowidget(DWidget *parent = nullptr);
    ~videowidget();

signals:
    /**
    * @brief sigDeviceChange　相机改变信号
    */
    void sigDeviceChange();

    /**
    * @brief takePicCancel　取消拍照
    */
    void takePicCancel();

    /**
    * @brief takePicDone　拍照结束信号
    */
    void takePicDone();

    /**
    * @brief takePicOnce　多连拍时，除最后一次，每次提交一个信号
    */
    void takePicOnce();

    /**
    * @brief takeVdCancel　录制倒计时期间取消
    */
    void takeVdCancel();

    /**
    * @brief takeVdDone　录制完成
    */
    void takeVdDone();

    /**
    * @brief updateBlockSystem　阻塞更新
    * @param bTrue
    */
    void updateBlockSystem(bool bTrue);

    /**
    * @brief noCam　无相机
    */
    void noCam();

    /**
    * @brief noCamAvailable　无相机可获取
    */
    void noCamAvailable();
    void camAvailable();

    /**
    * @brief filename　文件名
    * @param strFilename 文件名
    */
    void filename(QString strFilename);

    /**
    * @brief switchCameraSuccess　摄像头切换成功通知
    */
    void switchCameraSuccess(const QString &devName);

    /**
    * @brief updateRecordState　更新录制状态
    * @param state 状态
    */
    void updateRecordState(int state);

    /**
    * @brief updatePhotoState　更新拍照状态
    * @param state 状态
    */
    void updatePhotoState(int state);

    /**
     * @brief filterImage 更新滤镜预览帧图片
     * @param image 预览帧图片 大小40*40
     */
    void updateFilterImage(QImage *image);

    /**
     * @brief reflushSnapshotLabel 刷新照片预览图信号
     */
    void reflushSnapshotLabel();

public:
    /**
    * @brief setCapStatus　设置相机状态
    * @param status 拍照状态
    */
    void setCapStatus(bool status);

    /**
    * @brief getCapStatus　获取相机录制状态
    */
    bool getCapStatus()
    {
        return m_bActive;
    }

    /**
    * @brief setSaveVdFolder　设置保存视频文件夹
    * @param strVdFolder 视频文件夹路径
    */
    void setSaveVdFolder(QString &strVdFolder)
    {
        m_saveVdFolder = strVdFolder;
    }

    /**
    * @brief setSavePicFolder　设置保存照片文件夹
    * @param strPicFolder 照片文件夹路径
    */
    void setSavePicFolder(QString &strPicFolder)
    {
        m_savePicFolder = strPicFolder;
    }

    /**
    * @brief setInterval　设置倒计时
    * @param nInterval 拍照延时
    */
    void setInterval(int nInterval)
    {
        m_nMaxInterval = nInterval;
        m_Maxinterval = nInterval;
    }

    /**
    * @brief setContinuous　设置连拍次数
    * @param nContinuous 连拍次数
    */
    void setContinuous(int nContinuous)
    {
        m_nMaxContinuous = m_curTakePicTime = nContinuous;
    }

    /**
    * @brief delayInit　延迟加载
    */
    void delayInit();

    /**
     * @brief setHorizontalMirror 设置水平镜像
     * @param bMirror 是否镜像
     */
    void setHorizontalMirror(bool bMirror);

    /**
    * @brief setFlash　设置闪光灯开启关闭
    * @param bFlashOn  是否开启
    */
    void setFlash(bool bFlashOn);

    /**
    * @brief setMaxRecTime　设置最大录像时长
    * @param hour  最大录像小时数
    */
    void setMaxRecTime(int hour);

    /**
    * @brief setFilterType　设置滤镜类型
    * @param type  滤镜类型
    */
    void setFilterType(efilterType type);

    /**
     * @brief setState 设置拍照，录像状态
     * @param bPhoto  true 拍照状态， false 录像状态
     */
    void setState(bool bPhoto);

    /**
     * @brief getFrameSize 获取画面显示区域
     * @return 画面显示区域
     */
    QRect getFrameRect();

    /**
     * @brief setGridType 设置网格线类型
     * @param type 网格线类型
     */
    void setGridType(GridType type);

    /**
    * @brief showNocam　显示没有设备的图片的槽函数
    */
    void showNocam();

    /**
    * @brief showCountDownLabel　显示设备被占用或者拔掉的图片的槽函数
    */
    void showCamUsed();

public slots:
    /**
    * @brief onTakePic　拍照事件响应
    * @param bTrue
    */
    void onTakePic(bool bTrue);

    /**
    * @brief onTakeVideo　录像事件响应
    */
    void onTakeVideo();

    /**
    * @brief showCountdown　倒计时
    */
    void showCountdown();

    /**
    * @brief showRecTime　录制3秒后，每200ms读取时间并显示
    */
    void showRecTime();

    /**
    * @brief onChangeDev　设备切换
    */
    void onChangeDev();

    /**
    * @brief onEndBtnClicked　点击结束按钮
    */
    void onEndBtnClicked();

    /**
    * @brief restartDevices　重启设备
    */
    void onRestartDevices();

    /**
    * @brief onThemeTypeChanged　主题切换槽函数
    */
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);

    /**
     * @brief onExposureChanged 曝光值改变槽函数
     * @param exposure 曝光值
     */
    void onExposureChanged(int exposure);

    /**
     * @brief onFilterDisplayChanged 滤镜展开状态
     * @param bDisplay 是否展示
     */
    void onFilterDisplayChanged(int bDisplay);

private slots:
    /**
    * @brief ReceiveMajorImage　处理视频帧 mips、wayland下使用
    */
    void ReceiveMajorImage(QImage *image, int result);

#ifndef __mips__
    /**
    * @brief ReceiveOpenGLstatus　接收openGL状态
    */
    void ReceiveOpenGLstatus(bool);
#endif

    /**
    * @brief onReachMaxDelayedFrames　达到视频帧最大延迟
    */
    void onReachMaxDelayedFrames();

    /**
    * @brief flash　闪光
    */
    void flash();

    /**
    * @brief slotresolutionchanged　分辨率改变槽函数
    * @param 分辨率字符串(如：1920*1080)
    */
    void slotresolutionchanged(const QString &);

    /**
    * @brief slotGridTypeChanged　网格线类型改变槽函数
    * @param 网格线类型
    */
    void slotGridTypeChanged(int);

    /**
     * @brief slotVideoFormatChanged 视频格式改变槽函数
     * @param 视频格式
     */
    void slotVideoFormatChanged(const QString &);

    /**
    * @brief onRecordFrame　向GStreamer写入视频帧数据
    */
    void onRecordFrame(uchar* yuv, uint size);

    /**
    * @brief onRecordAudio　向GStreamer写入音频帧数据
    */
    void onRecordAudio(uchar* data, uint size);

private:
    void resizeEvent(QResizeEvent *size) Q_DECL_OVERRIDE;

    /**
    * @brief showCountDownLabel　显示录制和拍照倒计时
    * @param state 当前状态（拍照，没有设备，视频）
    */
    void showCountDownLabel(PRIVIEW_ENUM_STATE state);

    /**
    * @brief forbidScrollBar　禁止滚轮显示区域
    * @param view
    */
    void forbidScrollBar(QGraphicsView *view);

    /**
    * @brief startTakeVideo　开始录像
    */
    void startTakeVideo();

    /**
     * @brief startCaptureVideo gstreamer环境下录制视频
     */
    void startCaptureVideo();

    /**
    * @brief itemPosChange　item位置改变
    */
    void itemPosChange();

    /**
    * @brief itemPosChange　结束所有操作
    */
    void stopEverything();

    /**
    * @brief switchCamera　切换摄像机
    * @param device     设备识别号 /dev/video*
    * @param devName    设备名称
    * @return 0 成功 其他 失败
    */
    int switchCamera(const char *device, const char *devName);

    /**
     * @brief getSaveFilePrefix
     * @return UOS_ 专业版 DEEPIN_ 社区版 CAMERA_ 其他
     */
    QString getSaveFilePrefix();
public:
    MajorImageProcessingThread *m_imgPrcThread;
    AudioProcessingThread      *m_audPrcThread;
private:
    bool                       m_bActive;           //是否录制中
    int                        m_nMaxContinuous;    //最大连拍数：0,4,10
    int                        m_curTakePicTime;    //当前连拍次数
    int                        m_Maxinterval;       //最大间隔
    int                        m_nMaxInterval;      //最大间隔：0,3,6
    int                        m_nInterval;         //当前间隔时间,初始化为0,按钮响应时赋值
    int                        m_nCount;            //录制计时
    int                        m_nFileID;           //文件id
    int                        m_nFastClick;        //快速点击次数，小于200ms计入
    int                        m_nMaxRecTime;       //最大录像时长  小时
    GridType                   m_GridType;          //网格线类型
    DLabel                     *m_flashLabel;       //闪光灯Label
    DLabel                     *m_dLabel;           //倒计时
    DLabel                     *m_recordingTimeWidget;//录制时长窗口
    DLabel                     *m_recordingTime;    //录制时长
    QString                    m_videoFormat;       //录制视频格式

    QSound                     *m_takePicSound;     //拍照声音
    QString                    m_savePicFolder;     //图片文件夹路径
    QString                    m_saveVdFolder;      //视频文件夹路径
    QTimer                     *m_countTimer;       //倒计时定时器
    QTimer                     *m_flashTimer;       //闪光灯定时器
    QTimer                     *m_recordingTimer;   //录制3秒后，每200ms设置一次时间
    QDateTime                  m_btnClickTime;      //按钮点击时间
    QGridLayout                *m_pGridLayout;
    QGraphicsViewEx            *m_pNormalView;
    QGraphicsScene             *m_pNormalScene;
    QGraphicsSvgItem           *m_pSvgItem;
    PreviewOpenglWidget        *m_openglwidget;     //opengl渲染窗口
    GridLineItem               *m_pGridLineItem;    //网格线图元
    GridLineWidget             *m_gridlinewidget;   //网格线部件

    QPixmap                    m_framePixmap;       //帧图片
    QGraphicsPixmapItem        *m_pNormalItem;

    QGraphicsTextItem          *m_pCamErrItem;      //摄像头异常提示
    bool                       m_flashEnable;       //是否闪光灯
    bool                       m_bPhoto = true;     //相机当前状态，默认为拍照状态

    GstVideoWriter             *m_videoWriter;      //基于GStreamer实现的视频帧数据写入器
    efilterType                m_filterType;        //当前选择的滤镜名称
    int                        m_exposure;

};

#endif // VIDEOWIDGET_H
