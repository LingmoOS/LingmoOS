// Copyright (C) 2020 ~ now Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gstrecordx.h"
#include "utils.h"


/**
 * @brief gstBusMessageCb
 * @param bus:总线
 * @param msg:消息
 * @param userdata:用户数据
 * @return
 */
static gboolean gstBusMessageCb(GstBus *bus, GstMessage *message, GstRecordX *gstRecord)
{
    Q_UNUSED(bus);
    if (!message) {
        return true;
    }
    switch (message->type) {
    case GST_MESSAGE_ERROR: {

        GError *error = nullptr;
        gchar *dbg = nullptr;
        gstInterface::m_gst_message_parse_error(message, &error, &dbg);
        if (dbg) {
            gstInterface::m_g_free(dbg);
        }
        if (error) {
            QString errMsg = error->message;
            qCritical() << "Gstreamer pipeline error: " << errMsg;
            gstInterface::m_g_error_copy(error);
        }
        break;
    }
    case GST_MESSAGE_EOS: {
        gstInterface::m_g_main_loop_quit(gstRecord->getGloop());
        break;
    }
    default:
        break;
    }
    return true;
}

GstRecordX::GstRecordX(QObject *parent) : QObject(parent)
{
    initMemberVariables();
}

//初始化成员变量
void GstRecordX::initMemberVariables()
{
    m_pipeline = nullptr;
    m_audioType = AudioType::None;
    m_videoType = VideoType::webm;
    m_sysDevcieName = "";
    m_micDeviceName = "";
    m_savePath = "";
    m_framerate = 24;
    m_recordArea = QRect(0, 0, 0, 0);
    m_channels = 2;
    m_rate = 44100;
    m_boardVendorType = 0;
}

//x11协议下gstreamer录制视频
void GstRecordX::x11GstStartRecord()
{
    QStringList arguments;
    QStringList areaList;

    //设置录制区域的大小及位置 show-pointer:是否录制光标
    areaList << "ximagesrc"
             << "display-name=" + qgetenv("DISPLAY")
             << "use-damage=false"
             << "show-pointer=" + m_isRecordMouse //是否录制光标
             << "startx=" + QString::number(m_recordArea.x())
             << "starty=" + QString::number(m_recordArea.y())
             << "endx=" + QString::number(m_recordArea.x() + m_recordArea.width() - 1)
             << "endy=" + QString::number(m_recordArea.y() + m_recordArea.height() - 1);
    arguments << areaList.join(" ");

    //设置录屏的帧率
    arguments << QString("video/x-raw, framerate=%1/1").arg(m_framerate);
    //设置视频转换器
    arguments << "videoconvert";
    arguments << "videorate";
    arguments << "queue max-size-bytes=1073741824 max-size-time=10000000000 max-size-buffers=1000";

    //创建管道
    if (createPipeline(arguments)) {
        if (nullptr == m_pipeline) {
            qCritical() << "Error: Gstreamer's Pipeline create failure!";
            return;
        }
        qInfo() << "Gstreamer's Pipeline create successfully!";
        //启动Gstreamer录屏管道
        GstStateChangeReturn ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qWarning() << "Unable to set the pipeline to the playing state. Recording is failure";
            gstInterface::m_gst_object_unref(m_pipeline);
            return;
        }
        qInfo() << "(x11) Gstreamer's Pipeline starup successfully!";
    } else {
        qCritical() << "Error: Gstreamer's Pipeline create failure!";
    }
}

//x11协议下gstreamer停止录制视频
void GstRecordX::x11GstStopRecord()
{
    if (!m_pipeline) {
        qWarning() << "wayland Gstreamer 录屏未能正常结束！录屏管道被提前释放！";
        return;
    }
    stopPipeline();
    qInfo() << "x11 Gstreamer 录屏结束！";
}

