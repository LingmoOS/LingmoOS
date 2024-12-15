/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* Maintainer: shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>
#include "load_libs.h"
#include <encoder.h>
#include <libavformat/avformat.h>

void PrintError(){
    char *error;
    if ((error = dlerror()) != NULL)  {
        fprintf (stderr, "%s ", error);
    }
}
static LoadLibNames g_ldnames;
static LoadLibs *pLibs = NULL;
static LoadLibs *newClass(void)
{
    pLibs = (LoadLibs *)malloc(sizeof(LoadLibs));
//    RTLD_NOW：在dlopen返回前，解析出全部没有定义的符号，解析不出来返回NULL。
//    RTLD_LAZY：暂缓决定，等有需要时再解出符号
    void *handle = dlopen(g_ldnames.chAvcodec/*"libavcodec.so.58"*/,RTLD_LAZY);
    if (!handle) {
        PrintError();
    }

#if !LIBAVCODEC_VER_AT_LEAST(58,9)
    dlsym(handle, "avcodec_register_all");
    PrintError();
#endif

#if LIBAVCODEC_VER_AT_LEAST(57,64)
#else
    dlsym(handle, "avcodec_decode_video2");
    PrintError();
#endif

#if !LIBAVCODEC_VER_AT_LEAST(53,34)
    dlsym(handle, "avcodec_init");
    PrintError();
#endif


#if LIBAVCODEC_VER_AT_LEAST(53,6)
    pLibs->m_avcodec_alloc_context3 = (uos_avcodec_alloc_context3)dlsym(handle, "avcodec_alloc_context3");
    PrintError();
    pLibs->m_avcodec_free_context = (uos_avcodec_free_context)dlsym(handle,"avcodec_free_context");
    PrintError();
    #if !LIBAVCODEC_VER_AT_LEAST(60,0)
    pLibs->m_avcodec_get_context_defaults3 = (uos_avcodec_get_context_defaults3)dlsym(handle, "avcodec_get_context_defaults3");
    PrintError();
    #endif
#else
    pLibs->m_avcodec_alloc_context = (uos_avcodec_alloc_context)dlsym(handle, "avcodec_alloc_context");
    PrintError();
    dlsym(handle, "avcodec_get_context_defaults");
    PrintError();
#endif

#if LIBAVCODEC_VER_AT_LEAST(53,6)
    dlsym(handle, "avcodec_open2");
    PrintError();

#else
    dlsym(handle, "avcodec_open");
    PrintError();
#endif

#if LIBAVCODEC_VER_AT_LEAST(55,28)
    dlsym(handle, "av_frame_alloc");
    PrintError();
    dlsym(handle, "av_frame_unref");
    PrintError();
#else
    dlsym(handle, "avcodec_alloc_frame");
    PrintError();
    dlsym(handle, "avcodec_get_frame_defaults");
    PrintError();
#endif

#if LIBAVCODEC_VER_AT_LEAST(55,28)
    dlsym(handle, "av_frame_free");
    PrintError();
#else
    #if LIBAVCODEC_VER_AT_LEAST(54,28)
        dlsym(handle, "avcodec_free_frame");
        PrintError();
    #else
        dlsym(handle, "av_freep");
        PrintError();
    #endif
#endif

    pLibs->m_avcodec_find_encoder = (uos_avcodec_find_encoder)dlsym(handle, "avcodec_find_encoder");
    PrintError();
    pLibs->m_avcodec_find_decoder = (uos_avcodec_find_decoder)dlsym(handle, "avcodec_find_decoder");
    PrintError();
    pLibs->m_avcodec_find_encoder_by_name = (uos_avcodec_find_encoder_by_name)dlsym(handle, "avcodec_find_encoder_by_name");
    PrintError();

    pLibs->m_avcodec_open2 = (uos_avcodec_open2)dlsym(handle, "avcodec_open2");
    PrintError();
    pLibs->m_avcodec_flush_buffers = (uos_avcodec_flush_buffers)dlsym(handle, "avcodec_flush_buffers");
    PrintError();
    pLibs->m_avcodec_close = (uos_avcodec_close)dlsym(handle, "avcodec_close");
    PrintError();
    pLibs->m_avcodec_parameters_from_context = (uos_avcodec_parameters_from_context)dlsym(handle, "avcodec_parameters_from_context");
    PrintError();
    pLibs->m_avcodec_parameters_to_context = (uos_avcodec_parameters_to_context)dlsym(handle, "avcodec_parameters_to_context");
    PrintError();
#if LIBAVFORMAT_VERSION_MAJOR >= 57 && LIBAVFORMAT_VERSION_MINOR <= 25
    dlsym(handle, "avcodec_find_decoder");
    PrintError();
#endif
    pLibs->m_avcodec_send_packet = (uos_avcodec_send_packet)dlsym(handle, "avcodec_send_packet");
    PrintError();
    pLibs->m_avcodec_receive_frame = (uos_avcodec_receive_frame)dlsym(handle, "avcodec_receive_frame");
    PrintError();
    pLibs->m_avcodec_receive_packet = (uos_avcodec_receive_packet)dlsym(handle, "avcodec_receive_packet");
    PrintError();
    pLibs->m_av_packet_free = (uos_av_packet_free)dlsym(handle, "av_packet_free");
    PrintError();
    pLibs->m_avcodec_fill_audio_frame = (uos_avcodec_fill_audio_frame)dlsym(handle, "avcodec_fill_audio_frame");
    PrintError();
    pLibs->m_av_packet_unref = (uos_av_packet_unref)dlsym(handle, "av_packet_unref");
    PrintError();
    pLibs->m_avcodec_send_frame = (uos_avcodec_send_frame)dlsym(handle, "avcodec_send_frame");
    PrintError();
    pLibs->m_av_packet_rescale_ts = (uos_av_packet_rescale_ts)dlsym(handle, "av_packet_rescale_ts");
    PrintError();
    pLibs->m_av_init_packet = (uos_av_init_packet)dlsym(handle, "av_init_packet");
    PrintError();
    pLibs->m_av_packet_alloc = (uos_av_packet_alloc)dlsym(handle, "av_packet_alloc");
    PrintError();

    //libffmpegthumbnailer
    void *handle2 = dlopen(g_ldnames.chFfmpegthumbnailer/*"libffmpegthumbnailer.so.4"*/,RTLD_LAZY);
    if (!handle2) {
        PrintError();
    }
    pLibs->m_video_thumbnailer = (uos_video_thumbnailer)dlsym(handle2, "video_thumbnailer_create");
    PrintError();
    pLibs->m_video_thumbnailer_destroy = (uos_video_thumbnailer_destroy)dlsym(handle2, "video_thumbnailer_destroy");
    PrintError();
    pLibs->m_video_thumbnailer_create_image_data = (uos_video_thumbnailer_create_image_data)dlsym(handle2, "video_thumbnailer_create_image_data");
    PrintError();
    pLibs->m_video_thumbnailer_destroy_image_data = (uos_video_thumbnailer_destroy_image_data)dlsym(handle2, "video_thumbnailer_destroy_image_data");
    PrintError();
    pLibs->m_video_thumbnailer_generate_thumbnail_to_buffer = (uos_video_thumbnailer_generate_thumbnail_to_buffer)dlsym(handle2, "video_thumbnailer_generate_thumbnail_to_buffer");
    PrintError();

    //libswresample
    void *handle3 = dlopen(g_ldnames.chSwresample/*"libswresample.so.3"*/,RTLD_LAZY);
    if (!handle3) {
        PrintError();
    }
    pLibs->m_swr_free = (uos_swr_free)dlsym(handle3, "swr_free");
    PrintError();

    //libswscale
    void *handle4 = dlopen(g_ldnames.chSwscale/*"libswscale.so.5"*/,RTLD_LAZY);
    if (!handle4) {
        PrintError();
    }

    pLibs->m_sws_freeContext = (uos_sws_freeContext)dlsym(handle4, "sws_freeContext");
    PrintError();
    assert(pLibs != NULL);
    return pLibs;
}

