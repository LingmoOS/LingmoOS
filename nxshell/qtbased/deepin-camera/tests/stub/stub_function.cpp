#include "stub_function.h"
#include "stub.h"
extern "C"
{
#include "v4l2_devices.h"
#include "camview.h"
}

v4l2_dev_t *Stub_Function::m_v4l2_dev =  nullptr;//设备属性
v4l2_stream_formats_t *Stub_Function::m_list_stream_formats =  nullptr;//流格式列表
v4l2_device_list_t *Stub_Function::m_v4l2_device_list0 =  nullptr;//零个摄像头
v4l2_device_list_t *Stub_Function::m_v4l2_device_list1 =  nullptr;//一个摄像头
v4l2_device_list_t *Stub_Function::m_v4l2_device_list2 =  nullptr;//两个摄像头
v4l2_device_list_t *Stub_Function::m_v4l2_device_list3 =  nullptr;//三个摄像头
v4l2_frame_buff_t *Stub_Function::m_v4l2_frame_buff =  nullptr;//帧缓冲器
v4l2_frame_buff_t *Stub_Function::m_v4l2_frame_buff2 =  nullptr;//帧缓冲器
audio_context_t *Stub_Function::m_audio_ctx = nullptr;//音频上下文
Stub    Stub_Function::m_stub;


Stub_Function::Stub_Function()
{

}

Stub_Function::~Stub_Function()
{

}

//Settings
v4l2_dev_t *Stub_Function::get_v4l2_device_handler()
{
    return m_v4l2_dev;
}

v4l2_dev_t *Stub_Function::get_v4l2_device_handler_none()
{
    return nullptr;
}

int Stub_Function::v4l2core_get_frame_format_index(v4l2_dev_t *vd, int format)
{
    return 0;
}
int Stub_Function::v4l2core_get_format_resolution_index(v4l2_dev_t *vd, int format, int width, int height)
{
    return 0;
}

v4l2_stream_formats_t *Stub_Function::v4l2core_get_formats_list(v4l2_dev_t *vd)
{
    return m_list_stream_formats;
}

int Stub_Function::size()
{
    return 2;
}

int Stub_Function::toInt()
{
    return v4l2core_get_frame_width(get_v4l2_device_handler());
}

//thumbnailsbar
Stub_Function::DeviceStatus Stub_Function::getdevStatus()
{
    return CAM_CANUSE;
}

qint64 Stub_Function::msecsTo(const QDateTime &)
{
    return 0;
}

//int Stub_Function::get_wayland_status()
//{
//    return true;
//}

bool Stub_Function::isVisible()
{
    return true;
}

int Stub_Function::camInit_OK(const char *devicename)
{
    return E_OK;
}

int Stub_Function::camInit_FORMAT_ERR(const char *devicename)
{
    return E_FORMAT_ERR;
}

int Stub_Function::camInit_NO_DEVICE_ERR(const char *devicename)
{
    return E_NO_DEVICE_ERR;
}

bool Stub_Function::isActive()
{
    return true;
}

bool Stub_Function::isVisible_Countdown()
{
    return true;
}

bool Stub_Function::isVisible_Label()
{
    return true;
}

Stub_Function::DeviceStatus Stub_Function::getdevStatus_CAM_CANUSE()
{
    return CAM_CANUSE;
}

double Stub_Function::get_video_time_capture_hour_5()
{
    return 5;
}

double Stub_Function::get_video_time_capture_hour_60()
{
    return 60;
}

double Stub_Function::get_video_time_capture_hour_21966()
{
    return 21966;
}

double Stub_Function::get_video_time_capture_hour_40271()
{
    return 40271;
}

int Stub_Function::video_capture_get_save_video()
{
    return 1;
}

int Stub_Function::video_capture_get_save_video_no_capture()
{
    return 0;
}

v4l2_device_list_t *Stub_Function::get_device_list_0()
{
    return m_v4l2_device_list0;
}

v4l2_device_list_t *Stub_Function::get_device_list_1()
{
    return m_v4l2_device_list1;
}

v4l2_device_list_t *Stub_Function::get_device_list_2()
{
    return m_v4l2_device_list2;

}

v4l2_device_list_t *Stub_Function::get_device_list_3()
{
    return m_v4l2_device_list3;

}

int Stub_Function::start_encoder_thread()
{
    return 0;
}

void Stub_Function::start()
{

}

int Stub_Function::v4l2core_start_stream(v4l2_dev_t *vd)
{
    return 0;
}