//wayland协议下gstreamer录制管道构建及启动录制视频
void GstRecordX::waylandGstStartRecord()
{
    QStringList wlarguments;
    wlarguments << "appsrc name=videoSrc";
    //此处需特别注意视频帧的格式，由于在wayland协议上，采集的画面，hw机和普通机器的像素格式有差异
    if (m_boardVendorType) {
        wlarguments << QString("video/x-raw, format=RGB, framerate=%1/1, width=%2, height=%3").arg(m_framerate).arg(m_recordArea.width()).arg(m_recordArea.height());
    } else {
        wlarguments << QString("video/x-raw, format=RGBA, framerate=%1/1, width=%2, height=%3").arg(m_framerate).arg(m_recordArea.width()).arg(m_recordArea.height());
    }
    wlarguments << "queue ! videoconvert primaries-mode=2 name=convert ! queue";

    //创建管道
    if (createPipeline(wlarguments)) {
        if (nullptr == m_pipeline) {
            qCritical() << "Error: Gstreamer's Pipeline create failure!";
            return;
        }
        qInfo() << "Gstreamer's Pipeline create successfully!";
        //添加视频相关流信息
        GstElement *videoSrc = gstInterface::m_gst_bin_get_by_name(getGstBin(m_pipeline), "videoSrc");
        gstInterface::m_g_object_set(videoSrc, "format", GST_FORMAT_TIME, NULL);
        gstInterface::m_g_object_set(videoSrc, "is-live", TRUE, NULL);
        m_gloop = gstInterface::m_g_main_loop_new(NULL, TRUE);

        GstBus *bus = gstInterface::m_gst_pipeline_get_bus(reinterpret_cast<GstPipeline *>(m_pipeline));
        gstInterface::m_gst_bus_add_watch(bus, reinterpret_cast<GstBusFunc>(gstBusMessageCb), this);
        gstInterface::m_gst_object_unref(bus);

        //启动Gstreamer录屏管道
        GstStateChangeReturn ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            qWarning() << "Unable to set the pipeline to the playing state. Recording is failure";
            gstInterface::m_gst_object_unref(m_pipeline);
            return;
        }
        QtConcurrent::run(gstInterface::m_g_main_loop_run, m_gloop);
        qInfo() << "(Wayland) Gstreamer's Pipeline starup successfully!";
    } else {
        qCritical() << "Error: Gstreamer's Pipeline create failure!";
    }
}

//wayland协议下gstreamer停止录制视频
void GstRecordX::waylandGstStopRecord()
{
    if (!m_pipeline) {
        qWarning() << "wayland Gstreamer 录屏未能正常结束！录屏管道被提前释放！";
        return;
    }

    GstElement *videoSrc = gstInterface::m_gst_bin_get_by_name(getGstBin(m_pipeline), "videoSrc");
    GstFlowReturn ret = GST_FLOW_NOT_LINKED;
    if (videoSrc) {
        gstInterface::m_g_signal_emit_by_name(videoSrc, "end-of-stream", &ret);
    }
    if (GST_FLOW_OK == ret) {
        qInfo() << "(wayland Gstreamer) Stop writing video data";
    } else {
        qInfo() << "(wayland Gstreamer) Stopping video data writing failed! Gstreamer internal Error Code: " << ret;
    }
    stopPipeline();
    //发射wayland gstreamer录屏完成信号
    emit waylandGstRecrodFinish();
    qInfo() << "wayland Gstreamer 录屏结束！";
}

//wayland下写入视频帧
bool GstRecordX::waylandWriteVideoFrame(const unsigned char *frame, const int framewidth, const int frameheight)
{
    if (!m_pipeline) {
        qWarning() << "wayland Gstreamer 写入视频帧失败！录屏管道未初始化！";
        return false;
    }

    //qDebug() << " ======= 取视频帧进行编码 ";

    GstFlowReturn ret;
    guint8 *ptr;
    int size = m_recordArea.width() * m_recordArea.height() * 4;
    ptr = (guint8 *)gstInterface::m_g_malloc(size * sizeof(uchar));
    if (nullptr == ptr) {
        qWarning("GStreamerRecorder::writeFrame malloc failed!");
        return false;
    } else {
        GstBuffer *buffer;
        QImage img(frame, framewidth, frameheight, QImage::Format::Format_RGBA8888);
        img = img.copy(m_recordArea);
        memcpy(ptr, img.bits(), size);
        buffer = gstInterface::m_gst_buffer_new_wrapped((void *)ptr, size);

//        QImage *img = new QImage(frame, m_recordArea.width(), m_recordArea.height(), QImage::Format::Format_RGBA8888);
//        img->save(QString("/home/uos/Desktop/test/image/test_%1.png").arg(globalCount));
//        globalCount++;

        //设置时间戳
        GST_BUFFER_PTS(buffer) = gstInterface::m_gst_clock_get_time(m_pipeline->clock) - m_pipeline->base_time;
        //获取视频源
        GstElement *videoSrc = gstInterface::m_gst_bin_get_by_name(getGstBin(m_pipeline), "videoSrc");
        //注入视频帧数据
        gstInterface::m_g_signal_emit_by_name(videoSrc, "push-buffer", buffer, &ret);
        //释放buffer
        //        gstInterface::m_gst_buffer_unref(buffer);
        gstInterface::m_gst_mini_object_unref(GST_MINI_OBJECT_CAST(buffer));
    }

    return ret == GST_FLOW_OK;
}