/**
 * 饿汉式
 * 支持延迟加载，但是为了多线程安全，性能有所降低
 * 注意：方法内部要加锁，防止多线程多次创建
 * */
LoadLibs *getLoadLibsInstance()
{    
    static pthread_mutex_t mutex;
    //双检锁
    if (pLibs == NULL) {
        // 这里要对pLibs加锁
        pthread_mutex_lock(&mutex);
        if (pLibs == NULL)
            pLibs = newClass();

        //退出时解锁
        pthread_mutex_unlock(&mutex);
    }

    return pLibs;
}

static LoadAvformat *pAvformat = NULL;
static LoadAvformat *newAvformat(void)
{
    pAvformat = (LoadAvformat *)malloc(sizeof(LoadAvformat));
//    RTLD_NOW：在dlopen返回前，解析出全部没有定义的符号，解析不出来返回NULL。
//    RTLD_LAZY：暂缓决定，等有需要时再解出符号
    //libavformat
    void *handle1 = dlopen(g_ldnames.chAvformat/*"libavformat.so.58"*/,RTLD_LAZY);
    if (!handle1) {
        PrintError();
    }

    pAvformat->m_avformat_open_input = (uos_avformat_open_input)dlsym(handle1, "avformat_open_input");
    PrintError();
    pAvformat->m_avformat_find_stream_info = (uos_avformat_find_stream_info)dlsym(handle1, "avformat_find_stream_info");
    PrintError();
    pAvformat->m_av_find_best_stream = (uos_av_find_best_stream)dlsym(handle1, "av_find_best_stream");
    PrintError();
    pAvformat->m_av_dump_format = (uos_av_dump_format)dlsym(handle1, "av_dump_format");//目前未使用
    PrintError();
    pAvformat->m_avformat_close_input = (uos_avformat_close_input)dlsym(handle1, "avformat_close_input");
    PrintError();
    pAvformat->m_avformat_alloc_output_context2 = (uos_avformat_alloc_output_context2)dlsym(handle1, "avformat_alloc_output_context2");
    PrintError();
    pAvformat->m_avformat_new_stream = (uos_avformat_new_stream)dlsym(handle1, "avformat_new_stream");
    PrintError();
    pAvformat->m_avformat_free_context = (uos_avformat_free_context)dlsym(handle1, "avformat_free_context");
    PrintError();
    pAvformat->m_avformat_write_header = (uos_avformat_write_header)dlsym(handle1, "avformat_write_header");
    PrintError();
    pAvformat->m_avio_open = (uos_avio_open)dlsym(handle1, "avio_open");
    PrintError();
    pAvformat->m_av_write_frame = (uos_av_write_frame)dlsym(handle1, "av_write_frame");
    PrintError();
    pAvformat->m_av_write_trailer = (uos_av_write_trailer)dlsym(handle1, "av_write_trailer");
    PrintError();
    pAvformat->m_avio_closep = (uos_avio_closep)dlsym(handle1, "avio_closep");
    PrintError();


//    //libffmpegthumbnailer
//    void *handle2 = dlopen("libffmpegthumbnailer.so",RTLD_LAZY);
//    if (!handle2) {
//        PrintError();
//    }
//    pLibs->m_video_thumbnailer = (uos_video_thumbnailer)dlsym(handle2, "video_thumbnailer_create");
//    PrintError();
//    pLibs->m_video_thumbnailer_destroy = (uos_video_thumbnailer_destroy)dlsym(handle2, "video_thumbnailer_destroy");
//    PrintError();
//    pLibs->m_video_thumbnailer_create_image_data = (uos_video_thumbnailer_create_image_data)dlsym(handle2, "video_thumbnailer_create_image_data");
//    PrintError();
//    pLibs->m_video_thumbnailer_destroy_image_data = (uos_video_thumbnailer_destroy_image_data)dlsym(handle2, "video_thumbnailer_destroy_image_data");
//    PrintError();
//    pLibs->m_video_thumbnailer_generate_thumbnail_to_buffer = (uos_video_thumbnailer_generate_thumbnail_to_buffer)dlsym(handle2, "video_thumbnailer_generate_thumbnail_to_buffer");
//    PrintError();

//    void *handle3 = dlopen("libswresample.so",RTLD_LAZY);
//    if (!handle3) {
//        PrintError();
//    }
//    pLibs->m_swr_free = (uos_swr_free)dlsym(handle3, "swr_free");
//    PrintError();


//    void *handle4 = dlopen("libswscale.so",RTLD_LAZY);
//    if (!handle4) {
//        PrintError();
//    }
//    pLibs->m_sws_freeContext = (uos_sws_freeContext)dlsym(handle4, "sws_freeContext");
//    PrintError();
    assert(pAvformat != NULL);
    return pAvformat;
}

