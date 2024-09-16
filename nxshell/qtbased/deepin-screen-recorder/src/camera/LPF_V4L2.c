// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef __cplusplus
extern "C" {
#endif
#include "camview.h"
#include "LPF_V4L2.h"
#include "gviewaudio.h"
#include "gviewv4l2core.h"
#include "v4l2_devices.h"
#include "v4l2_core.h"
#include "gviewencoder.h"
#include "gviewrender.h"
#include "audio.h"
#include "core_io.h"
#include "colorspaces.h"


int camInit(const char *devicename)
{
    // 初始化yuv转rgb浮点值查找表 wayland下需要使用
    init_yuv2rgb_num_table();

    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    options_t *my_options = options_get();
    /*用option更新配置文件*/
    config_update(my_options);
    /*获取配置项数据*/
    config_t *my_config = config_get();
    //v4l2的调试级别 0最低
     my_options->verbosity = 0;
    debug_level = my_options->verbosity;
    if (debug_level > 1)
        printf("deepin-screen-recorder: main thread (tid: %u)\n",
               (unsigned int) syscall (SYS_gettid));

    /*设置v4l2debug*/
    v4l2core_set_verbosity(debug_level);

    /*得到v4l2的句柄*/
    if (!my_config->device_location)
        my_config->device_location = strdup("");

    v4l2_dev_t *my_vd;
    if (strcmp(devicename, "") == 0)
    {
        my_vd = create_v4l2_device_handler(my_config->device_location);
    }
    else{
        my_vd = create_v4l2_device_handler(devicename);
    }
    //fprintf(stderr, " %s: my_vd->videodevice: %s\n",__FUNCTION__,my_vd->videodevice);

    if (my_vd == NULL) {
        int i;
        v4l2_device_list_t *devlist = get_device_list();
        if (devlist == NULL){
            return E_NO_DEVICE_ERR;
        }
        for (i = 0; i < devlist->num_devices; i++) {
            fprintf(stderr, "deepin-screen-recorder: devlist->list_devices[%d].device= %s\n",i,devlist->list_devices[i].device);
            my_vd = create_v4l2_device_handler(devlist->list_devices[i].device);
            if (my_vd != NULL)
                break;
            else
                continue;
        }
        if (my_vd == NULL) {
            char message[150];
            sprintf(message, "no video device (%s) found", my_options->device);
            return E_NO_DEVICE_ERR;
        }
    }
    /*选择捕捉方式*/
    if (strcasecmp(my_config->capture, "read") == 0)
        v4l2core_set_capture_method(my_vd, IO_READ);
    else
        v4l2core_set_capture_method(my_vd, IO_MMAP);

    /*设置软件自动对焦排序方法*/
    v4l2core_soft_autofocus_set_sort(AUTOF_SORT_QUICK);
    /*定义fps*/
    v4l2core_define_fps(my_vd, my_config->fps_num, my_config->fps_denom);

    if (!my_options->control_panel) {
        int ret = E_NO_DATA;
        v4l2_device_list_t *devlist = get_device_list();

        if (devlist == NULL){
            return E_NO_DEVICE_ERR;
        }

        v4l2core_prepare_new_format(my_vd, (int)my_config->format);
        v4l2core_prepare_valid_resolution(my_vd);
        ret = v4l2core_update_current_format(my_vd);
        if (ret != E_OK) {
            v4l2core_prepare_valid_format(my_vd);
            v4l2core_prepare_valid_resolution(my_vd);
            ret = v4l2core_update_current_format(my_vd);
            if (ret != E_OK) {
                fprintf(stderr, "cheese: also could not set the first listed stream format\n");
                fprintf(stderr, "cheese: Video capture failed\n");
            }
        }
        return ret;
    }
    return E_OK;
}

int camUnInit()
{
    v4l2_dev_t *vd =  get_v4l2_device_handler();

    if (vd)
        close_v4l2_device_handler();
    return 0;
}

#ifdef __cplusplus
}
#endif