//设置输入设备名称
void GstRecordX::setInputDeviceName(const QString &device)
{
    //即使为空，也不影响功能，设备名称不会用作判断是否采集音频，只是存储设备名称，判断是否采集有另外的字段
//    if (device.isEmpty() || device == "") {
//        qInfo() << "devcieType: " << type << " current audio device name is empty!";
//    }
    m_micDeviceName = device;
}

//设置输出设备名称
void GstRecordX::setOutputDeviceName(const QString &device)
{
    //即使为空，也不影响功能，设备名称不会用作判断是否采集音频，只是存储设备名称，判断是否采集有另外的字段
//    if (device.isEmpty() || device == "") {
//        qInfo() << "devcieType: " << type << " current audio device name is empty!";
//    }
    m_sysDevcieName = device;
}

//设置音频类型
void GstRecordX::setAudioType(GstRecordX::AudioType audioType)
{
    m_audioType = audioType;
}

//设置视频类型
void GstRecordX::setVidoeType(GstRecordX::VideoType videoType)
{
    m_videoType = videoType;
}

//设置视频帧率
void GstRecordX::setFramerate(const int &framerate)
{
    m_framerate = framerate;
}

//设置录制区域的位置及大小
void GstRecordX::setRecordArea(const QRect &recordArea)
{
    m_recordArea = recordArea;
}

//设置保存路径
void GstRecordX::setSavePath(const QString &savePath)
{
    m_savePath = savePath;
}

//设置x11录制光标
void GstRecordX::setX11RecordMouse(const bool recordMouse)
{
    m_isRecordMouse =  recordMouse ? "true" : "false";
}

//设置主板供应商类型
void GstRecordX::setBoardVendorType(int boardVendorType)
{
    m_boardVendorType = boardVendorType;
}

//创建Gstreamer录屏管道，这部分x11和wayland可共用
bool GstRecordX::createPipeline(QStringList arguments)
{
    //设置编码器
    // arguments << QString("vp8enc min_quantizer=20 max_quantizer=20 cpu-used=%1 deadline=1000000 threads=%2").arg(QThread::idealThreadCount()).arg(QThread::idealThreadCount());
    if (m_videoType == VideoType::webm) {
        //vp8编码
        arguments << "vp8enc min-quantizer=1 max-quantizer=50 undershoot=95 cpu-used=5 deadline=1 static-threshold=50 error-resilient=1";
    } else if (m_videoType == VideoType::ogg) {
        //ogg不支持vp8编码，需使用theora编码，该编码器基于vp3,编码效率低，生成的视频容存在时长不正确的问题
        arguments << "theoraenc  bitrate=2200 drop-frames=false keyframe-auto=false keyframe-force=5 keyframe-freq=5";
    } else {
        arguments << "vp8enc min-quantizer=1 max-quantizer=50 undershoot=95 cpu-used=5 deadline=1 static-threshold=50 error-resilient=1";
    }
    arguments << "queue";
    arguments << "mux.";

    //混音模式
    if (m_audioType == AudioType::Mix) {
        //为保证Gstreamer管道顺利执行，做了规避处理，遇到音频设备名称不存在的情况下，输出错误日志，不在管道命令中添加音频混流器
        if (m_sysDevcieName.isEmpty() && m_micDeviceName.isEmpty()) {
            qCritical() << "Error: The system audio device name and mic audio device name is empty in Mix audio!";
        } else {
            //音频的混流器
            arguments << "adder name=mix ! queue ! audiorate ! audioconvert ! vorbisenc ! mux.";

            //添加系统音频
            arguments << getAudioPipeline(m_sysDevcieName, "sys", "mix");
            //添加麦克风音频
            arguments << getAudioPipeline(m_micDeviceName, "mic", "mix");
        }
    } else if (m_audioType == AudioType::Mic) { //单麦克风音频模式
        //为保证Gstreamer管道顺利执行，做了规避处理，遇到音频设备名称不存在的情况下，输出错误日志，不在管道命令中添加音频,录制无声视频
        arguments << getAudioPipeline(m_micDeviceName, "mic", "mux");
    } else if (m_audioType == AudioType::Sys) { //单系统音频模式
        //为保证Gstreamer管道顺利执行，做了规避处理，遇到音频设备名称不存在的情况下，输出错误日志，不在管道命令中添加音频,录制无声视频
        arguments << getAudioPipeline(m_sysDevcieName, "sys", "mux");
    }

    if (m_videoType == VideoType::webm) {
        //webmmux 复用器
        arguments << "webmmux name=mux";
    } else if (m_videoType == VideoType::ogg) {
        arguments << "oggmux name=mux";
    } else {
        qWarning() << "the recordType is Error! set default recordtype webm";
        arguments << "webmmux name=mux";
    }
    //文件路径
    arguments << "filesink location=" + m_savePath;
    arguments.removeAll("");
    //转换未字符串
    QString recordArg = arguments.join(" ! ");
    recordArg = recordArg.replace("mix. !", "mix.");
    recordArg = recordArg.replace("mux. !", "mux.");
    //格式化输出Gstreamer管道构建命令行
    pipelineStructuredOutput(recordArg);

    QByteArray byteArray = recordArg.toUtf8();
    const gchar *line = byteArray.constData();
    GError *error = Q_NULLPTR;

    //解析gstreamer管道命令
    m_pipeline = gstInterface::m_gst_parse_launch(line, &error);
    if (error != Q_NULLPTR) {
        qCritical() << "gstreamer pares error: " << error->code << " , " << error->message;
        return false;
    }
    return true;
}

