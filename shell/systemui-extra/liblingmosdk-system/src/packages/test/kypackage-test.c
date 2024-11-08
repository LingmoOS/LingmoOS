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

#include "../libkypackages.h"
#include <stdio.h>
#include <stdlib.h>

static void print_package(kdk_package_t *package)
{
    if (NULL == package)
        return;
    printf("包名：%s\t", package->name);
    printf("版本号：%s\t", package->version);
    printf("包类型：%s\t", package->section);
    printf("包状态：%s\t", package->status);
    printf("包大小：%lu\n", package->size_kb);
}

int main()
{
    kdk_package_list *list = kdk_package_get_packagelist();
    printf("系统中一共%u个软件包\n", list->nums);
    for (size_t i = 0; i < list->nums; i ++)
    {
        print_package(list->list[i]);
    }
    kdk_package_free_packagelist(list);

    printf("Evolution是否安装：%s\n", kdk_package_is_installed("evolution", NULL) == 1 ? "是" : "否");
    char *version = kdk_package_get_version("evolution");
    printf("Evolution版本号：%s\n", version);
    free(version);

    char *description = kdk_package_get_description("aapt");
    if(description != NULL)
    {
        printf("%s\n", description);
        free(description);
    }

    char **dir_list = kdk_package_get_code_path("aapt");
    if(NULL != dir_list)
    {
        int i = 0;
        while (dir_list[i])
        {
            printf("%s\n", dir_list[i++]);
        }
        for(int i = 0; dir_list[i]; i++)
        {
            free(dir_list[i]);
        }
        free(dir_list);
    }
    

    printf("%d\n", kdk_package_get_file_count("aapt"));

    printf("%d\n", kdk_package_verify_disk_space("/home/lingmo/systemd_245.4-4lingmo3.20k0.17oemhwy0.4.u_arm64.deb"));

    printf("%s\n", kdk_package_get_default_browser() ? kdk_package_get_default_browser() : "get browser failed");
    printf("%s\n", kdk_package_get_default_image_viewer() ? kdk_package_get_default_image_viewer() : "get image failed");
    printf("%s\n", kdk_package_get_default_audio_player() ? kdk_package_get_default_audio_player() : "get audio failed");
    printf("%s\n", kdk_package_get_default_video_player() ? kdk_package_get_default_video_player() : "get video failed");
    printf("%s\n", kdk_package_get_default_pdf_viewer() ? kdk_package_get_default_pdf_viewer() : "get pdf failed");
    printf("%s\n", kdk_package_get_default_word_viewer() ? kdk_package_get_default_word_viewer() : "get word failed");
    printf("%s\n", kdk_package_get_default_excel_viewer() ? kdk_package_get_default_excel_viewer() : "get execl failed");
    printf("%s\n", kdk_package_get_default_ppt_viewer() ? kdk_package_get_default_ppt_viewer() : "get ppt failed");

    kdk_startmenu_list *start_menu_list = kdk_package_get_startmenu_list();
    if (NULL != start_menu_list)
    {
        for (int i = 0; i < start_menu_list->nums; i++)
        {
            kdk_startmenu_t *tmp = start_menu_list->list[i];
            printf("%s\n", tmp->cmd);
            printf("%s\n", tmp->company);
            printf("%s\n", tmp->icon);
            printf("%s\n", tmp->name);
            printf("%s\n", tmp->param);
            printf("%s\n", tmp->version);
            printf("\n");
        }
        kdk_package_free_startmenu_list(start_menu_list);
    }

    return 0;
}