/**
 * 饿汉式
 * 支持延迟加载，但是为了多线程安全，性能有所降低
 * 注意：方法内部要加锁，防止多线程多次创建
 * */
LoadAvformat *getAvformat()
{
    static pthread_mutex_t mutexAvformat;
    //双检锁
    if (pAvformat == NULL) {
        // 这里要对pLibs加锁
        pthread_mutex_lock(&mutexAvformat);
        if (pAvformat == NULL)
        {
            pAvformat = newAvformat();
        }
        //退出时解锁
        pthread_mutex_unlock(&mutexAvformat);
    }

    return pAvformat;
}


static LoadAvutil *Avutil = NULL;
static LoadAvutil *newAvutil(void)
{
    Avutil = (LoadAvutil *)malloc(sizeof(LoadAvutil));
    //libavutil
    void *handle5 = dlopen(g_ldnames.chAvutil/*"libavutil.so.56"*/,RTLD_LAZY);
    if (!handle5) {
        PrintError();
    }
    Avutil->m_av_dict_get = (uos_av_dict_get)dlsym(handle5, "av_dict_get");//目前未使用
    PrintError();
    Avutil->m_av_strerror = (uos_av_strerror)dlsym(handle5, "av_strerror");//目前未使用
    PrintError();

    Avutil->m_av_dict_copy = (uos_av_dict_copy)dlsym(handle5, "av_dict_copy");
    PrintError();
    Avutil->m_av_dict_free = (uos_av_dict_free)dlsym(handle5, "av_dict_free");
    PrintError();
    Avutil->m_av_dict_set = (uos_av_dict_set)dlsym(handle5, "av_dict_set");
    PrintError();
    Avutil->m_av_dict_set_int = (uos_av_dict_set_int)dlsym(handle5, "av_dict_set_int");
    PrintError();
    Avutil->m_av_log_set_level = (uos_av_log_set_level)dlsym(handle5, "av_log_set_level");
    PrintError();
    Avutil->m_av_image_copy_to_buffer = (uos_av_image_copy_to_buffer)dlsym(handle5, "av_image_copy_to_buffer");
    PrintError();
    Avutil->m_av_frame_free = (uos_av_frame_free)dlsym(handle5, "av_frame_free");
    PrintError();
    Avutil->m_av_frame_alloc = (uos_av_frame_alloc)dlsym(handle5, "av_frame_alloc");
    PrintError();
    Avutil->m_av_freep = (uos_av_freep)dlsym(handle5, "av_freep");
    PrintError();
    Avutil->m_av_frame_unref = (uos_av_frame_unref)dlsym(handle5, "av_frame_unref");
    PrintError();
    Avutil->m_av_free = (uos_av_free)dlsym(handle5, "av_free");
    PrintError();
    Avutil->m_av_samples_get_buffer_size = (uos_av_samples_get_buffer_size)dlsym(handle5, "av_samples_get_buffer_size");
    PrintError();
    Avutil->m_av_get_media_type_string = (uos_av_get_media_type_string)dlsym(handle5, "av_get_media_type_string");
    PrintError();
    Avutil->m_av_image_get_buffer_size = (uos_av_image_get_buffer_size)dlsym(handle5, "av_image_get_buffer_size");
    PrintError();

    assert(Avutil != NULL);
    return Avutil;
}