//根据传入的参数获取，音频管道创建命令
QString GstRecordX::getAudioPipeline(const QString &audioDevName, const QString &audioType, const QString &arg)
{
    QString result = "";
    //判断设备名称是否为空，为空不会生成管道命令
    if (audioDevName.isEmpty()) {
        qCritical() << "Error: The " << audioType << " audio device name is empty! (" << arg << ")";
    } else {
        //添加pulse源，采集设备的声音
        result = "pulsesrc device=" + audioDevName + " ! queue ! audioconvert ! audioresample ! ";
        //添加管道中流媒体格式
        result += QString("capsfilter caps=audio/x-raw,rate=%1,channels=%2").arg(m_rate).arg(m_channels);
        //根据传入的参数，生成不同的处理器（音频混流器或复用器）
        if (arg == "mix") {
            result += " ! queue ! mix.";
        } else if (arg == "mux") {
            result += " ! vorbisenc ! queue ! mux.";
        } else {
            qCritical() << "Error: Parameter that does not exist!";
            result = "";
        }
    }
    return result;
}

//停止管道，x11和wayland可共用
void GstRecordX::stopPipeline()
{
    bool a = gstInterface::m_gst_element_send_event(m_pipeline, gstInterface::m_gst_event_new_eos());
    Q_UNUSED(a);

    GstClockTime timeout = 5 * GST_SECOND;
    GstMessage *msg = gstInterface::m_gst_bus_timed_pop_filtered(GST_ELEMENT_BUS(m_pipeline), timeout, GST_MESSAGE_EOS);
    Q_UNUSED(msg);

    GstStateChangeReturn ret ;
    Q_UNUSED(ret);
    ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    Q_UNUSED(ret);
    ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_READY);
    Q_UNUSED(ret);
    ret = gstInterface::m_gst_element_set_state(m_pipeline, GST_STATE_NULL);
    Q_UNUSED(ret);
    gstInterface::m_gst_object_unref(m_pipeline);
}


//格式化输出gstreamer命令
void GstRecordX::pipelineStructuredOutput(QString pipeline)
{
    QString string;
    QString nl;
    nl = "\\";
    pipeline.prepend("gst-launch-1.0 -e --gst-debug-level=2" + nl + "\n    ");
    pipeline.replace("mux.", "mux. " + nl + "\n   ");
    pipeline.replace("mix.", "mix. " + nl + "\n   ");
    string = pipeline.replace("!", nl + "\n        !");
    string.append("\n");
    //    printf("%s\n", string.toLocal8Bit().data());
    qInfo() << "\n" << string.toLocal8Bit().data();
}

GstBin *GstRecordX::getGstBin(GstElement *element)
{
    return (GstBin *)gstInterface::m_g_type_check_instance_cast((GTypeInstance *)element, gstInterface::m_gst_bin_get_type());
}
GstRecordX::~GstRecordX()
{
    if (m_pipeline) {
        m_pipeline = nullptr;
        delete m_pipeline;
    }

}