int Stub_Function::v4l2core_stop_stream(v4l2_dev_t *vd)
{
    return 0;
}

v4l2_frame_buff_t *Stub_Function::v4l2core_get_decoded_frame(v4l2_dev_t *vd)
{
    return m_v4l2_frame_buff;
}

v4l2_frame_buff_t *Stub_Function::v4l2core_get_decoded_frame_none(v4l2_dev_t *vd)
{
    return nullptr;
}

v4l2_frame_buff_t *Stub_Function::v4l2core_get_decoded_frame_changed(v4l2_dev_t *vd)
{
    return m_v4l2_frame_buff2;
}

int Stub_Function::get_resolution_status()
{
    return 1;
}

void Stub_Function::v4l2core_clean_buffers(v4l2_dev_t *vd)
{

}

int Stub_Function::v4l2core_update_current_format_OK(v4l2_dev_t *vd)
{
    return 0;
}

int Stub_Function::v4l2core_update_current_format_Not_OK(v4l2_dev_t *vd)
{
    return -10;
}

int Stub_Function::v4l2core_release_frame(v4l2_dev_t *vd, v4l2_frame_buff_t *frame)
{
    return 0;
}

void Stub_Function::v4l2core_prepare_valid_format(v4l2_dev_t *vd)
{

}

void Stub_Function::v4l2core_prepare_valid_resolution(v4l2_dev_t *vd)
{

}

void Stub_Function::set_device_name(const char *name)
{

}

int Stub_Function::v4l2core_get_requested_frame_format_264(v4l2_dev_t *vd)
{
    return V4L2_PIX_FMT_H264;
}

int Stub_Function::v4l2core_get_requested_frame_format_yuv(v4l2_dev_t *vd)
{
    return V4L2_PIX_FMT_NV12;
}

int Stub_Function::get_capture_pause_true()
{
    return 1;
}

int Stub_Function::get_capture_pause_false()
{
    return 0;
}

double Stub_Function::encoder_buff_scheduler(int mode, double thresh, double max_time)
{
    return 0;
}

double Stub_Function::encoder_buff_scheduler_one(int mode, double thresh, double max_time)
{
    return 1;
}

int Stub_Function::v4l2core_set_h264_frame_rate_config(v4l2_dev_t *vd, uint32_t framerate)
{
    return 0;
}

audio_context_t *Stub_Function::get_audio_context()
{
    return m_audio_ctx;
}

int Stub_Function::audio_start(audio_context_t *audio_ctx)
{
    return 0;
}

int Stub_Function::audio_stop(audio_context_t *audio_ctx)
{
    return 0;
}

int Stub_Function::audio_get_next_buffer(audio_context_t *audio_ctx, audio_buff_t *buff, int type, uint32_t mask)
{
    static int time = 0;
    time++;
    if (30 == time)
        return 0;

    return 1;
}

QVariant Stub_Function::toString()
{
    return "/a";
}

bool Stub_Function::mkdir(const QString &dirName)
{
    return false;
}

QString Stub_Function::currentPath()
{
    return "~/a";
}

QString Stub_Function::suffix()
{
    return "webm";
}

bool Stub_Function::parseFromFile()
{
    return true;
}

int Stub_Function::config_load(const char *filename)
{
    (void*)filename;
    return 1;
}

void Stub_Function::v4l2core_init()
{
}

v4l2_dev_t *Stub_Function::create_v4l2_device_handler(const char *device)
{
    (void*)device;
    return m_v4l2_dev;
}

int Stub_Function::camInit(const char *devicename)
{
    (void*)devicename;
    return 0;
}

int Stub_Function::camInitFormatError(const char *devicename)
{
    (void*)devicename;
    return E_FORMAT_ERR;
}

int Stub_Function::camInitNoDevice(const char *devicename)
{
    (void*)devicename;
    return E_NO_DEVICE_ERR;
}

int Stub_Function::check_device_list_events(v4l2_dev_t *vd)
{
    (void*)vd;
    return 0;
}

void Stub_Function::devnumMonitorStartCheck()
{

}

int Stub_Function::get_video_codec_ind()
{
    return 0;
}

QList<QSize> Stub_Function::getSupportResolutionsSize()
{
    QList<QSize> resolutions;
    resolutions << QSize(800,600);
    return resolutions;
}

QCamera::Status Stub_Function::cameraStatus()
{
    return QCamera::UnavailableStatus;
}

Stub_Function::DeviceStatus Stub_Function::getNoDevStatus()
{
    return NOCAM;
}