/**
 * 饿汉式
 * 支持延迟加载，但是为了多线程安全，性能有所降低
 * 注意：方法内部要加锁，防止多线程多次创建
 * */
LoadAvutil *getAvutil()
{
    static pthread_mutex_t mutexAvutil;
    //双检锁
    if (Avutil == NULL) {
        // 这里要对pLibs加锁
        pthread_mutex_lock(&mutexAvutil);
        if (Avutil == NULL)
        {
            Avutil = newAvutil();
        }
        //退出时解锁
        pthread_mutex_unlock(&mutexAvutil);
    }

    return Avutil;
}

static LoadUdev *pUdev = NULL;
static LoadUdev *newUdev(void)
{
    pUdev = (LoadUdev *)malloc(sizeof(LoadUdev));
    //libudev
    void *handle = dlopen(g_ldnames.chUdev/*"libudev.so.1"*/,RTLD_LAZY);
    if (!handle) {
        PrintError();
    }
    pUdev->m_udev_enumerate_new = (uos_udev_enumerate_new)dlsym(handle, "udev_enumerate_new");
    PrintError();
    pUdev->m_udev_enumerate_get_udev = (uos_udev_enumerate_get_udev)dlsym(handle, "udev_enumerate_get_udev");
    PrintError();
    pUdev->m_udev_enumerate_add_match_subsystem = (uos_udev_enumerate_add_match_subsystem)dlsym(handle, "udev_enumerate_add_match_subsystem");
    PrintError();
    pUdev->m_udev_enumerate_scan_devices = (uos_udev_enumerate_scan_devices)dlsym(handle, "udev_enumerate_scan_devices");
    PrintError();
    pUdev->m_udev_enumerate_get_list_entry = (uos_udev_enumerate_get_list_entry)dlsym(handle, "udev_enumerate_get_list_entry");
    PrintError();
    pUdev->m_udev_list_entry_get_name = (uos_udev_list_entry_get_name)dlsym(handle, "udev_list_entry_get_name");
    PrintError();
    pUdev->m_udev_device_new_from_syspath = (uos_udev_device_new_from_syspath)dlsym(handle, "udev_device_new_from_syspath");
    PrintError();
    pUdev->m_udev_device_get_devnode = (uos_udev_device_get_devnode)dlsym(handle, "udev_device_get_devnode");
    PrintError();
    pUdev->m_udev_device_get_parent_with_subsystem_devtype = (uos_udev_device_get_parent_with_subsystem_devtype)dlsym(handle, "udev_device_get_parent_with_subsystem_devtype");
    PrintError();
    pUdev->m_udev_device_get_sysattr_value = (uos_udev_device_get_sysattr_value)dlsym(handle, "udev_device_get_sysattr_value");
    PrintError();
    pUdev->m_udev_device_unref = (uos_udev_device_unref)dlsym(handle, "udev_device_unref");
    PrintError();
    pUdev->m_udev_enumerate_unref = (uos_udev_enumerate_unref)dlsym(handle, "udev_enumerate_unref");
    PrintError();
    pUdev->m_udev_new = (uos_udev_new)dlsym(handle, "udev_new");
    PrintError();
    pUdev->m_udev_monitor_new_from_netlink = (uos_udev_monitor_new_from_netlink)dlsym(handle, "udev_monitor_new_from_netlink");
    PrintError();
    pUdev->m_udev_monitor_filter_add_match_subsystem_devtype = (uos_udev_monitor_filter_add_match_subsystem_devtype)dlsym(handle, "udev_monitor_filter_add_match_subsystem_devtype");
    PrintError();
    pUdev->m_udev_monitor_enable_receiving = (uos_udev_monitor_enable_receiving)dlsym(handle, "udev_monitor_enable_receiving");
    PrintError();
    pUdev->m_udev_monitor_get_fd = (uos_udev_monitor_get_fd)dlsym(handle, "udev_monitor_get_fd");
    PrintError();
    pUdev->m_udev_monitor_receive_device = (uos_udev_monitor_receive_device)dlsym(handle, "udev_monitor_receive_device");
    PrintError();
    pUdev->m_udev_device_get_devtype = (uos_udev_device_get_devtype)dlsym(handle, "udev_device_get_devtype");
    PrintError();
    pUdev->m_udev_device_get_action = (uos_udev_device_get_action)dlsym(handle, "udev_device_get_action");
    PrintError();
    pUdev->m_udev_unref = (uos_udev_unref)dlsym(handle, "udev_unref");
    PrintError();
    pUdev->m_udev_list_entry_get_next = (uos_udev_list_entry_get_next)dlsym(handle, "udev_list_entry_get_next");
    PrintError();

    assert(pUdev != NULL);
    return pUdev;
}

