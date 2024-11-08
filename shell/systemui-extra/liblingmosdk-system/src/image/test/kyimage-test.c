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

#include <stdio.h>
#include <time.h>
#include <libkyimageproc.h>

double my_get_timestamp()
{
  struct timespec a;
  clock_gettime(CLOCK_MONOTONIC, &a);
  return ((double)a.tv_nsec * 1.0e-9) + (double)a.tv_sec;
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
      printf("请输入目标编码文件\n");
      return 0;
    }
    double start = my_get_timestamp();
    void * handle = kdk_jpeg_encoder_init();
    double end = my_get_timestamp();
    printf("jpeg encoder init %.2fms\n", (end - start) * 1000);

    start = my_get_timestamp();
    kdk_jpeg_encode_to_file(handle,argv[1],"dest.jpg");
    end = my_get_timestamp();
   	printf("encode image %.2fms\n", (end - start) * 1000);
    kdk_jpeg_encoder_release(handle);
    return 0;
}