bool Stub_Function::videoFrameMapReadOnly(QAbstractVideoBuffer::MapMode mode)
{
    return true;
}

EncodeEnv Stub_Function::qCameraEnv()
{
    return QCamera_Env;
}

EncodeEnv Stub_Function::gstreamerEnv()
{
    return GStreamer_Env;
}

int Stub_Function::recordingState()
{
    return 1;
}

void Stub_Function::init()
{
    if (nullptr == m_audio_ctx) {
        m_audio_ctx = (audio_context_t*)malloc(sizeof(audio_context_t));
        memset(m_audio_ctx, 0, sizeof(audio_context_t));
        m_audio_ctx->channels = 2;
    }

    if (m_v4l2_frame_buff == nullptr) {
        m_v4l2_frame_buff = (v4l2_frame_buff_t *)malloc(sizeof(v4l2_frame_buff_t));
        memset(m_v4l2_frame_buff, 0, sizeof(v4l2_frame_buff_t));

        m_v4l2_frame_buff->width = 1920;
        m_v4l2_frame_buff->height = 1080;
        uint yuvsize = m_v4l2_frame_buff->width * m_v4l2_frame_buff->height * 3 / 2;
        m_v4l2_frame_buff->yuv_frame = (uint8_t *)malloc(size_t(yuvsize));
    }

    if (m_v4l2_frame_buff2 == nullptr) {
        m_v4l2_frame_buff2 = (v4l2_frame_buff_t *)malloc(sizeof(v4l2_frame_buff_t));
        memset(m_v4l2_frame_buff2, 0, sizeof(v4l2_frame_buff_t));

        m_v4l2_frame_buff2->width = 1280;
        m_v4l2_frame_buff2->height = 720;
        uint yuvsize = m_v4l2_frame_buff2->width * m_v4l2_frame_buff2->height * 3 / 2;
        m_v4l2_frame_buff2->yuv_frame = (uint8_t *)malloc(size_t(yuvsize));
    }

    if (m_v4l2_device_list3 == nullptr) {
        m_v4l2_device_list3 = (v4l2_device_list_t *)malloc(sizeof(v4l2_device_list_t));
        m_v4l2_device_list3->list_devices = new v4l2_dev_sys_data_t[3];
        m_v4l2_device_list3->num_devices = 3;
        m_v4l2_device_list3->list_devices[0].device = strdup("/dev/video0");
        m_v4l2_device_list3->list_devices[1].device = strdup("/dev/video1");
        m_v4l2_device_list3->list_devices[2].device = strdup("/dev/video2");
        m_v4l2_device_list3->list_devices[0].name = strdup("/dev/video0");
        m_v4l2_device_list3->list_devices[1].name = strdup("/dev/video1");
        m_v4l2_device_list3->list_devices[2].name = strdup("/dev/video2");
        m_v4l2_device_list3->list_devices[0].driver= strdup("/dev/video0");
        m_v4l2_device_list3->list_devices[1].driver = strdup("/dev/video1");
        m_v4l2_device_list3->list_devices[2].driver = strdup("/dev/video2");
        m_v4l2_device_list3->list_devices[0].location= strdup("/dev/video0");
        m_v4l2_device_list3->list_devices[1].location = strdup("/dev/video1");
        m_v4l2_device_list3->list_devices[2].location = strdup("/dev/video2");
    }

    if (m_v4l2_device_list2 == nullptr) {
        m_v4l2_device_list2 = (v4l2_device_list_t *)malloc(sizeof(v4l2_device_list_t));
        m_v4l2_device_list2->list_devices = new v4l2_dev_sys_data_t[2];
        m_v4l2_device_list2->num_devices = 2;
        m_v4l2_device_list2->list_devices[0].device = strdup("/dev/video0");
        m_v4l2_device_list2->list_devices[1].device = strdup("/dev/video1");
        m_v4l2_device_list2->list_devices[0].name = strdup("/dev/video0");
        m_v4l2_device_list2->list_devices[1].name = strdup("/dev/video1");
        m_v4l2_device_list2->list_devices[0].driver= strdup("/dev/video0");
        m_v4l2_device_list2->list_devices[1].driver = strdup("/dev/video1");
        m_v4l2_device_list2->list_devices[0].location= strdup("/dev/video0");
        m_v4l2_device_list2->list_devices[1].location = strdup("/dev/video1");
    }

    if (m_v4l2_device_list1 == nullptr) {
        m_v4l2_device_list1 = (v4l2_device_list_t *)malloc(sizeof(v4l2_device_list_t));
        m_v4l2_device_list1->list_devices = new v4l2_dev_sys_data_t[1];
        m_v4l2_device_list1->num_devices = 1;
        m_v4l2_device_list1->list_devices[0].device = strdup("/dev/video0");
        m_v4l2_device_list1->list_devices[0].name = strdup("/dev/video0");
        m_v4l2_device_list1->list_devices[0].driver= strdup("/dev/video0");
        m_v4l2_device_list1->list_devices[0].location= strdup("/dev/video0");
    }

    if (nullptr == m_v4l2_device_list0){
        m_v4l2_device_list0 = (v4l2_device_list_t *)malloc(sizeof(v4l2_device_list_t));
        memset(m_v4l2_device_list0, 0, sizeof(v4l2_device_list_t));
    }

    if (m_list_stream_formats == nullptr) {
        m_list_stream_formats = (v4l2_stream_formats_t *)malloc(sizeof(v4l2_stream_formats_t));
        m_list_stream_formats[0].numb_res = 2;
        m_list_stream_formats[0].list_stream_cap = (v4l2_stream_cap_t *)malloc(sizeof(v4l2_stream_formats_t));
        m_list_stream_formats[0].list_stream_cap[0].width = 1920;
        m_list_stream_formats[0].list_stream_cap[0].height = 1080;
        m_list_stream_formats[0].list_stream_cap[1].width = 1280;
        m_list_stream_formats[0].list_stream_cap[1].height = 720;
    }

    if (m_v4l2_dev == nullptr) {
        m_v4l2_dev = (v4l2_dev_t *)malloc(sizeof(v4l2_dev_t));
        m_v4l2_dev->list_stream_formats = (v4l2_stream_formats_t *)malloc(sizeof(v4l2_stream_formats_t));
        m_v4l2_dev->list_stream_formats[0].list_stream_cap = (v4l2_stream_cap_t *)malloc(sizeof(v4l2_stream_cap_t));
        m_v4l2_dev->list_stream_formats[0].list_stream_cap[0].width = 640;
        m_v4l2_dev->list_stream_formats[0].list_stream_cap[0].width = 480;
        QString str1 = "/dev/video0";
        m_v4l2_dev->videodevice = (char *)malloc(strlen(str1.toLatin1().data()) + 1);
        strcpy(m_v4l2_dev->videodevice, str1.toLatin1().data());
        m_v4l2_dev->this_device = 0;
    }
}