LoadUdev *getUdev()
{
    static pthread_mutex_t mutexUdev;
    //双检锁
    if (pUdev == NULL) {
        // 这里要对pUdev加锁
        pthread_mutex_lock(&mutexUdev);
        if (pUdev == NULL)
        {
            pUdev = newUdev();
        }
        //退出时解锁
        pthread_mutex_unlock(&mutexUdev);
    }

    return pUdev;
}

static LoadUSB *pUSB = NULL;
static LoadUSB *newUSB(void)
{
    pUSB = (LoadUSB *)malloc(sizeof(LoadUSB));
    //libusb
    void *handle = dlopen(g_ldnames.chUsb/*"libusb-1.0.so.0"*/,RTLD_LAZY);
    if (!handle) {
        PrintError();
    }
    pUSB->m_libusb_init = (uos_libusb_init)dlsym(handle, "libusb_init");
    PrintError();
    pUSB->m_libusb_get_device_list = (uos_libusb_get_device_list)dlsym(handle, "libusb_get_device_list");
    PrintError();
    pUSB->m_libusb_get_bus_number = (uos_libusb_get_bus_number)dlsym(handle, "libusb_get_bus_number");
    PrintError();
    pUSB->m_libusb_get_device_address = (uos_libusb_get_device_address)dlsym(handle, "libusb_get_device_address");
    PrintError();
    pUSB->m_libusb_ref_device = (uos_libusb_ref_device)dlsym(handle, "libusb_ref_device");
    PrintError();
    pUSB->m_libusb_free_device_list = (uos_libusb_free_device_list)dlsym(handle, "libusb_free_device_list");
    PrintError();
    pUSB->m_libusb_get_device_descriptor = (uos_libusb_get_device_descriptor)dlsym(handle, "libusb_get_device_descriptor");
    PrintError();
    pUSB->m_libusb_get_config_descriptor = (uos_libusb_get_config_descriptor)dlsym(handle, "libusb_get_config_descriptor");
    PrintError();
    pUSB->m_libusb_unref_device = (uos_libusb_unref_device)dlsym(handle, "libusb_unref_device");
    PrintError();

    assert(pUSB != NULL);
    return pUSB;
}

