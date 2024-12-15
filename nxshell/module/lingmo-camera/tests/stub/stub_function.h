#ifndef STUB_FUNCTION_H
#define STUB_FUNCTION_H

#include <QObject>
#include <QCamera>
#include "../../src/devnummonitor.h"
#include "../../src/imageitem.h"
#include "datamanager.h"
#include "stub.h"
extern "C"
{
#include "v4l2_devices.h"
#include "camview.h"
#include "audio.h"
}
class Stub_Function
{
public:
    enum DeviceStatus {NOCAM, CAM_CANNOT_USE, CAM_CANUSE};
    Stub_Function();
    ~Stub_Function();
    //Setting
    v4l2_dev_t *get_v4l2_device_handler();
    v4l2_dev_t *get_v4l2_device_handler_none();
    int v4l2core_get_frame_format_index(v4l2_dev_t *vd, int format);
    int v4l2core_get_format_resolution_index(v4l2_dev_t *vd, int format, int width, int height);
    v4l2_stream_formats_t *v4l2core_get_formats_list(v4l2_dev_t *vd);
    int size();
    int toInt();

    v4l2_device_list_t m_devlist2;
    //thumbnailsbar
    enum DeviceStatus getdevStatus();
    qint64 msecsTo(const QDateTime &);

    //videowidget
//    int get_wayland_status();//获取wayland状态函数打桩
    bool isVisible();//可见函数打桩
    //调用delayInit分支
    int camInit_OK(const char *devicename);//返回值为E_OK(进入该分支会出现错误)
    int camInit_FORMAT_ERR(const char *devicename);//返回值为E_FORMAT_ERR
    int camInit_NO_DEVICE_ERR(const char *devicename);//返回值为E_NO_DEVICE_ERR
    //调用showCountdown分支
    bool isActive();//定时器使用中函数打桩
    bool isVisible_Countdown();//倒计时可见函数打桩
    bool isVisible_Label();//闪光灯Label可见函数打桩
    enum DeviceStatus getdevStatus_CAM_CANUSE();//获取相机可用状态函数打桩
    //调用showRecTime
    //获取写video的时间m_nCount打桩函数
    double get_video_time_capture_hour_5();//0时0分5秒,小于3为不正常时长
    double get_video_time_capture_hour_60();//0时1分0秒,调用秒数为0分支
    double get_video_time_capture_hour_21966();//6时6分6秒
    double get_video_time_capture_hour_40271();//11时11分11秒
    //调用onEndBtnClicked
    int video_capture_get_save_video();
    int video_capture_get_save_video_no_capture();

    v4l2_device_list_t *get_device_list_0();
    //获取设备列表，进入设备数目为1分支
    v4l2_device_list_t *get_device_list_1();
    //获取设备列表，进入设备数目为2分支
    v4l2_device_list_t *get_device_list_2();
    //获取设备列表，进入设备数目为3分支
    v4l2_device_list_t *get_device_list_3();
    //开启线程打桩
    int start_encoder_thread();
    //MajorImageProcessingThread开始函数打桩
    void start();

    //MajorImageProcessingThread
    //开始启动流
    int v4l2core_start_stream(v4l2_dev_t *vd);
    //停止启动流
    int v4l2core_stop_stream(v4l2_dev_t *vd);
    //获得解码数据库帧
    v4l2_frame_buff_t *v4l2core_get_decoded_frame(v4l2_dev_t *vd);

    //获得解码数据库帧
    v4l2_frame_buff_t *v4l2core_get_decoded_frame_none(v4l2_dev_t *vd);