void Stub_Function::release()
{
    if (m_v4l2_dev != nullptr) {
        free(m_v4l2_dev->list_stream_formats[0].list_stream_cap);
        free(m_v4l2_dev->list_stream_formats);
        free(m_v4l2_dev->videodevice);
        free(m_v4l2_dev);
        m_v4l2_dev = nullptr;
    }

    if (m_list_stream_formats != nullptr) {
        free(m_list_stream_formats[0].list_stream_cap);
        free(m_list_stream_formats);
        m_list_stream_formats = nullptr;
    }
    if (m_v4l2_device_list0){
        free(m_v4l2_device_list0);
    }

    if (m_v4l2_device_list1 != nullptr) {
        free(m_v4l2_device_list1->list_devices[0].device);
        free(m_v4l2_device_list1->list_devices[0].name);
        free(m_v4l2_device_list1->list_devices[0].driver);
        free(m_v4l2_device_list1->list_devices[0].location);
        delete []m_v4l2_device_list1->list_devices;
        free(m_v4l2_device_list1);
        m_v4l2_device_list1 = nullptr;
    }

    if (m_v4l2_device_list2 != nullptr) {
        free(m_v4l2_device_list2->list_devices[0].device);
        free(m_v4l2_device_list2->list_devices[0].name);
        free(m_v4l2_device_list2->list_devices[0].driver);
        free(m_v4l2_device_list2->list_devices[0].location);
        free(m_v4l2_device_list2->list_devices[1].device);
        free(m_v4l2_device_list2->list_devices[1].name);
        free(m_v4l2_device_list2->list_devices[1].driver);
        free(m_v4l2_device_list2->list_devices[1].location);
        delete []m_v4l2_device_list2->list_devices;
        free(m_v4l2_device_list2);
        m_v4l2_device_list2 = nullptr;
    }

    if (m_v4l2_device_list3 != nullptr) {
        free(m_v4l2_device_list3->list_devices[0].device);
        free(m_v4l2_device_list3->list_devices[0].name);
        free(m_v4l2_device_list3->list_devices[0].driver);
        free(m_v4l2_device_list3->list_devices[0].location);
        free(m_v4l2_device_list3->list_devices[1].device);
        free(m_v4l2_device_list3->list_devices[1].name);
        free(m_v4l2_device_list3->list_devices[1].driver);
        free(m_v4l2_device_list3->list_devices[1].location);
        free(m_v4l2_device_list3->list_devices[2].device);
        free(m_v4l2_device_list3->list_devices[2].name);
        free(m_v4l2_device_list3->list_devices[2].driver);
        free(m_v4l2_device_list3->list_devices[2].location);
        delete []m_v4l2_device_list3->list_devices;
        free(m_v4l2_device_list3);
        m_v4l2_device_list3 = nullptr;
    }

    if (m_v4l2_frame_buff != nullptr) {
        free(m_v4l2_frame_buff->yuv_frame);
        free(m_v4l2_frame_buff);
        m_v4l2_frame_buff = nullptr;
    }

    if (m_v4l2_frame_buff2 != nullptr) {
        free(m_v4l2_frame_buff2->yuv_frame);
        free(m_v4l2_frame_buff2);
        m_v4l2_frame_buff2 = nullptr;
    }

    if (m_audio_ctx != nullptr) {
        free(m_audio_ctx);
        m_audio_ctx = nullptr;
    }
}