LoadUSB *getUSB()
{
    static pthread_mutex_t mutexUSB;
    //双检锁
    if (pUSB == NULL) {
        // 这里要对pUSB加锁
        pthread_mutex_lock(&mutexUSB);
        if (pUSB == NULL)
        {
            pUSB = newUSB();
        }
        //退出时解锁
        pthread_mutex_unlock(&mutexUSB);
    }

    return pUSB;
}

static LoadPortAudio *pPortAudio = NULL;
static LoadPortAudio *newPortAudio(void)
{
    pPortAudio = (LoadPortAudio *)malloc(sizeof(LoadPortAudio));
    //libportaudio
    void *handle = dlopen(g_ldnames.chPortaudio/*"libportaudio.so.2"*/,RTLD_LAZY);
    if (!handle) {
        PrintError();
    }
    pPortAudio->m_Pa_GetDeviceCount = (uos_Pa_GetDeviceCount)dlsym(handle, "Pa_GetDeviceCount");
    PrintError();
    pPortAudio->m_Pa_IsStreamStopped = (uos_Pa_IsStreamStopped)dlsym(handle, "Pa_IsStreamStopped");
    PrintError();
    pPortAudio->m_Pa_GetDeviceInfo = (uos_Pa_GetDeviceInfo)dlsym(handle, "Pa_GetDeviceInfo");
    PrintError();
    pPortAudio->m_Pa_Initialize = (uos_Pa_Initialize)dlsym(handle, "Pa_Initialize");
    PrintError();
    pPortAudio->m_Pa_IsStreamActive = (uos_Pa_IsStreamActive)dlsym(handle, "Pa_IsStreamActive");
    PrintError();
    pPortAudio->m_Pa_Terminate = (uos_Pa_Terminate)dlsym(handle, "Pa_Terminate");
    PrintError();
    pPortAudio->m_Pa_AbortStream = (uos_Pa_AbortStream)dlsym(handle, "Pa_AbortStream");
    PrintError();
    pPortAudio->m_Pa_StopStream = (uos_Pa_StopStream)dlsym(handle, "Pa_StopStream");
    PrintError();
    pPortAudio->m_Pa_CloseStream = (uos_Pa_CloseStream)dlsym(handle, "Pa_CloseStream");
    PrintError();
    pPortAudio->m_Pa_GetDefaultInputDevice = (uos_Pa_GetDefaultInputDevice)dlsym(handle, "Pa_GetDefaultInputDevice");
    PrintError();
    pPortAudio->m_Pa_GetHostApiInfo = (uos_Pa_GetHostApiInfo)dlsym(handle, "Pa_GetHostApiInfo");
    PrintError();
    pPortAudio->m_Pa_GetDefaultOutputDevice = (uos_Pa_GetDefaultOutputDevice)dlsym(handle, "Pa_GetDefaultOutputDevice");
    PrintError();
    pPortAudio->m_Pa_OpenStream = (uos_Pa_OpenStream)dlsym(handle, "Pa_OpenStream");
    PrintError();
    pPortAudio->m_Pa_StartStream = (uos_Pa_StartStream)dlsym(handle, "Pa_StartStream");
    PrintError();
    pPortAudio->m_Pa_GetStreamInfo = (uos_Pa_GetStreamInfo)dlsym(handle, "Pa_GetStreamInfo");
    PrintError();
    pPortAudio->m_Pa_GetErrorText = (uos_Pa_GetErrorText)dlsym(handle, "Pa_GetErrorText");
    PrintError();
    assert(pPortAudio != NULL);
    return pPortAudio;
}

