/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lingmosdk/lingmosdk-base/libkylog.h>
#include "libkyimageproc.h"

#define JPEG_ENCODER_LIB                "/usr/local/lib/libopencl-jpegencoder.so"
#define JPEG_ENCODER_GET_HANDLE         "oclget_jpegencoder_handle"
#define JPEG_ENCODER_HANDLE_INIT        "ocljpeg_encoder_init"
#define JPEG_ENCODER_IMAGE_ENCODE       "ocljpeg_encode_image"
#define JPEG_ENCODER_HANDLE_RELEASE     "ocljpeg_encoder_release"


typedef void*(*get_handle)(); 
typedef int (*handle_init)(void *handle);
typedef int (*encode_image)(void *handle, unsigned char*image, size_t width, size_t height, char*file);
typedef void (*handle_release)(void *handle);

typedef struct rgb {
    unsigned char r;
    unsigned char g;
    unsigned char b;
}rgb_t;


static int verify_file(char *pFileName)
{
    char *p = pFileName;
    int len = strlen(pFileName);
    if(len <= 4)
    {
        klog_err("filename is illeagal\n");
        return -1;
    }
    len -= 4;
    p+= len;
    if(strcmp(p,".ppm"))
    {
        klog_err("file format is illeagal\n");
        return -1;
    }
    else
        return 0;
}
int read_image(const char * const file, size_t *width, size_t *height, rgb_t **buffer)
{
    char line[0x80];
    char *tok;
    int ret;
    int ret1;
    char* ret2;

    ret = 0;
    char canonical_filename[256]={0};
    if(!realpath(file,canonical_filename) || verify_file(canonical_filename))
    {
        return 0x05;
    }
    FILE *fp = fopen(canonical_filename, "rb");
    if(fp == NULL) {
        klog_err("Could not open file:%s\n",file);
        return 0x1;
    }

    if(fgets(line, 0x80, fp) == NULL) {
        klog_err("Could not get content from file:%s\n",file);
        ret = 0x2;
        goto end;
    }

    if(strcmp(line, "P6\n")) {
        klog_err("Illegal file format\n");
        ret = 0x3;
        goto end;
    }
    while(fgets(line, 0x80, fp)) {
        if(line[0] == '#')
            continue;
        else {
            tok = strtok(line, " ");
            *width = atoi(tok);
            tok = strtok(NULL, " ");
            *height = atoi(tok);
            ret2 = fgets(line, 0x80, fp);
            break;
        }
    }


    *buffer = malloc(*width * *height * sizeof(rgb_t));
    if(*buffer == NULL) {
        klog_err("Memory Allocation failed\n");
        ret = 0x4;
        goto end;
    }

    ret1 = fread(*buffer, sizeof(rgb_t), *width * *height, fp);

end:
    fclose(fp);
    return ret;
}

void* kdk_jpeg_encoder_init()
{
    void* libHandle = dlopen(JPEG_ENCODER_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        klog_err("load %s failed\n",JPEG_ENCODER_LIB);
        return NULL;
    }
    get_handle p_get_handle = dlsym(libHandle, JPEG_ENCODER_GET_HANDLE);
    if(!p_get_handle)
    {
        klog_err("undefined symbol:%s\n",JPEG_ENCODER_GET_HANDLE);
        return NULL;
    }

    void *handle = p_get_handle();
    
    if(!handle)
    {
        klog_err("get image process handle failed\n");
        return NULL;
    }
    handle_init p_handle_init = dlsym(libHandle, JPEG_ENCODER_HANDLE_INIT);
    if(!p_handle_init)
    {
        klog_err("undefined symbol:%s\n",JPEG_ENCODER_HANDLE_INIT);
        return NULL;
    }

    int ret = p_handle_init(handle);
    
    if(ret == -1)
    {
        klog_err("jpeg encoder init failed\n");
        return NULL;
    }
    return handle;
    
}

int kdk_jpeg_encode_to_file(void *handle, char*srcFile, char *destFile)
{
    size_t width = 0;
    size_t height = 0;
    rgb_t *raw_data = NULL;
    if(read_image(srcFile, &width, &height, &raw_data))
    {
        return -1;
    }
    void* libHandle = dlopen(JPEG_ENCODER_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        klog_err("load %s failed\n",JPEG_ENCODER_LIB);
        return -1;
    }
    encode_image p_encode_image = dlsym(libHandle, JPEG_ENCODER_IMAGE_ENCODE);
    if(!p_encode_image)
    {
        klog_err("undefined symbol:%s\n",JPEG_ENCODER_IMAGE_ENCODE);
        return -1;
    }

    int ret = p_encode_image(handle , (unsigned char*)raw_data, width, height, destFile);
    if (ret == -1) {
        klog_err("jpeg ecode image failed\n");
        kdk_jpeg_encoder_release(handle);
        free(raw_data);
        return -1;
    }
    free(raw_data);
    return 0;
}

void kdk_jpeg_encoder_release(void* handle)
{
    if(!handle)
        return;
    void* libHandle = dlopen(JPEG_ENCODER_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        klog_err("load %s failed\n",JPEG_ENCODER_LIB);
        return;
    }
    handle_release p_release_handle = dlsym(libHandle, JPEG_ENCODER_HANDLE_RELEASE);
    if(!p_release_handle)
    {
        klog_err("undefined symbol:%s\n",JPEG_ENCODER_HANDLE_RELEASE);
        return;
    }
    p_release_handle(handle);
}
