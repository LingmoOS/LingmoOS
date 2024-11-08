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
#include <stdlib.h>
#include <sensors/sensors.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <langinfo.h>
#include "libkyfan.h"

static char *print_label(const char *label, int space)
{
    char tmp[20] = "\0";
    char *ret = (char *)malloc(sizeof(char) * 20);
    // printf("%s:%*s", label, space - len, "");
    sprintf(tmp, "%s:", label);
    strcpy(ret, tmp);
    return ret;
}

static int get_input_value(const sensors_chip_name *name,
                           const sensors_subfeature *sub,
                           double *val)
{
    int err;

    err = sensors_get_value(name, sub->number, val);
    return err;
}

static char* print_chip_fan(const sensors_chip_name *name,
                           const sensors_feature *feature,
                           int label_size)
{
    const sensors_subfeature *sf;
    double val;
    char *label;
    char tmp[64] = "\0";
    
    if (!(label = sensors_get_label(name, feature)))
    {
        return NULL;
    }
    char *ret = print_label(label, label_size);
    free(label);

    sf = sensors_get_subfeature(name, feature, SENSORS_SUBFEATURE_FAN_FAULT);
    sf = sensors_get_subfeature(name, feature, SENSORS_SUBFEATURE_FAN_INPUT);
    if (sf && get_input_value(name, sf, &val) == 0)
    {
        sprintf(tmp, "%s%4.0f RPM", ret, val);
        free(ret);
        char *fan = (char *)malloc(strlen(tmp) + 1);
        if(!fan)
        {
            return NULL;
        }
        strcpy(fan, tmp);
        return fan;
    }
    else
    {
        free(ret);
        return NULL;
    }
}

static int get_label_size(const sensors_chip_name *name)
{
    int i;
    const sensors_feature *iter;
    char *label;
    unsigned int max_size = 11; /* 11 as minimum label width */

    i = 0;
    while ((iter = sensors_get_features(name, &i)))
    {
        if ((label = sensors_get_label(name, iter)) && strlen(label) > max_size)
            max_size = strlen(label);
        free(label);
    }

    /* One more for the colon, and one more to guarantee at least one
	   space between that colon and the value */
    return max_size + 2;
}

char** print_chip(const sensors_chip_name *name)
{
    const sensors_feature *feature;
    int label_size;
    char *ret = NULL;
    char **fan = NULL;

    label_size = get_label_size(name);

    int j = 0;
    int nr = 0;
    int count = 0;
    while ((feature = sensors_get_features(name, &nr)))
        count++;

    nr = 0;
    while ((feature = sensors_get_features(name, &nr)))
    {
        switch (feature->type)
        {
        case SENSORS_FEATURE_FAN:
        {
            fan = calloc(count + 1, sizeof(char *));
            if (NULL == fan)
                goto out;

            ret = print_chip_fan(name, feature, label_size);
            if(!ret)
            {
                continue;
            }
            fan[j++] = ret;
        }
            break;
        default:
            continue;
        }
    }
out:
    return fan;
}

char** kdk_fan_get_information()
{
    int err;
    const char *config_file_name = NULL;
    FILE *config_file;

    setlocale(LC_CTYPE, "");

    if (config_file_name)
    {
        if (!strcmp(config_file_name, "-"))
            config_file = stdin;
        else
            config_file = fopen(config_file_name, "r");

        if (!config_file)
        {
            perror(config_file_name);
            return NULL;
        }
    }
    else
    {
        config_file = NULL;
    }

    err = sensors_init(config_file);
    if (err)
    {
        if (config_file)
            fclose(config_file);
        return NULL;
    }

    if (config_file && fclose(config_file) == EOF)
        perror(config_file_name);

    const sensors_chip_name *match = NULL;
    const sensors_chip_name *chip;
    int index = 0;
    char **fan = NULL;

    int chip_nr = 0;
    while ((chip = sensors_get_detected_chips(match, &chip_nr)))
    {

        fan = print_chip(chip);
    }
    sensors_cleanup();
    return fan;
err_out:
    while (fan[index])
    {
        free(fan[index]);
        index ++;
    }
    free(fan);
    return NULL;
}

inline void kdk_fan_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}