LoadPortAudio *getPortAudio()
{
    static pthread_mutex_t mutexPortAudio;
    //双检锁
    if (pPortAudio == NULL) {
        // 这里要对pUSB加锁
        pthread_mutex_lock(&mutexPortAudio);
        if (pPortAudio == NULL)
        {
            pPortAudio = newPortAudio();
        }
        //退出时解锁
        pthread_mutex_unlock(&mutexPortAudio);
    }

    return pPortAudio;
}

static LoadV4l2 *pV4l2 = NULL;
static LoadV4l2 *newV4l2(void)
{
    pV4l2 = (LoadV4l2 *)malloc(sizeof(LoadV4l2));
    //libv4l2
    void *handle = dlopen(g_ldnames.chV4l2/*"libv4l2.so.0"*/,RTLD_LAZY);
    if (!handle) {
        PrintError();
    }

    pV4l2->m_v4l2_ioctl = (uos_v4l2_ioctl)dlsym(handle, "v4l2_ioctl");
    PrintError();
    pV4l2->m_v4l2_munmap = (uos_v4l2_munmap)dlsym(handle, "v4l2_munmap");
    PrintError();
    pV4l2->m_v4l2_mmap = (uos_v4l2_mmap)dlsym(handle, "v4l2_mmap");
    PrintError();
    pV4l2->m_v4l2_read = (uos_v4l2_read)dlsym(handle, "v4l2_read");
    PrintError();
    pV4l2->m_v4l2_close = (uos_v4l2_close)dlsym(handle, "v4l2_close");
    PrintError();
    pV4l2->m_v4l2_open = (uos_v4l2_open)dlsym(handle, "v4l2_open");
    PrintError();

    assert(pV4l2 != NULL);
    return pV4l2;
}