    v4l2_frame_buff_t *v4l2core_get_decoded_frame_changed(v4l2_dev_t *vd);
    //获取重启状态
    int get_resolution_status();
    //清理缓存区
    void v4l2core_clean_buffers(v4l2_dev_t *vd);
    //更新当前格式
    int v4l2core_update_current_format_OK(v4l2_dev_t *vd);//返回零
    int v4l2core_update_current_format_Not_OK(v4l2_dev_t *vd);//返回非零
    //释放帧数据
    int v4l2core_release_frame(v4l2_dev_t *vd, v4l2_frame_buff_t *frame);
    //有效格式
    void v4l2core_prepare_valid_format(v4l2_dev_t *vd);
    //宽度高度获取
    void v4l2core_prepare_valid_resolution(v4l2_dev_t *vd);
    //设置设备名
    void set_device_name(const char *name);
    //进入V4L2_PIX_FMT_H264分支
    int v4l2core_get_requested_frame_format_264(v4l2_dev_t *vd);
    //返回yuv
    int v4l2core_get_requested_frame_format_yuv(v4l2_dev_t *vd);
    //获得暂停标志
    int get_capture_pause_true();

    int get_capture_pause_false();
    //编码器任务调度
    double encoder_buff_scheduler(int mode, double thresh, double max_time);

    double encoder_buff_scheduler_one(int mode, double thresh, double max_time);
    //h264帧率设置
    int v4l2core_set_h264_frame_rate_config(v4l2_dev_t *vd, uint32_t framerate);

    //获取音频上下文
    audio_context_t *get_audio_context();
    int audio_start(audio_context_t *audio_ctx);
    int audio_stop(audio_context_t *audio_ctx);
    int audio_get_next_buffer(audio_context_t *audio_ctx, audio_buff_t *buff, int type, uint32_t mask);

    //mainwindow
    //返回不存在的路径
    QVariant toString();
    //返回创建路径失败
    bool mkdir(const QString &dirName);
    //获得当前路径，此处需要获得一个以～开头的的路径
    static QString currentPath();

    //imageitem
    QString suffix();//获得文件后缀函数打桩
    bool parseFromFile();//解析文件函数打桩

    int config_load(const char *filename);
    void v4l2core_init();
    v4l2_dev_t *create_v4l2_device_handler(const char *device);
    int camInit(const char *devicename);
    int camInitFormatError(const char *devicename);
    int camInitNoDevice(const char *devicename);
    int check_device_list_events(v4l2_dev_t *vd);
    void devnumMonitorStartCheck();
    int get_video_codec_ind();

    // Camera类相关桩函数--------------------begin
    QList<QSize> getSupportResolutionsSize();
    QCamera::Status cameraStatus();
    DeviceStatus getNoDevStatus();
    bool videoFrameMapReadOnly(QAbstractVideoBuffer::MapMode mode);
    EncodeEnv qCameraEnv();
    EncodeEnv gstreamerEnv();
    int recordingState();
    // Camera类相关桩函数--------------------end

    //重置单个桩函数
    template<typename T,typename S>
    static void resetSub(T addr, S addr_stub)
    {
        m_stub.set(addr, addr_stub);
    }

    template<typename T>
    static void clearSub(T addr)
    {
        m_stub.reset(addr);
    }

    //初始化
    static void init();
    //release all statuc members
    static void release();

    //初始化所有的桩函数
    static void initSub();
private:
    //定义静态成员变量用于打桩时多次调用
    static v4l2_dev_t *m_v4l2_dev;//设备属性
    static v4l2_stream_formats_t *m_list_stream_formats;//流格式列表
    static v4l2_device_list_t *m_v4l2_device_list0;//一个摄像头
    static v4l2_device_list_t *m_v4l2_device_list1;//一个摄像头
    static v4l2_device_list_t *m_v4l2_device_list2;//两个摄像头
    static v4l2_device_list_t *m_v4l2_device_list3;//三个摄像头
    static v4l2_frame_buff_t *m_v4l2_frame_buff;//帧缓冲器
    static v4l2_frame_buff_t *m_v4l2_frame_buff2;//帧缓冲器
    static audio_context_t *m_audio_ctx;//音频上下文
    static Stub    m_stub;
};

#endif // STUB_FUNCTION_H