//初始化所有桩函数，对libcam里的函数进行打桩
void Stub_Function::initSub()
{
    m_stub.set(::v4l2core_start_stream, ADDR(Stub_Function, v4l2core_start_stream));
    m_stub.set(::v4l2core_stop_stream, ADDR(Stub_Function, v4l2core_stop_stream));
    m_stub.set(::v4l2core_get_decoded_frame, ADDR(Stub_Function, v4l2core_get_decoded_frame));
    m_stub.set(::get_resolution_status, ADDR(Stub_Function, get_resolution_status));
    m_stub.set(::v4l2core_clean_buffers, ADDR(Stub_Function, v4l2core_clean_buffers));
    m_stub.set(::v4l2core_update_current_format, ADDR(Stub_Function, v4l2core_update_current_format_OK));
    m_stub.set(::v4l2core_prepare_valid_format, ADDR(Stub_Function, v4l2core_prepare_valid_format));
    m_stub.set(::v4l2core_prepare_valid_resolution, ADDR(Stub_Function, v4l2core_prepare_valid_resolution));
    m_stub.set(::get_v4l2_device_handler, ADDR(Stub_Function, get_v4l2_device_handler));
    m_stub.set(::set_device_name, ADDR(Stub_Function, set_device_name));
    m_stub.set(::v4l2core_release_frame, ADDR(Stub_Function, v4l2core_release_frame));
//    m_stub.set(::get_wayland_status, ADDR(Stub_Function, get_wayland_status));
    m_stub.set(::start_encoder_thread, ADDR(Stub_Function, start_encoder_thread));
    m_stub.set(::v4l2core_get_format_resolution_index,ADDR(Stub_Function, v4l2core_get_format_resolution_index));
    m_stub.set(::config_load, ADDR(Stub_Function, config_load));
    m_stub.set(::v4l2core_init, ADDR(Stub_Function, v4l2core_init));
    m_stub.set(::v4l2core_get_frame_format_index, ADDR(Stub_Function, v4l2core_get_frame_format_index));
    m_stub.set(::create_v4l2_device_handler, ADDR(Stub_Function, create_v4l2_device_handler));
    m_stub.set(::get_device_list, ADDR(Stub_Function, get_device_list_3));
    m_stub.set(::camInit, ADDR(Stub_Function, camInit));
    m_stub.set(::check_device_list_events, ADDR(Stub_Function, check_device_list_events));
    m_stub.set(::video_capture_get_save_video, ADDR(Stub_Function, video_capture_get_save_video));
    m_stub.set(::get_video_codec_ind, ADDR(Stub_Function, get_video_codec_ind));
    m_stub.set(::v4l2core_get_requested_frame_format, ADDR(Stub_Function, v4l2core_get_requested_frame_format_yuv));
    m_stub.set(::get_capture_pause, ADDR(Stub_Function, get_capture_pause_false));
    m_stub.set(::encoder_buff_scheduler, ADDR(Stub_Function, encoder_buff_scheduler));
    m_stub.set(::v4l2core_set_h264_frame_rate_config, ADDR(Stub_Function, v4l2core_set_h264_frame_rate_config));
}



