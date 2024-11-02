/*
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

/**
 * @file kyutils_sample.c
 * @brief 
 * @author Fnoily <liuyang@kylinos.cn>
 * @version 1.0
 * @date 2021-10-29
 * Copyright: 2021,KylinSoft Co.,Ltd.
 * 
 */
#include "../kyutils.h"

#include <stdio.h>
#include <string.h>


int main()
{


    int ret = 0;

    // 调用字符型数据单位转换接口
    char  origin_data [20] = "10000.24MB";
    char result_data [50] = {0};
    ret = kdkVolumeBaseCharacterConvert(origin_data, KDK_GIGABYTE, result_data);
    printf("%s\n", result_data);


    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_MEGABYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);

    return 0;
}