LoadV4l2 *getV4l2()
{
    static pthread_mutex_t mutexV4l2;
    //双检锁
    if (pV4l2 == NULL) {
        // 这里要对pUSB加锁
        pthread_mutex_lock(&mutexV4l2);
        if (pV4l2 == NULL)
        {
            pV4l2 = newV4l2();
        }
        //退出时解锁
        pthread_mutex_unlock(&mutexV4l2);
    }

    return pV4l2;
}

void setLibNames(LoadLibNames tmp)
{
    g_ldnames.chAvcodec = ( char*)malloc(strlen(tmp.chAvcodec)+1);
   // memset(g_ldnames.chAvcodec,strlen(tmp.chAvcodec),0);
    strcpy(g_ldnames.chAvcodec,tmp.chAvcodec);

    g_ldnames.chFfmpegthumbnailer = ( char*)malloc(strlen(tmp.chFfmpegthumbnailer)+1);
   // memset(g_ldnames.chFfmpegthumbnailer,strlen(tmp.chFfmpegthumbnailer),0);
    strcpy(g_ldnames.chFfmpegthumbnailer,tmp.chFfmpegthumbnailer);

    g_ldnames.chSwresample = ( char*)malloc(strlen(tmp.chSwresample)+1);
   // memset(g_ldnames.chSwresample,strlen(tmp.chSwresample),0);
    strcpy(g_ldnames.chSwresample,tmp.chSwresample);

    g_ldnames.chSwscale = ( char*)malloc(strlen(tmp.chSwscale)+1);
   // memset(g_ldnames.chSwscale,strlen(tmp.chSwscale),0);
    strcpy(g_ldnames.chSwscale,tmp.chSwscale);

    g_ldnames.chAvformat = ( char*)malloc(strlen(tmp.chAvformat)+1);
   // memset(g_ldnames.chAvformat,strlen(tmp.chAvformat),0);
    strcpy(g_ldnames.chAvformat,tmp.chAvformat);

    g_ldnames.chAvutil = ( char*)malloc(strlen(tmp.chAvutil)+1);
   // memset(g_ldnames.chAvutil,strlen(tmp.chAvutil),0);
    strcpy(g_ldnames.chAvutil,tmp.chAvutil);

    g_ldnames.chUdev = ( char*)malloc(strlen(tmp.chUdev)+1);
   // memset(g_ldnames.chUdev,strlen(tmp.chUdev),0);
    strcpy(g_ldnames.chUdev,tmp.chUdev);

    g_ldnames.chUsb = ( char*)malloc(strlen(tmp.chUsb)+1);
   // memset(g_ldnames.chUsb,strlen(tmp.chUsb),0);
    strcpy(g_ldnames.chUsb,tmp.chUsb);

    g_ldnames.chPortaudio = ( char*)malloc(strlen(tmp.chPortaudio)+1);
   // memset(g_ldnames.chPortaudio,strlen(tmp.chPortaudio),0);
    strcpy(g_ldnames.chPortaudio,tmp.chPortaudio);

    g_ldnames.chV4l2 = ( char*)malloc(strlen(tmp.chV4l2)+1);
   // memset(g_ldnames.chV4l2,strlen(tmp.chV4l2),0);
    strcpy(g_ldnames.chV4l2,tmp.chV4l2);
}
