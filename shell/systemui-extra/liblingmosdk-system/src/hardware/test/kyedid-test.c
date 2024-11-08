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

#include "../libkyedid.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char** name = kdk_edid_get_interface();
    if(NULL == name)
        return 0;
    
    for(size_t count = 0; name[count]; count++)
    {
        printf("当前接口 = %s\n", name[count]);

        float gamma = kdk_edid_get_gamma(name[count]);
        printf("gamma = %0.2f\n", gamma);

        float size = kdk_edid_get_size(name[count]);
        printf("size = %0.1f\n", size);

        char *max_resolution = kdk_edid_get_max_resolution(name[count]);
        printf("max_resolution = %s\n", max_resolution);
        free(max_resolution);

        char *model = kdk_edid_get_model(name[count]);
        printf("model = %s\n", model);
        free(model);

        char *area = kdk_edid_get_visible_area(name[count]);
        printf("area = %s\n", area);
        free(area);

        char *manufacturer = kdk_edid_get_manufacturer(name[count]);
        printf("manufacturer = %s\n", manufacturer);
        free(manufacturer);

        int week = kdk_edid_get_week(name[count]);
        printf("week = %d\n", week);

        int year = kdk_edid_get_year(name[count]);
        printf("year = %d\n", year);

        int primary = kdk_edid_get_primary(name[count]);
        printf("%s\n", primary == 1 ? "是" : "否");

        char *resolution = kdk_edid_get_resolution(name[count]);
        printf("resolution = %s\n", resolution);
        free(resolution);

        char *ratio = kdk_edid_get_ratio(name[count]);
        printf("ratio = %s\n", ratio);
        free(ratio);

        char *character = kdk_edid_get_character(name[count]);
        printf("character = %s\n", character);
        free(character);

        char* serial = kdk_edid_get_serialNumber(name[count]);
        printf("serial = %s\n", serial);
        free(serial);

        BrightnessInfo *max_info = kdk_edid_get_max_brightness(name[count]);
        if(max_info)
        {
            printf("brightness = %d\n", max_info->brightness_percentage);
            printf("brightness_value = %d\n", max_info->brightness_value);
        }
        

        BrightnessInfo *cur_brightness = kdk_edid_get_current_brightness(name[count]);
        if (cur_brightness)
        {
            printf("brightness = %d\n", cur_brightness->brightness_percentage);
            printf("brightness_value = %d\n", cur_brightness->brightness_value);
        }
        

        ChromaticityCoordinates *red = kdk_edid_get_red_primary(name[count]);
        if(red)
            printf("redx = %s, redy = %s\n", red->xCoordinate, red->yCoordinate);

        ChromaticityCoordinates *green = kdk_edid_get_green_primary(name[count]);
        if(green)
            printf("greenx = %s, greeny = %s\n", green->xCoordinate, green->yCoordinate);

        ChromaticityCoordinates *blue = kdk_edid_get_blue_primary(name[count]);
        if(blue)
            printf("bluex = %s, bluey = %s\n", blue->xCoordinate, blue->yCoordinate);

        ChromaticityCoordinates *white = kdk_edid_get_white_primary(name[count]);
        if(white)
            printf("whitex = %s, whitey = %s\n", white->xCoordinate, white->yCoordinate);

        float rawDpiX = kdk_edid_get_rawDpiX(name[count]);
        printf("rawDpiX = %.2f\n", rawDpiX);

        float rawDpiY = kdk_edid_get_rawDpiY(name[count]);
        printf("rawDpiY = %.2f\n", rawDpiY);

        char *Rate = kdk_edid_get_refreshRate(name[count]);
        printf("Rate = %s\n", Rate);

        char* rotation = kdk_edid_get_rotation(name[count]);
        printf("rotation = %s\n", rotation);

        printf("---------------------------------------\n");

    }
    kdk_edid_freeall(name);
    return 0;
} 
